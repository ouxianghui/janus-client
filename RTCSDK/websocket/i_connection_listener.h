/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <string>
#include <vector>

namespace vi {
	class IConnectionListener {
	public:
		virtual ~IConnectionListener() {}

		virtual void onOpen() = 0;

		virtual void onFail(int errorCode, const std::string& reason) = 0;

		virtual void onClose( int closeCode, const std::string& reason) = 0;

		virtual bool onValidate() = 0;

		virtual void onTextMessage(const std::string& text) = 0;

		virtual void onBinaryMessage(const std::vector<uint8_t>& data) = 0;

		virtual bool onPing(const std::string& text) = 0;

		virtual void onPong(const std::string& text) = 0;

		virtual void onPongTimeout(const std::string& text) = 0;
	};
}