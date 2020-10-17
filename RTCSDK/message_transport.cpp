/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "message_transport.h"
#include <iostream>
#include "Websocket/i_connection_listener.h"
#include "Websocket/websocket_endpoint.h"
#include "i_message_transport_listener.h"
#include "logger/logger.h"

namespace vi {
	MessageTransport::MessageTransport()
	{
		_websocket = std::make_shared<WebsocketEndpoint>();
	}

	MessageTransport::~MessageTransport()
	{

	}

	bool MessageTransport::isValid()
	{
		if (_websocket && _connectionId != -1) {
			return true;
		}
		return false;
	}

	// IMessageTransportor
	void MessageTransport::addListener(std::shared_ptr<IMessageTransportListener> listener)
	{
		std::lock_guard<std::mutex> locker(_listenerMutex);
		addBizObserver<IMessageTransportListener>(_listeners, listener);
	}

	void MessageTransport::removeListener(std::shared_ptr<IMessageTransportListener> listener)
	{
		std::lock_guard<std::mutex> locker(_listenerMutex);
		removeBizObserver<IMessageTransportListener>(_listeners, listener);
	}

	void MessageTransport::connect(const std::string& url)
	{
		_url = url;
		if (_websocket) {
			_connectionId = _websocket->connect(_url, shared_from_this(), "janus-protocol");
		}
	}

	void MessageTransport::disconnect()
	{
		if (isValid()) {
			_websocket->close(_connectionId, websocketpp::close::status::normal, "");
		}
	}

	void MessageTransport::send(const std::string& data, std::shared_ptr<JCHandler> handler)
	{
		if (isValid()) {
			_websocket->sendText(_connectionId, data);
			DLOG("sendText: {}", data.c_str());
			if (handler->valid()) {
				std::lock_guard<std::mutex> locker(_callbackMutex);
				_callbacksMap[handler->transaction] = handler->callback;
			}
		}
	}

	void MessageTransport::send(const std::vector<uint8_t>& data, std::shared_ptr<JCHandler> handler)
	{
		if (isValid()) {
			_websocket->sendBinary(_connectionId, data);
			if (handler->valid()) {
				std::lock_guard<std::mutex> locker(_callbackMutex);
				_callbacksMap[handler->transaction] = handler->callback;
			}
		}
	}

	// IConnectionListener
	void MessageTransport::onOpen()
	{
		DLOG("opened");
		std::lock_guard<std::mutex> locker(_listenerMutex);
		for (const auto& listener : _listeners) {
			if (auto li = listener.lock()) {
				li->onOpened();
			}
		}
	}

	void MessageTransport::onFail(int errorCode, const std::string& reason)
	{
		DLOG("errorCode = {}, reason = {}", errorCode, reason.c_str());
	}

	void MessageTransport::onClose(int closeCode, const std::string& reason)
	{
		DLOG("errorCode = {}, reaseon = {}", closeCode, reason.c_str());
		std::lock_guard<std::mutex> locker(_listenerMutex);
		for (const auto& listener : _listeners) {
			if (auto li = listener.lock()) {
				li->onClosed();
			}
		}
	}

	bool MessageTransport::onValidate()
	{
		DLOG("validate");
		return true;
	}

	void MessageTransport::onTextMessage(const std::string& text)
	{
		DLOG("text = {}", text.c_str());

		// |unpublished| can be int or string, replace string 'ok' to 0
		std::string data = text;
		std::string tag("\"unpublished\": \"ok\"");
		size_t pos = data.find(tag);
		if (pos != std::string::npos) {
			data = data.replace(pos, tag.length(), "\"unpublished\": 0");
		}

		std::shared_ptr<JanusResponse> model = std::make_shared<JanusResponse>();
		//JanusResponse  model;
		x2struct::X::loadjson(data, *model, false, true);

		if (!model->xhas("janus")) {
			DLOG("!response->xhas(\"janus\")");
			return;
		}
		if (model->janus == "event") {
			std::string event = model->janus;
		}
		if (model->xhas("transaction") && (model->janus == "ack" || model->janus == "success" || model->janus == "error" || model->janus == "server_info")) {
			std::lock_guard<std::mutex> locker(_callbackMutex);
			const std::string& transaction = model->transaction;
			if (_callbacksMap.find(transaction) != _callbacksMap.end()) {
				std::shared_ptr<JCCallback> callback = _callbacksMap[transaction];
				_callbacksMap.erase(transaction);
				if (callback) {
					(*callback)(model);
				}
			}
		}
		else {
			std::lock_guard<std::mutex> locker(_listenerMutex);
			for (const auto& listener : _listeners) {
				if (auto li = listener.lock()) {
					li->onMessage(model);
				}
			}
		}
	}

	void MessageTransport::onBinaryMessage(const std::vector<uint8_t>& data)
	{
		DLOG("data.size() = {}", data.size());
	}

	bool MessageTransport::onPing(const std::string& text)
	{
		DLOG("text = {}", text.c_str());
		return true;
	}

	void MessageTransport::onPong(const std::string& text)
	{
		DLOG("text = {}", text.c_str());
	}

	void MessageTransport::onPongTimeout(const std::string& text)
	{
		DLOG("text = {}", text.c_str());
	}
}