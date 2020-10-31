/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "janus_api_client.h"
#include <iostream>
#include "message_transport.h"
#include "message_models.h"
#include "x2struct.hpp"
#include "string_utils.h"
#include "logger/logger.h"
#include "thread_manager.h"
#include "rtc_base/thread.h"

namespace vi {

	JanusApiClient::JanusApiClient(const std::string& url, rtc::Thread* callbackThread)
		: _url(url)
		, _thread(callbackThread)
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
		addBizObserver<ISfuApiClientListener>(_listeners, listener);
	}

	void JanusApiClient::removeListener(std::shared_ptr<ISfuApiClientListener> listener)
	{
		removeBizObserver<ISfuApiClientListener>(_listeners, listener);
	}

	void JanusApiClient::init()
	{
		_transport->addListener(shared_from_this());
		_transport->connect(_url);
	}

	void JanusApiClient::createSession(std::shared_ptr<JCCallback> callback) 
	{
		JanusRequest request;
		request.janus = "create";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;

		auto handler = std::make_shared<JCHandler>(request.transaction, wrapAsyncCallback(callback));

		std::string data = x2struct::X::tojson(request);

		_transport->send(data, handler);
	}

	void JanusApiClient::destroySession(int64_t sessionId, std::shared_ptr<JCCallback> callback) 
	{
		KeepAliveRequest request;
		request.janus = "destroy";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;

		auto handler = std::make_shared<JCHandler>(request.transaction, wrapAsyncCallback(callback));

		std::string data = x2struct::X::tojson(request);

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

		auto handler = std::make_shared<JCHandler>(request.transaction, wrapAsyncCallback(callback));

		std::string data = x2struct::X::tojson(request);

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

		auto handler = std::make_shared<JCHandler>(request.transaction, wrapAsyncCallback(callback));

		std::string data = x2struct::X::tojson(request);

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

		auto handler = std::make_shared<JCHandler>(request.transaction, wrapAsyncCallback(callback));

		std::string data = x2struct::X::tojson(request);

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

		auto handler = std::make_shared<JCHandler>(request.transaction, wrapAsyncCallback(callback));

		std::string data = x2struct::X::tojson(request);

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

			auto handler = std::make_shared<JCHandler>(request.transaction, wrapAsyncCallback(callback));

			std::string data = x2struct::X::tojson(request);
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

			auto handler = std::make_shared<JCHandler>(request.transaction, wrapAsyncCallback(callback));

			std::string data = x2struct::X::tojson(request);

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

		auto handler = std::make_shared<JCHandler>(request.transaction, wrapAsyncCallback(callback));

		std::string data = x2struct::X::tojson(request);

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

		auto handler = std::make_shared<JCHandler>(request.transaction, wrapAsyncCallback(callback));

		std::string data = x2struct::X::tojson(request);

		_transport->send(data, handler);
	}

	void JanusApiClient::onOpened()
	{
		notifyObserver4Change<ISfuApiClientListener>(_listeners, [wself = weak_from_this()](const std::shared_ptr<ISfuApiClientListener>& listener) {
			if (auto self = wself.lock()) {
				if (self->_thread) {
					self->_thread->PostTask(RTC_FROM_HERE, [listener]() {
						listener->onOpened();
					});
				}
				else {
					listener->onOpened();
				}
			}
		});
	}

	void JanusApiClient::onClosed()
	{
		notifyObserver4Change<ISfuApiClientListener>(_listeners, [wself = weak_from_this()](const std::shared_ptr<ISfuApiClientListener>& listener) {
			if (auto self = wself.lock()) {
				if (self->_thread) {
					self->_thread->PostTask(RTC_FROM_HERE, [listener]() {
						listener->onClosed();
					});
				}
				else {
					listener->onClosed();
				}
			}
		});
	}

	void JanusApiClient::onFailed(int errorCode, const std::string& reason)
	{
		notifyObserver4Change<ISfuApiClientListener>(_listeners, [wself = weak_from_this(), errorCode, reason](const std::shared_ptr<ISfuApiClientListener>& listener) {
			if (auto self = wself.lock()) {
				if (self->_thread) {
					self->_thread->PostTask(RTC_FROM_HERE, [listener, errorCode, reason]() {
						listener->onFailed(errorCode, reason);
					});
				}
				else {
					listener->onFailed(errorCode, reason);
				}
			}
		});
	}

	void JanusApiClient::onMessage(std::shared_ptr<JanusResponse> model)
	{
		notifyObserver4Change<ISfuApiClientListener>(_listeners, [wself = weak_from_this(), model](const std::shared_ptr<ISfuApiClientListener>& listener) {
			if (auto self = wself.lock()) {
				if (self->_thread) {
					self->_thread->PostTask(RTC_FROM_HERE, [listener, model]() {
						listener->onMessage(model);
					});
				}
				else {
					listener->onMessage(model);
				}
			}
		});
	}

	std::shared_ptr<JCCallback> JanusApiClient::wrapAsyncCallback(std::shared_ptr<JCCallback> callback)
	{
		if (!_thread) {
			return callback;
		}
		else {
			auto lambda = [wself = weak_from_this(), callback](std::shared_ptr<JanusResponse> model) {
				if (auto self = wself.lock()) {
					if (self->_thread && callback) {
						self->_thread->PostTask(RTC_FROM_HERE, [callback, model]() {
							(*callback)(model);
						});
					}
				}
			};
			return std::make_shared<JCCallback>(lambda);
		}
	}
}