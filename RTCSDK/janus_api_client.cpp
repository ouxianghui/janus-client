/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "janus_api_client.h"
#include <iostream>
#include "message_transport.h"
#include "message_models.h"
#include "string_utils.h"
#include "logger/logger.h"
#include "rtc_base/thread.h"

namespace vi {

	JanusApiClient::JanusApiClient(const std::string& callbackThreadName) : _callbackThreadName(callbackThreadName)
	{
		_transport = std::make_shared<MessageTransport>();
	}

	JanusApiClient::~JanusApiClient()
	{

	}

	void JanusApiClient::setToken(const std::string& token)
	{
		_token = token;
	}

	void JanusApiClient::setApiSecret(const std::string& apisecret)
	{
		_apisecret = apisecret;
	}

	void JanusApiClient::addListener(std::shared_ptr<ISfuApiClientListener> listener)
	{
		UniversalObservable<ISfuApiClientListener>::addWeakObserver(listener, _callbackThreadName);
	}

	void JanusApiClient::removeListener(std::shared_ptr<ISfuApiClientListener> listener)
	{
		UniversalObservable<ISfuApiClientListener>::removeObserver(listener);
	}

	void JanusApiClient::init()
	{
		_transport->addListener(shared_from_this());
	}

	void JanusApiClient::connect(const std::string& url)
	{
		_url = url;
		_transport->connect(url);
	}

	void JanusApiClient::createSession(std::shared_ptr<JCCallback> callback) 
	{
		JanusRequest request;
		request.janus = "create";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;

		auto handler = std::make_shared<JCHandler>(request.transaction.value(), wrapAsyncCallback(callback));

		std::string data = request.toJsonStr();

		_transport->send(data, handler);
	}

	void JanusApiClient::destroySession(int64_t sessionId, std::shared_ptr<JCCallback> callback) 
	{
		DestroyRequest request;
		request.janus = "destroy";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;

		auto handler = std::make_shared<JCHandler>(request.transaction.value(), wrapAsyncCallback(callback));

		std::string data = request.toJsonStr();

		_transport->send(data, handler);
	}

	void JanusApiClient::reconnectSession(int64_t sessionId, std::shared_ptr<JCCallback> callback) 
	{
		ReconnectRequest request;
		request.janus = "claim";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;

		auto handler = std::make_shared<JCHandler>(request.transaction.value(), wrapAsyncCallback(callback));

		std::string data = request.toJsonStr();

		_transport->send(data, handler);
	}

	void JanusApiClient::keepAlive(int64_t sessionId, std::shared_ptr<JCCallback> callback) 
	{
		KeepAliveRequest request;
		request.janus = "keepalive";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;

		auto handler = std::make_shared<JCHandler>(request.transaction.value(), wrapAsyncCallback(callback));

		std::string data = request.toJsonStr();

		_transport->send(data, handler);
	}

	void JanusApiClient::attach(int64_t sessionId, const std::string& plugin, const std::string& opaqueId, std::shared_ptr<JCCallback> callback)
	{
		AttachRequest request;
		request.janus = "attach";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;
		request.plugin = plugin;
		request.opaque_id = opaqueId;

		auto handler = std::make_shared<JCHandler>(request.transaction.value(), wrapAsyncCallback(callback));

		std::string data = request.toJsonStr();

		_transport->send(data, handler);
	}

	void JanusApiClient::detach(int64_t sessionId, int64_t handleId, std::shared_ptr<JCCallback> callback) 
	{
		DetachRequest request;
		request.janus = "detach";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;
		request.handle_id = handleId;

		auto handler = std::make_shared<JCHandler>(request.transaction.value(), wrapAsyncCallback(callback));

		std::string data = request.toJsonStr();

		_transport->send(data, handler);
	}

