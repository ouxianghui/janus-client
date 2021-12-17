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
		DLOG("~MessageTransport()");
	}

	void MessageTransport::init()
	{

	}

	void MessageTransport::destroy()
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
		UniversalObservable<IMessageTransportListener>::addWeakObserver(listener, "message-transport");
	}

	void MessageTransport::removeListener(std::shared_ptr<IMessageTransportListener> listener)
	{
		UniversalObservable<IMessageTransportListener>::removeObserver(listener);
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

		UniversalObservable<IMessageTransportListener>::notifyObservers([wself = weak_from_this()](const auto& observer) {
			if (auto self = wself.lock()) {
				observer->onOpened();
			}
		});
	}

	void MessageTransport::onFail(int errorCode, const std::string& reason)
	{
		DLOG("errorCode = {}, reason = {}", errorCode, reason.c_str());

		UniversalObservable<IMessageTransportListener>::notifyObservers([wself = weak_from_this(), errorCode, reason](const auto& observer) {
			if (auto self = wself.lock()) {
				observer->onFailed(errorCode, reason);
			}
		});
	}

	void MessageTransport::onClose(int closeCode, const std::string& reason)
	{
		DLOG("errorCode = {}, reaseon = {}", closeCode, reason.c_str());

		UniversalObservable<IMessageTransportListener>::notifyObservers([wself = weak_from_this()](const auto& observer) {
			if (auto self = wself.lock()) {
				observer->onClosed();
			}
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
		auto response = fromJsonString<JanusResponse>(data, err);

		if (!err.empty()) {
			DLOG("parse JanusResponse failed");
			return;
		}

		if (!response->janus) {
			DLOG("could not find 'janus' in response");
			return;
		}

		if (response->transaction && (response->janus.value_or("") == "ack"
			|| response->janus.value_or("") == "success"
			|| response->janus.value_or("") == "error"
			|| response->janus.value_or("") == "server_info")) {
			if (auto thread = TMgr->thread("message-transport")) {
				TMgr->thread("message-transport")->PostTask(RTC_FROM_HERE, [wself = weak_from_this(), response, data]() {
					if (auto self = wself.lock()) {
						std::lock_guard<std::mutex> locker(self->_callbackMutex);
						const std::string& transaction = response->transaction.value();
						if (self->_callbacksMap.find(transaction) != self->_callbacksMap.end()) {
							std::shared_ptr<JCCallback> callback = self->_callbacksMap[transaction];
							if (callback) {
								(*callback)(data);
							}
							self->_callbacksMap.erase(transaction);
						}
					}
				});
			}

		}
		else {
			UniversalObservable<IMessageTransportListener>::notifyObservers([wself = weak_from_this(), data](const auto& observer) {
				if (auto self = wself.lock()) {
					observer->onMessage(data);
				}
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