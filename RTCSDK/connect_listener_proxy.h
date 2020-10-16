/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-17
 **/

#pragma once

#include "websocket/i_connection_listener.h"
#include "weak_proxy.h"
#include <algorithm>
#include <list>

namespace vi {
	class ConnectionListenerInterface : public IConnectionListener {
	public:
		virtual ~ConnectionListenerInterface() {}

		virtual void attach(std::shared_ptr<IConnectionListener> listener) = 0;

		virtual void detach(std::shared_ptr<IConnectionListener> listener) = 0;

		virtual void detachAll() = 0;

		void onOpen() override {}

		void onFail(int errorCode, const std::string& reason) override {}

		void onClose(int closeCode, const std::string& reason) override {}

		bool onValidate() override { return true; }

		void onTextMessage(const std::string& text) override {}

		void onBinaryMessage(const std::vector<uint8_t>& data) override {}

		bool onPing(const std::string& text) override { return true; }

		void onPong(const std::string& text) override {}

		void onPongTimeout(const std::string& text) override {}
	};

	class ConnectionListener : public ConnectionListenerInterface {
	public:
		~ConnectionListener() {

		}

		void attach(std::shared_ptr<IConnectionListener> listener) override {
			if (!exist(listener)) {
				_listeners.emplace_back(listener);
			}
		}

		void detach(std::shared_ptr<IConnectionListener> listener) override {
			_listeners.remove_if([listener](const std::weak_ptr<IConnectionListener>& l) {
				return listener == l.lock();
			});
		}

		void detachAll() override {
			_listeners.clear();
		}

	private:
		void onOpen() override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onOpen();
				}
			}
		}

		void onFail(int errorCode, const std::string& reason) override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onFail(errorCode, reason);
				}
			}
		}

		void onClose(int closeCode, const std::string& reason) override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onClose(closeCode, reason);
				}
			}
		}

		bool onValidate() override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onValidate();
				}
			}
			return true;
		}

		void onTextMessage(const std::string& text) override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onTextMessage(text);
				}
			}
		}

		void onBinaryMessage(const std::vector<uint8_t>& data) override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onBinaryMessage(data);
				}
			}
		}

		bool onPing(const std::string& text) override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onPing(text);
				}
			}
			return true;
		}

		void onPong(const std::string& text) override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onPong(text);
				}
			}
		}

		void onPongTimeout(const std::string& text) override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onPongTimeout(text);
				}
			}
		}
		
	private:
		bool exist(std::shared_ptr<IConnectionListener> listener) {
			return std::any_of(_listeners.begin(), _listeners.end(), [listener](const std::weak_ptr<IConnectionListener>& l) {
				return listener == l.lock();
			});
		}
	private:
		std::list<std::weak_ptr<IConnectionListener>> _listeners;
	};

	BEGIN_WEAK_PROXY_MAP(ConnectionListener)
		WEAK_PROXY_THREAD_DESTRUCTOR()
		WEAK_PROXY_METHOD1(void, attach, std::shared_ptr<IConnectionListener>)
		WEAK_PROXY_METHOD1(void, detach, std::shared_ptr<IConnectionListener>)
		WEAK_PROXY_METHOD0(void, detachAll)
		WEAK_PROXY_METHOD0(void, onOpen)
		WEAK_PROXY_METHOD2(void, onFail, int, const std::string&)
		WEAK_PROXY_METHOD2(void, onClose, int, const std::string&)
		WEAK_PROXY_METHOD0(bool, onValidate)
		WEAK_PROXY_METHOD1(void, onTextMessage, const std::string&)
		WEAK_PROXY_METHOD1(void, onBinaryMessage, const std::vector<uint8_t>&)
		WEAK_PROXY_METHOD1(bool, onPing, const std::string&)
		WEAK_PROXY_METHOD1(void, onPong, const std::string&)
		WEAK_PROXY_METHOD1(void, onPongTimeout, const std::string&)
	END_WEAK_PROXY_MAP()
}