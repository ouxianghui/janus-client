/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "Janus_client.h"
#include <iostream>
#include "message_transport.h"
#include "message_models.h"
#include "x2struct.hpp"
#include "string_utils.h"
#include "logger/logger.h"

namespace vi {

	JanusClient::JanusClient(const std::string& url)
		: _url(url)
	{

	}

	JanusClient::JanusClient(const std::string& url, std::shared_ptr<IMessageTransport> transport)
		: _url(url)
		, _transport(transport)
	{

	}

	JanusClient::~JanusClient()
	{

	}

	void JanusClient::setToken(const std::string& token)
	{
		_token = token;
	}

	void JanusClient::setApiSecret(const std::string& apisecret)
	{
		_apisecret = apisecret;
	}

	void JanusClient::addListener(std::shared_ptr<ISFUClientListener> listener)
	{
		addBizObserver<ISFUClientListener>(_listeners, listener);
	}

	void JanusClient::removeListener(std::shared_ptr<ISFUClientListener> listener)
	{
		removeBizObserver<ISFUClientListener>(_listeners, listener);
	}

	void JanusClient::init()
	{
		if (!_transport) {
			_transport = std::make_shared<MessageTransport>();
		}
		_transport->addListener(shared_from_this());
		_transport->connect(_url);
	}

	void JanusClient::createSession(std::shared_ptr<JCCallback> callback) 
	{
		JanusRequest request;
		request.janus = "create";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;

		auto handler = std::make_shared<JCHandler>(request.transaction, callback);

		std::string data = x2struct::X::tojson(request);

		_transport->send(data, handler);
	}

	void JanusClient::destroySession(int64_t sessionId, std::shared_ptr<JCCallback> callback) 
	{
		KeepAliveRequest request;
		request.janus = "destroy";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;

		auto handler = std::make_shared<JCHandler>(request.transaction, callback);

		std::string data = x2struct::X::tojson(request);

		_transport->send(data, handler);
	}

	void JanusClient::reconnectSession(int64_t sessionId, std::shared_ptr<JCCallback> callback) 
	{
		ReconnectRequest request;
		request.janus = "claim";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;

		auto handler = std::make_shared<JCHandler>(request.transaction, callback);

		std::string data = x2struct::X::tojson(request);

		_transport->send(data, handler);
	}

	void JanusClient::keepAlive(int64_t sessionId, std::shared_ptr<JCCallback> callback) 
	{
		KeepAliveRequest request;
		request.janus = "keepalive";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;

		auto handler = std::make_shared<JCHandler>(request.transaction, callback);

		std::string data = x2struct::X::tojson(request);

		_transport->send(data, handler);
	}

	void JanusClient::attach(int64_t sessionId, const std::string& plugin, const std::string& opaqueId, std::shared_ptr<JCCallback> callback)
	{
		AttachRequest request;
		request.janus = "attach";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;
		request.plugin = plugin;
		request.opaque_id = opaqueId;

		auto handler = std::make_shared<JCHandler>(request.transaction, callback);

		std::string data = x2struct::X::tojson(request);

		_transport->send(data, handler);
	}

	void JanusClient::detach(int64_t sessionId, int64_t handleId, std::shared_ptr<JCCallback> callback) 
	{
		DetachRequest request;
		request.janus = "detach";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;
		request.handle_id = handleId;

		auto handler = std::make_shared<JCHandler>(request.transaction, callback);

		std::string data = x2struct::X::tojson(request);

		_transport->send(data, handler);
	}

	void JanusClient::sendMessage(int64_t sessionId, int64_t handleId, const std::string& message, const std::string& jsep, std::shared_ptr<JCCallback> callback)
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

			auto handler = std::make_shared<JCHandler>(request.transaction, callback);

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

			auto handler = std::make_shared<JCHandler>(request.transaction, callback);

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

	void JanusClient::sendTrickleCandidate(int64_t sessionId, int64_t handleId, const CandidateData& candidate, std::shared_ptr<JCCallback> callback) 
	{
		TrickleRequest request;
		request.janus = "trickle";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;
		request.handle_id = handleId;
		request.candidate = candidate;

		auto handler = std::make_shared<JCHandler>(request.transaction, callback);

		std::string data = x2struct::X::tojson(request);

		_transport->send(data, handler);
	}

	void JanusClient::hangup(int64_t sessionId, int64_t handleId, std::shared_ptr<JCCallback> callback) 
	{
		HangupRequest request;
		request.janus = "hangup";
		request.transaction = StringUtils::randomString(12);
		request.token = _token;
		request.apisecret = _apisecret;
		request.session_id = sessionId;
		request.handle_id = handleId;

		auto handler = std::make_shared<JCHandler>(request.transaction, callback);

		std::string data = x2struct::X::tojson(request);

		_transport->send(data, handler);
	}

	void JanusClient::onOpened()
	{
		notifyObserver4Change<ISFUClientListener>(_listeners, [](const std::shared_ptr<ISFUClientListener>& listener) {
			listener->onOpened();
		});
	}

	void JanusClient::onClosed()
	{
		notifyObserver4Change<ISFUClientListener>(_listeners, [](const std::shared_ptr<ISFUClientListener>& listener) {
			listener->onClosed();
		});
	}

	void JanusClient::onFailed(int errorCode, const std::string& reason)
	{
		notifyObserver4Change<ISFUClientListener>(_listeners, [errorCode, reason](const std::shared_ptr<ISFUClientListener>& listener) {
			listener->onFailed(errorCode, reason);
		});
	}

	void JanusClient::onMessage(std::shared_ptr<JanusResponse> model)
	{
		notifyObserver4Change<ISFUClientListener>(_listeners, [model](const std::shared_ptr<ISFUClientListener>& listener) {
			listener->onMessage(model);
		});
	}
}