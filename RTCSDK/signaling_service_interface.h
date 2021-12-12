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
#include "signaling_service_status.h"

namespace vi {
	class PluginClient;
	class ISignalingServiceObserver;
	class SignalingServiceInterface
	{
	public:
		virtual ~SignalingServiceInterface() {}

		virtual void init() = 0;

		virtual void cleanup() = 0;

		virtual void registerObserver(std::shared_ptr<ISignalingServiceObserver> observer) = 0;

		virtual void unregisterObserver(std::shared_ptr<ISignalingServiceObserver> observer) = 0;

		virtual SessionStatus sessionStatus() = 0;

		virtual void connect(const std::string& url) = 0;

		virtual void attach(const std::string& plugin, const std::string& opaqueId, std::shared_ptr<PluginClient> pluginClient) = 0;

		virtual void destroy(std::shared_ptr<DestroySessionEvent> event) = 0;

		virtual void sendMessage(int64_t handleId, std::shared_ptr<MessageEvent> event) = 0;

		virtual void sendTrickleCandidate(int64_t handleId, std::shared_ptr<TrickleCandidateEvent> event) = 0;

		virtual void hangup(int64_t handleId, bool sendRequest) = 0;

		virtual void detach(int64_t handleId, std::shared_ptr<DetachEvent> event) = 0;
	};
}