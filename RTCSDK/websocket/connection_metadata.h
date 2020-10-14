/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include "Websocket/i_connection_listener.h"


typedef websocketpp::client<websocketpp::config::asio_client> client;

namespace vi {

	class ConnectionMetadata {
	public:
		typedef websocketpp::lib::shared_ptr<ConnectionMetadata> ptr;

		ConnectionMetadata(int id, websocketpp::connection_hdl hdl, const std::string& uri, std::shared_ptr<IConnectionListener> listener);

		void onOpen(client* c, websocketpp::connection_hdl hdl);

		void onFail(client* c, websocketpp::connection_hdl hdl);

		void onClose(client* c, websocketpp::connection_hdl hdl);

		bool onValidate(client* c, websocketpp::connection_hdl hdl);

		void onMessage(client* c, websocketpp::connection_hdl, client::message_ptr msg);

		bool onPing(client* c, websocketpp::connection_hdl, std::string msg);

		void onPong(client* c, websocketpp::connection_hdl, std::string msg);

		void onPongTimeout(client* c, websocketpp::connection_hdl, std::string msg);

		websocketpp::connection_hdl getHdl() const;

		int getId() const;

		std::string getStatus() const;

		friend std::ostream & operator<< (std::ostream& out, ConnectionMetadata const& data);
	private:
		int _id;
		websocketpp::connection_hdl _hdl;
		std::string _status;
		std::string _uri;
		std::string _server;
		std::string _errorReason;
		std::weak_ptr<IConnectionListener> _listener;
	};

}


