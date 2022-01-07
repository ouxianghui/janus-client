/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "signaling_client.h"
#include "Service/unified_factory.h"
#include "janus_api_client.h"
#include "plugin_client.h"
#include "rtc_base/thread.h"
#include "logger/logger.h"
#include "service/rtc_engine.h"
#include "utils/thread_provider.h"
#include "utils/task_scheduler.h"
#include "message_models.h"
#include "absl/types/optional.h"

namespace vi {

	SignalingClient::SignalingClient()
		: _eventHandlerThread(nullptr)
	{
	}

	SignalingClient::~SignalingClient()
	{
		DLOG("~SignalingClient");

		if (_heartbeatTaskScheduler) {
			_heartbeatTaskScheduler->cancelAll();
		}
		DLOG("~SignalingClient");
	}

	void SignalingClient::init()
	{
		_eventHandlerThread = TMgr->thread("plugin-client");
		
		//std::string url = "ws://192.168.0.108:8188/ws";
		_client = std::make_shared<vi::JanusApiClient>("signaling-service");
		_client->addListener(shared_from_this());
		_client->init();

		_heartbeatTaskScheduler = TaskScheduler::create();
	}

	void SignalingClient::cleanup()
	{
		auto event = std::make_shared<DestroySessionEvent>();
		event->notifyDestroyed = true;
		event->cleanupHandles = true;
		event->callback = std::make_shared<EventCallback>([](bool success, const std::string& response) {
			DLOG("destroy, success = {}, response = {}", success, response.c_str());
		});
		this->destroy(event);
	}

	void SignalingClient::registerObserver(std::shared_ptr<ISignalingClientObserver> observer)
	{
		UniversalObservable<ISignalingClientObserver>::addWeakObserver(observer, "main");
	}

	void SignalingClient::unregisterObserver(std::shared_ptr<ISignalingClientObserver> observer)
	{
		UniversalObservable<ISignalingClientObserver>::removeObserver(observer);
	}

	void SignalingClient::connect(const std::string& url)
	{
		if (!_client) {
			DLOG("_client == nullptr");
			return;
		}
		DLOG("janus api client, connecting...");
		_client->connect(url);
	}

	SessionStatus SignalingClient::sessionStatus()
	{
		return _sessionStatus;
	}