	void JanusApiClient::sendMessage(int64_t sessionId, int64_t handleId, const std::string& message, const std::string& jsep, std::shared_ptr<JCCallback> callback)
	{
		if (jsep.empty()) {
			MessageRequest request;
			request.janus = "message";
			request.transaction = StringUtils::randomString(12);
			request.token = _token;
			request.apisecret = _apisecret;
			request.session_id = sessionId;
			request.handle_id = handleId;
			request.body = "#-MESSAGE-#";

			auto handler = std::make_shared<JCHandler>(request.transaction.value(), wrapAsyncCallback(callback));

			std::string data = request.toJsonStr();
			DLOG("before replace, data = {}", data.c_str());
			std::string tag("\"#-MESSAGE-#\"");
			size_t pos = data.find(tag);
			if (pos != std::string::npos) {
				data = data.replace(pos, tag.length(), message);
			}
			DLOG("after replace, data = {}", data.c_str());
			_transport->send(data, handler);
		}
		else {
			JsepRequest request;
			request.janus = "message";
			request.transaction = StringUtils::randomString(12);
			request.token = _token;
			request.apisecret = _apisecret;
			request.session_id = sessionId;
			request.handle_id = handleId;
			request.body = "#-MESSAGE-#";
			request.jsep = "#-JSEP-#";

			auto handler = std::make_shared<JCHandler>(request.transaction.value(), wrapAsyncCallback(callback));

			std::string data = request.toJsonStr();

			DLOG("before replace, data = {}", data.c_str());

			std::string tag("\"#-MESSAGE-#\"");
			size_t pos = data.find(tag);
			if (pos != std::string::npos) {
				data = data.replace(pos, tag.length(), message);
			}

			tag = "\"#-JSEP-#\"";
			pos = std::string::npos;
			pos = data.find(tag);
			if (pos != std::string::npos) {
				data = data.replace(pos, tag.length(), jsep);
			}

			DLOG("after replace, data = {}", data.c_str());

			_transport->send(data, handler);
		}
	}

	void JanusApiClient::sendTrickleCandidate(int64_t sessionId, int64_t handleId, const CandidateData& candidate, std::shared_ptr<JCCallback> callback) 
	{
		TrickleRequest request;
		request.janus = "trickle";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;
		request.handle_id = handleId;
		request.candidate = candidate;

		auto handler = std::make_shared<JCHandler>(request.transaction.value(), wrapAsyncCallback(callback));

		std::string data = request.toJsonStr();

		_transport->send(data, handler);
	}

	void JanusApiClient::hangup(int64_t sessionId, int64_t handleId, std::shared_ptr<JCCallback> callback) 
	{
		HangupRequest request;
		request.janus = "hangup";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;
		request.handle_id = handleId;

		auto handler = std::make_shared<JCHandler>(request.transaction.value(), wrapAsyncCallback(callback));

		std::string data = request.toJsonStr();

		_transport->send(data, handler);
	}

	void JanusApiClient::onOpened()
	{
		UniversalObservable<ISfuApiClientListener>::notifyObservers([wself = weak_from_this()](const auto& observer) {
			if (auto self = wself.lock()) {
				observer->onOpened();
			}
		});
	}

	void JanusApiClient::onClosed()
	{
		UniversalObservable<ISfuApiClientListener>::notifyObservers([wself = weak_from_this()](const auto& observer) {
			if (auto self = wself.lock()) {
				observer->onClosed();
			}
		});
	}

	void JanusApiClient::onFailed(int errorCode, const std::string& reason)
	{
		UniversalObservable<ISfuApiClientListener>::notifyObservers([wself = weak_from_this(), errorCode, reason](const auto& observer) {
			if (auto self = wself.lock()) {
				observer->onFailed(errorCode, reason);
			}
		});
	}

	void JanusApiClient::onMessage(const std::string& json)
	{
		UniversalObservable<ISfuApiClientListener>::notifyObservers([wself = weak_from_this(), json](const auto& observer) {
			if (auto self = wself.lock()) {
				observer->onMessage(json);
			}
		});
	}

	std::shared_ptr<JCCallback> JanusApiClient::wrapAsyncCallback(std::shared_ptr<JCCallback> callback)
	{
		auto lambda = [wself = weak_from_this(), callback](const std::string& json) {
			if (auto self = wself.lock()) {
				if (callback) {
					self->UniversalObservable<ISfuApiClientListener>::notifyObservers([wself, callback, json](const auto& observer) {
						if (auto self = wself.lock()) {
							if (callback) {
								(*callback)(json);
							}
						}
					});
				}
			}
		};
		return std::make_shared<JCCallback>(lambda);
	}
}