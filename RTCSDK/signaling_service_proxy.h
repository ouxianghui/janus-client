/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "weak_proxy.h"

namespace vi {
	BEGIN_WEAK_PROXY_MAP(SignalingService)
		WEAK_PROXY_THREAD_DESTRUCTOR()
		WEAK_PROXY_METHOD0(void, init)
		WEAK_PROXY_METHOD0(void, cleanup)
		WEAK_PROXY_METHOD1(void, registerObserver, std::shared_ptr<ISignalingServiceObserver>)
		WEAK_PROXY_METHOD1(void, unregisterObserver, std::shared_ptr<ISignalingServiceObserver>)
		WEAK_PROXY_METHOD1(void, connect, const std::string&)
		WEAK_PROXY_METHOD0(SessionStatus, sessionStatus)
		WEAK_PROXY_METHOD3(void, attach, const std::string&, const std::string&, std::shared_ptr<PluginClient>)
		WEAK_PROXY_METHOD1(void, destroy, std::shared_ptr<DestroySessionEvent>)
		WEAK_PROXY_METHOD2(void, sendMessage, int64_t, std::shared_ptr<MessageEvent>)
		WEAK_PROXY_METHOD2(void, sendTrickleCandidate, int64_t, std::shared_ptr<TrickleCandidateEventEvent>)
		WEAK_PROXY_METHOD2(void, hangup, int64_t, bool)
		WEAK_PROXY_METHOD2(void, detach, int64_t, std::shared_ptr<DetachEvent>)
	END_WEAK_PROXY_MAP()
}