	void SignalingClient::attach(const std::string& plugin, const std::string& opaqueId, std::shared_ptr<PluginClient> pluginClient)
	{
		if (!pluginClient) {
			return;
		}

		auto lambda = [wself = weak_from_this(), pluginClient](const std::string& json) {
			std::string err;
			std::shared_ptr<AttachResponse> model = fromJsonString<AttachResponse>(json, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			DLOG("model->janus = {}", model->janus.value_or(""));
			if (auto self = wself.lock()) {
				if (model->janus.value_or("") == "success") {
					int64_t handleId = model->data->id.value();
					pluginClient->setHandleId(handleId);
					self->_pluginClientMap[handleId] = pluginClient;
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [wself, pluginClient]() {
						auto self = wself.lock();
						if (!self) {
							return;
						}
						pluginClient->onAttached(true);
					});
				}
				else if (model->janus.value_or("") == "error") {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [wself, pluginClient]() {
						auto self = wself.lock();
						if (!self) {
							return;
						}
						pluginClient->onAttached(false);
					});
				}
			}
		};
		std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
		_client->attach(_sessionId, plugin, opaqueId, callback);
	}

	void SignalingClient::destroy(std::shared_ptr<DestroySessionEvent> event)
	{
		destroySession(event);
	}

	void SignalingClient::reconnectSession()
	{
		// TODO: ?
		std::shared_ptr<CreateSessionEvent> event = std::make_shared<CreateSessionEvent>();
		event->reconnect = true;
		auto lambda = [](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
		};
		event->callback = std::make_shared<vi::EventCallback>(lambda);
		createSession(event);
	}

	void SignalingClient::sendMessage(int64_t handleId, std::shared_ptr<MessageEvent> event)
	{
		if (_sessionStatus == SessionStatus::CONNECTED) {
			if (const auto& pluginClient = getHandler(handleId)) {
				auto lambda = [wself = weak_from_this(), event](const std::string& json) {
					DLOG("janus = {}", json);
					if (auto self = wself.lock()) {
						if (!event) {
							return;
						}

						std::string err;
						std::shared_ptr<JanusResponse> model = fromJsonString<JanusResponse>(json, err);
						if (!err.empty()) {
							DLOG("parse JanusResponse failed");
							return;
						}

						if (event->callback) {
							if (model->janus.value_or("") == "success" || model->janus.value_or("") == "ack") {
								self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback, json]() {
									if (cb) {
										(*cb)(true, json);
									}
								});
							}
							else if (model->janus.value_or("") != "ack") {
								self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback, json]() {
									if (cb) {
										(*cb)(false, json);
									}
								});
							}
						}
					}
				};
				std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
				_client->sendMessage(_sessionId, handleId, event->message, event->jsep, callback);
			}
		}
		else {
			if (event && event->callback) {
				_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "service down!");
				});
			}
		}
	}

	void SignalingClient::hangup(int64_t handleId, bool hangupRequest)
	{
		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			return;
		}

		if (hangupRequest == true) {
			auto lambda = [wself = weak_from_this()](const std::string& json) {
				DLOG("janus = {}", json);
				if (auto self = wself.lock()) {
				}
			};
			std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
			_client->hangup(_sessionId, handleId, callback);
		}
	}

	void SignalingClient::destroyHandle(int64_t handleId, std::shared_ptr<DetachEvent> event) 
	{
		DLOG("destroyHandle()");

		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			DLOG("Invalid handle");
			if (event && event->callback) {
				_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(true, "");
				});
			}
			return;
		}

		_eventHandlerThread->PostTask(RTC_FROM_HERE, [wself = weak_from_this(), handleId]() {
			auto self = wself.lock();
			if (!self) {
				return;
			}
			if (auto& pluginClient = self->getHandler(handleId)) {
				pluginClient->onCleanup();
			}
		});

		if (!event) {
			return;
		}
		if (event->noRequest) {
			// We're only removing the handle locally
			if (event->callback) {
				_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(true, "");
				});
			}
			return;
		}
		if (!_connected) {
			DLOG("Is the server down? (connected = false)");
			return;
		}

		auto lambda = [wself = weak_from_this(), handleId](const std::string& json) {
			DLOG("janus = {}", json);
			auto self = wself.lock();
			if (!self) {
				return;
			}

			self->_pluginClientMap.erase(handleId);
		};
		std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
		_client->detach(_sessionId, handleId, callback);
	}

	void SignalingClient::detach(int64_t handleId, std::shared_ptr<DetachEvent> event) 
	{
		destroyHandle(handleId, event);
	}

	// ISfuApiClientListener

	void SignalingClient::onOpened()
	{
		std::shared_ptr<CreateSessionEvent> event = std::make_shared<CreateSessionEvent>();
		event->reconnect = false;
		auto lambda = [wself = weak_from_this()](bool success, const std::string& response) {
			if (auto self = wself.lock()) {
				self->_connected = true;
			}
		};
		event->callback = std::make_shared<vi::EventCallback>(lambda);
		createSession(event);
	}

	void SignalingClient::onClosed()
	{
		_connected = false;
	}	
	
	void SignalingClient::onFailed(int errorCode, const std::string& reason)
	{
		_connected = false;
	}

	void SignalingClient::onMessage(const std::string& json)
	{
		std::string err;
		std::shared_ptr<JanusResponse> response = fromJsonString<JanusResponse>(json, err);
		if (!err.empty()) {
			DLOG("parse JanusResponse failed");
			return;
		}

		int64_t sender = response->sender.value();
		auto& pluginClient = getHandler(sender);
		if (!pluginClient) {
			return;
		}

		auto wself = weak_from_this();

		int32_t retries = 0;

		if (response->janus.value_or("") == "keepalive") {
			DLOG("Got a keepalive on session: {}", _sessionId);
		}
		else if (response->janus.value_or("") == "server_info") {
			// Just info on the Janus instance
			DLOG("Got info on the Janus instance: {}", response->janus.value_or(""));
		}
		else if (response->janus.value_or("") == "trickle") {
			DLOG("Got info on the Janus instance: {}", response->janus.value_or(""));

			_eventHandlerThread->PostTask(RTC_FROM_HERE, [json, sender, wself]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (auto pluginClient = self->getHandler(sender)) {
					pluginClient->onTrickle(json);
				}
			});
		}
		else if (response->janus.value_or("") == "webrtcup") {
			// The PeerConnection with the server is up! Notify this
			DLOG("Got a webrtcup event on session: {}", _sessionId);

			_eventHandlerThread->PostTask(RTC_FROM_HERE, [sender, wself]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (auto pluginClient = self->getHandler(sender)) {
					pluginClient->onWebrtcStatus(true, "");
				}
			});
		}
		else if (response->janus.value_or("") == "hangup") {
			// A plugin asked the core to hangup a PeerConnection on one of our handles
			DLOG("Got a hangup event on session: {}", _sessionId);

			std::string err;
			std::shared_ptr<HangupResponse> model = fromJsonString<HangupResponse>(json, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			_eventHandlerThread->PostTask(RTC_FROM_HERE, [sender, wself, reason = model->reason.value_or("")]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (auto pluginClient = self->getHandler(sender)) {
					pluginClient->onWebrtcStatus(false, reason);
					pluginClient->onHangup();
				}
			});
		}
		else if (response->janus.value_or("") == "detached") {
			// A plugin asked the core to detach one of our handles
			DLOG("Got a detached event on session: {}", _sessionId);

			_eventHandlerThread->PostTask(RTC_FROM_HERE, [sender, wself]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (auto pluginClient = self->getHandler(sender)) {
					pluginClient->onDetached();
				}
			});
		}
		else if (response->janus.value_or("") == "media") {
			// Media started/stopped flowing
			DLOG("Got a media event on session: {}", _sessionId);

			std::string err;
			std::shared_ptr<MediaResponse> model = fromJsonString<MediaResponse>(json, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			_eventHandlerThread->PostTask(RTC_FROM_HERE, [sender, wself, model]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (auto pluginClient = self->getHandler(sender)) {
					pluginClient->onMediaStatus(model->type.value_or(""), model->receiving.value_or(false), model->mid.value_or(""));

				}
			});
		}
		else if (response->janus.value_or("") == "slowlink") {
			DLOG("Got a slowlink event on session: {}", _sessionId);

			std::string err;
			std::shared_ptr<SlowlinkResponse> model = fromJsonString<SlowlinkResponse>(json, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			_eventHandlerThread->PostTask(RTC_FROM_HERE, [sender, wself, model]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (auto pluginClient = self->getHandler(sender)) {
					pluginClient->onSlowLink(model->uplink.value_or(false), model->lost.value_or(false), model->mid.value_or(""));
				}
			});
		}
		else if (response->janus.value_or("") == "event") {
			DLOG("Got a plugin event on session: {}", _sessionId);

			std::string err;
			std::shared_ptr<JanusEvent> event = fromJsonString<JanusEvent>(json, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			if (!event->plugindata) {
				ELOG("Missing plugindata...");
				return;
			}
			
			DLOG(" -- Event is coming from {} ({})", sender, event->plugindata->plugin.value_or(""));

			std::string jsep = event->jsep ? event->jsep->toJsonStr() : "";

			_eventHandlerThread->PostTask(RTC_FROM_HERE, [sender, wself, json, jsep]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (auto pluginClient = self->getHandler(sender)) {
					pluginClient->onMessage(json, jsep);
				}
			});
		}
		else if (response->janus.value_or("") == "timeout") {
			ELOG("Timeout on session: {}", _sessionId);
			_eventHandlerThread->PostTask(RTC_FROM_HERE, [sender, wself]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (auto pluginClient = self->getHandler(sender)) {
					pluginClient->onTimeout();
				}
			});
		}
		else if (response->janus.value_or("") == "error") {
			// something wrong happened
			DLOG("Something wrong happened: {}", response->janus.value_or(""));

			_eventHandlerThread->PostTask(RTC_FROM_HERE, [sender, wself]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (auto pluginClient = self->getHandler(sender)) {
					pluginClient->onError("");
				}
			});
		}
		else {
			WLOG("Unknown message/event {} on session: {}'", response->janus.value_or(""),  _sessionId);
		}
	}

	void SignalingClient::createSession(std::shared_ptr<CreateSessionEvent> event)
	{
		auto lambda = [wself = weak_from_this(), event](const std::string& json) {
			std::string err;
			std::shared_ptr<CreateSessionResponse> model = fromJsonString<CreateSessionResponse>(json, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			DLOG("model.janus = {}", model->janus.value_or(""));
			if (auto self = wself.lock()) {
				self->_sessionId = model->session_id.value_or(0) > 0 ? model->session_id.value() : model->data->id.value();
				self->startHeartbeat();
				self->_sessionStatus =SessionStatus::CONNECTED;

				self->UniversalObservable<ISignalingClientObserver>::notifyObservers([](const auto& observer) {
					observer->onSessionStatus(SessionStatus::CONNECTED);
				});

				if (event && event->callback) {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(true, "");
					});
				}
			}
		};
		std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
		if (event && event->reconnect) {
			_client->reconnectSession(_sessionId, callback);
		}
		else {
			_client->createSession(callback);
		}
	}

	void SignalingClient::startHeartbeat()
	{
		_heartbeatTaskId = _heartbeatTaskScheduler->schedule([wself = weak_from_this()]() {
			if (auto self = wself.lock()) {
				DLOG("sessionHeartbeat() called");
				auto lambda = [](const std::string& json) {
					DLOG("janus = {}", json);
				};
				std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
				self->_client->keepAlive(self->_sessionId, callback);
			}
		}, 5000, true);
	}

	std::shared_ptr<PluginClient> SignalingClient::getHandler(int64_t handleId)
	{
		if (handleId == -1) {
			ELOG("Missing sender...");
			return nullptr;
		}
		if (_pluginClientMap.find(handleId) == _pluginClientMap.end()) {
			ELOG("This handle is not attached to this session");
			return nullptr;
		}
		return _pluginClientMap[handleId].lock();
	}

	void SignalingClient::sendTrickleCandidate(int64_t handleId, std::shared_ptr<TrickleCandidateEvent> event)
	{
		auto lambda = [wself = weak_from_this(), event](const std::string& json) {
			if (auto self = wself.lock()) {
				if (event && event->callback) {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback, json]() {
						(*cb)(true, json);
					});
				}
			}
		};
		std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
		_client->sendTrickleCandidate(_sessionId, handleId, event->candidate, callback);
	}

	void SignalingClient::destroySession(std::shared_ptr<DestroySessionEvent> event)
	{
		DLOG("Destroying session: {}", _sessionId);
		if (_sessionId == -1) {
			DLOG("No session to destroy");
			if (event && event->callback) {
				_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(true, "");
				});
			}
			if (event->notifyDestroyed) {
				// TODO:
			}
			return;
		}
		if (!event) {
			return;
		}
		if (event->cleanupHandles) {
			for (auto pair : _pluginClientMap) {
				std::shared_ptr<DetachEvent> de = std::make_shared<DetachEvent>();
				de->noRequest = true;
				int64_t hId = pair.first;
				auto lambda = [hId](bool success, const std::string& response) {
					DLOG("destroyHandle, handleId = {}, success = {}, response = {}", hId, success, response.c_str());
				};
				de->callback = std::make_shared<vi::EventCallback>(lambda);
				destroyHandle(hId, de);
			}

			//_pluginClientMap.clear();
		}
		if (!_connected) {
			DLOG("Is the server down? (connected = false)");
			if(event->callback) {
				//_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
				const auto& cb = event->callback;
				(*cb)(true, "");
				//});
			}
			return;
		}

		// TODO: destroy session from janus 
		auto lambda = [wself = weak_from_this()](const std::string& json) {
			DLOG("janus = {}", json);
			if (auto self = wself.lock()) {
				self->_client->removeListener(self);
			}
		};
		std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
		_client->destroySession(_sessionId, callback);
	}
}
