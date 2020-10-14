/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "i_sfu_client_listener.h"
#include "weak_proxy.h"
#include <algorithm>
#include <list>

namespace vi {
	class SFUListenerInterface : public ISFUClientListener {
	public:
		virtual ~SFUListenerInterface() {}

		virtual void attach(std::shared_ptr<ISFUClientListener> listener) = 0;

		virtual void detach(std::shared_ptr<ISFUClientListener> listener) = 0;

		virtual void detachAll() = 0;

		void onOpened() override {}

		void onClosed() override {}

		void onFailed(int errorCode, const std::string& reason) override {}

		void onMessage(std::shared_ptr<JanusResponse> model) override {}
	};

	class SFUListener : public SFUListenerInterface {
	public:
		~SFUListener() {

		}

		void attach(std::shared_ptr<ISFUClientListener> listener) override {
			if (!exist(listener)) {
				_listeners.emplace_back(listener);
			}
		}

		void detach(std::shared_ptr<ISFUClientListener> listener) override {
			_listeners.remove_if([listener](const std::weak_ptr<ISFUClientListener>& l) {
				return listener == l.lock();
			});
		}

		void detachAll() override {
			_listeners.clear();
		}

	private:
		void onOpened() override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onOpened();
				}
			}
		}

		void onClosed() override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onClosed();
				}
			}
		}

		void onFailed(int errorCode, const std::string& reason) override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onFailed(errorCode, reason);
				}
			}
		}

		void onMessage(std::shared_ptr<JanusResponse> model) override {
			for (const auto& listener : _listeners) {
				if (auto l = listener.lock()) {
					l->onMessage(model);
				}
			}
		}
		
	private:
		bool exist(std::shared_ptr<ISFUClientListener> listener) {
			return std::any_of(_listeners.begin(), _listeners.end(), [listener](const std::weak_ptr<ISFUClientListener>& l) {
				return listener == l.lock();
			});
		}
	private:
		std::list<std::weak_ptr<ISFUClientListener>> _listeners;
	};

	BEGIN_WEAK_PROXY_MAP(SFUListener)
		WEAK_PROXY_THREAD_DESTRUCTOR()
		WEAK_PROXY_METHOD1(void, attach, std::shared_ptr<ISFUClientListener>)
		WEAK_PROXY_METHOD1(void, detach, std::shared_ptr<ISFUClientListener>)
		WEAK_PROXY_METHOD0(void, detachAll)
		WEAK_PROXY_METHOD0(void, onOpened)
		WEAK_PROXY_METHOD0(void, onClosed)
		WEAK_PROXY_METHOD2(void, onFailed, int, const std::string&)
		WEAK_PROXY_METHOD1(void, onMessage, std::shared_ptr<JanusResponse>)
	END_WEAK_PROXY_MAP()
}