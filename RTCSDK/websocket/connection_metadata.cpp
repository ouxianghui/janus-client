/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "connection_metadata.h"
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <malloc.h>
#include "logger/logger.h"

namespace vi {
	ConnectionMetadata::ConnectionMetadata(int id, websocketpp::connection_hdl hdl, const std::string& uri, std::shared_ptr<IConnectionListener> listener)
		: _id(id)
		, _hdl(hdl)
		, _status("Connecting")
		, _uri(uri)
		, _server("N/A")
		, _listener(listener)
	{}

	void ConnectionMetadata::onOpen(client* c, websocketpp::connection_hdl hdl) {
		_status = "Open";

		client::connection_ptr con = c->get_con_from_hdl(hdl);
		_server = con->get_response_header("Server");
		if (auto listener = _listener.lock()) {
			listener->onOpen();
		}
	}

	void ConnectionMetadata::onFail(client* c, websocketpp::connection_hdl hdl) {
		_status = "Failed";

		client::connection_ptr con = c->get_con_from_hdl(hdl);
		_server = con->get_response_header("Server");
		_errorReason = con->get_ec().message(); 
		int errorCode = con->get_ec().value();
		if (auto listener = _listener.lock()) {
			listener->onFail(errorCode, _errorReason);
		}
	}

	void ConnectionMetadata::onClose(client* c, websocketpp::connection_hdl hdl) {
		_status = "Closed";
		client::connection_ptr con = c->get_con_from_hdl(hdl);
		std::stringstream s;
		s << "close code: " << con->get_remote_close_code() << " (" << websocketpp::close::status::get_string(con->get_remote_close_code()) << "), close reason: " << con->get_remote_close_reason();
		_errorReason = s.str();

		int closeCode = con->get_remote_close_code();
		if (auto listener = _listener.lock()) {
			listener->onClose(closeCode, _errorReason);
		}
	}

	bool ConnectionMetadata::onValidate(client* c, websocketpp::connection_hdl hdl)
	{
		if (auto listener = _listener.lock()) {
			listener->onValidate();
		}
		return true;
	}

	void ConnectionMetadata::onMessage(client* c, websocketpp::connection_hdl, client::message_ptr msg) {
		if (auto listener = _listener.lock()) {
			if (msg->get_opcode() == websocketpp::frame::opcode::text) {
				//DLOG("> received text message: {}", msg->get_payload());
				listener->onTextMessage(msg->get_payload());
			} else if (msg->get_opcode() == websocketpp::frame::opcode::binary) {
				//DLOG("> received binary message {}", websocketpp::utility::to_hex(msg->get_payload()));
				std::vector<uint8_t> data(msg->get_payload().begin(), msg->get_payload().end());
				listener->onBinaryMessage(data);
			}
		}
	}

	bool ConnectionMetadata::onPing(client* c, websocketpp::connection_hdl, std::string msg)
	{
		if (auto listener = _listener.lock()) {
			listener->onPing(msg);
		}
		return true;
	}

	void ConnectionMetadata::onPong(client* c, websocketpp::connection_hdl, std::string msg)
	{
		if (auto listener = _listener.lock()) {
			listener->onPong(msg);
		}
	}

	void  ConnectionMetadata::onPongTimeout(client* c, websocketpp::connection_hdl, std::string msg)
	{
		if (auto listener = _listener.lock()) {
			listener->onPongTimeout(msg);
		}
	}

	websocketpp::connection_hdl ConnectionMetadata::getHdl() const {
		return _hdl;
	}

	int ConnectionMetadata::getId() const {
		return _id;
	}

	std::string ConnectionMetadata::getStatus() const {
		return _status;
	}

	std::ostream & operator<< (std::ostream& out, ConnectionMetadata const& data) {
		out << "> URI: " << data._uri << "\n"
			<< "> Status: " << data._status << "\n"
			<< "> Remote Server: " << (data._server.empty() ? "None Specified" : data._server) << "\n"
			<< "> Error/close reason: " << (data._errorReason.empty() ? "N/A" : data._errorReason) << "\n";
		return out;
	}

}
