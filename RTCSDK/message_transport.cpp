/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "message_transport.h"
#include <iostream>
#include "websocket/i_connection_listener.h"
#include "websocket/websocket_endpoint.h"
#include "i_message_transport_listener.h"
#include "logger/logger.h"
#include "message_models.h"
#include "json/serialization_json.hpp"

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
		addBizObserver<IMessageTransportListener>(_listeners, listener);
	}

	void MessageTransport::removeListener(std::shared_ptr<IMessageTransportListener> listener)
	{
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
		notifyObserver4Change<IMessageTransportListener>(_listeners, [](const auto& observer) {
			observer->onOpened();
		});
	}

	void MessageTransport::onFail(int errorCode, const std::string& reason)
	{
		DLOG("errorCode = {}, reason = {}", errorCode, reason.c_str());
		notifyObserver4Change<IMessageTransportListener>(_listeners, [errorCode, reason](const auto& observer) {
			observer->onFailed(errorCode, reason);
		});
	}

	void MessageTransport::onClose(int closeCode, const std::string& reason)
	{
		DLOG("errorCode = {}, reaseon = {}", closeCode, reason.c_str());
		notifyObserver4Change<IMessageTransportListener>(_listeners, [](const auto& observer) {
			observer->onClosed();
		});
	}

	bool MessageTransport::onValidate()
	{
		DLOG("validate");
		return true;
	}

	void MessageTransport::onTextMessage(const std::string& json)
	{
		DLOG("json = {}", json.c_str());

		// |unpublished| can be int or string, replace string 'ok' to 0
		std::string data = json;

		// TODO: remove workaround
		std::string tag1("\"unpublished\": \"ok\"");
		size_t pos = data.find(tag1);
		if (pos != std::string::npos) {
			data = data.replace(pos, tag1.length(), "\"unpublished\": 0");
		}

		std::string tag2("\"leaving\": \"ok\"");
		pos = data.find(tag2);
		if (pos != std::string::npos) {
			data = data.replace(pos, tag2.length(), "\"leaving\": 0");
		}

		std::string err;
		auto respone = fromJsonString<JanusResponse>(data, err);

		if (!err.empty()) {
			DLOG("parse JanusResponse failed");
			return;
		}

		if (!respone->janus) {
			DLOG("could not find 'janus' in response");
			return;
		}

		if (respone->transaction && (respone->janus.value_or("") == "ack"
			|| respone->janus.value_or("") == "success"
			|| respone->janus.value_or("") == "error"
			|| respone->janus.value_or("") == "server_info")) {
			std::lock_guard<std::mutex> locker(_callbackMutex);
			const std::string& transaction = respone->transaction.value();
			if (_callbacksMap.find(transaction) != _callbacksMap.end()) {
				std::shared_ptr<JCCallback> callback = _callbacksMap[transaction];
				_callbacksMap.erase(transaction);
				if (callback) {
					(*callback)(data);
				}
			}
		}
		else {
			notifyObserver4Change<IMessageTransportListener>(_listeners, [data](const auto& observer) {
				observer->onMessage(data);;
			});
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