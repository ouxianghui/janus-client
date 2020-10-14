/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <thread>
#include "i_message_transport.h"
#include <unordered_map>
#include "Websocket/i_connection_listener.h"
#include "Websocket/websocket_endpoint.h"
#include "Service/observable.h"

namespace vi {
	class IMessageTransportListener;
	class MessageTransport
		: public IMessageTransport
		, public IConnectionListener
		, public core::Observable
		, public std::enable_shared_from_this<MessageTransport>
	{
	public:
		MessageTransport(const std::string& url);
		~MessageTransport() override;

		// IMessageTransportor

	    void addListener(std::shared_ptr<IMessageTransportListener> listener) override;

		void removeListener(std::shared_ptr<IMessageTransportListener> listener) override;

		void connect(const std::string& url) override;

		void disconnect() override;

		void send(const std::string& data, std::shared_ptr<JCHandler> handler) override;
		
		void send(const std::vector<uint8_t>& data, std::shared_ptr<JCHandler> handler) override;

	protected:
		// IConnectionListener implement
		void onOpen() override;

		void onFail(int errorCode, const std::string& reason) override;

		void onClose(int closeCode, const std::string& reason) override;

		bool onValidate() override;

		void onTextMessage(const std::string& text) override;

		void onBinaryMessage(const std::vector<uint8_t>& data) override;

		bool onPing(const std::string& text) override;

		void onPong(const std::string& text) override;

		void onPongTimeout(const std::string& text) override;

	private:
		bool isValid();

	private:
		std::string _url;

		int _connectionId = -1;

		std::shared_ptr<WebsocketEndpoint> _websocket;

		std::mutex _callbackMutex;
		std::unordered_map<std::string, std::shared_ptr<JCCallback>> _callbacksMap;

		std::mutex _listenerMutex;
		std::vector<std::weak_ptr<IMessageTransportListener>> _listeners;
	};
}
