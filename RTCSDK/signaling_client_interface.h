/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <functional>
#include "signaling_events.h"
#include "service/i_unified_factory.h"
#include "signaling_client_status.h"
#include "weak_proxy.h"

namespace vi {
	class PluginClient;
	class ISignalingClientObserver;
	class SignalingClientInterface
	{
	public:
		virtual ~SignalingClientInterface() {}

		virtual void init() = 0;

		virtual void cleanup() = 0;

		virtual void registerObserver(std::shared_ptr<ISignalingClientObserver> observer) = 0;

		virtual void unregisterObserver(std::shared_ptr<ISignalingClientObserver> observer) = 0;

		virtual SessionStatus sessionStatus() = 0;

		virtual void connect(const std::string& url) = 0;

		virtual void attach(const std::string& plugin, const std::string& opaqueId, std::shared_ptr<PluginClient> pluginClient) = 0;

		virtual void destroy(std::shared_ptr<DestroySessionEvent> event) = 0;

		virtual void sendMessage(int64_t handleId, std::shared_ptr<MessageEvent> event) = 0;

		virtual void sendTrickleCandidate(int64_t handleId, std::shared_ptr<TrickleCandidateEvent> event) = 0;

		virtual void hangup(int64_t handleId, bool sendRequest) = 0;

		virtual void detach(int64_t handleId, std::shared_ptr<DetachEvent> event) = 0;
	};

	BEGIN_WEAK_PROXY_MAP(SignalingClient)
		WEAK_PROXY_THREAD_DESTRUCTOR()
		WEAK_PROXY_METHOD0(void, init)
		WEAK_PROXY_METHOD0(void, cleanup)
		WEAK_PROXY_METHOD1(void, registerObserver, std::shared_ptr<ISignalingClientObserver>)
		WEAK_PROXY_METHOD1(void, unregisterObserver, std::shared_ptr<ISignalingClientObserver>)
		WEAK_PROXY_METHOD1(void, connect, const std::string&)
		WEAK_PROXY_METHOD0(SessionStatus, sessionStatus)
		WEAK_PROXY_METHOD3(void, attach, const std::string&, const std::string&, std::shared_ptr<PluginClient>)
		WEAK_PROXY_METHOD1(void, destroy, std::shared_ptr<DestroySessionEvent>)
		WEAK_PROXY_METHOD2(void, sendMessage, int64_t, std::shared_ptr<MessageEvent>)
		WEAK_PROXY_METHOD2(void, sendTrickleCandidate, int64_t, std::shared_ptr<TrickleCandidateEvent>)
		WEAK_PROXY_METHOD2(void, hangup, int64_t, bool)
		WEAK_PROXY_METHOD2(void, detach, int64_t, std::shared_ptr<DetachEvent>)
	END_WEAK_PROXY_MAP()
}