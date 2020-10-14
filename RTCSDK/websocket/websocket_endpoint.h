/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "connection_metadata.h"
#include <websocketpp/config/asio_no_tls_client.hpp>
//#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
#include <string>
#include <vector>

namespace vi {
	class WebsocketEndpoint {
	public:
		WebsocketEndpoint();

		~WebsocketEndpoint();

		int connect(std::string const& uri, std::shared_ptr<IConnectionListener> listener, const std::string& subprotocol = "");

		void close(int id, websocketpp::close::status::value code, const std::string& reason);

		void sendText(int id, const std::string& data);

		void sendBinary(int id, const std::vector<uint8_t>& data);

		void sendPing(int id, const std::string& data);

		void sendPong(int id, const std::string& data);

		ConnectionMetadata::ptr getMetadata(int id) const;

	private:
		typedef std::map<int, ConnectionMetadata::ptr> ConnectionList;

		client _endpoint;
		websocketpp::lib::shared_ptr<websocketpp::lib::thread> _thread;

		ConnectionList _connectionList;
		int _nextId;
	};
}

