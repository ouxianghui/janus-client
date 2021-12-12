/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "api/peer_connection_interface.h"
#include "i_sfu_api_client.h"
#include "i_sfu_api_client_listener.h"
#include "signaling_events.h"
#include "signaling_service_interface.h"
#include "utils/universal_observable.hpp"
#include "signaling_service_status.h"
#include "i_signaling_service_observer.h"

namespace rtc {
	class Thread;
}

namespace vi {
	class TaskScheduler;
	class CapturerTrackSource;
	class PluginClient;
	class SignalingService
		: public SignalingServiceInterface
		, public ISfuApiClientListener
		, public UniversalObservable<ISignalingServiceObserver>
		, public std::enable_shared_from_this<SignalingService>
	{
	public:
		SignalingService();

		~SignalingService() override;

		void init() override;

		void cleanup() override;

		void registerObserver(std::shared_ptr<ISignalingServiceObserver> observer) override;

		void unregisterObserver(std::shared_ptr<ISignalingServiceObserver> observer) override;

		SessionStatus sessionStatus() override;

		void connect(const std::string& url) override;

	protected:

		void attach(const std::string& plugin, const std::string& opaqueId, std::shared_ptr<PluginClient> pluginClient) override;

		void destroy(std::shared_ptr<DestroySessionEvent> event) override;

		void sendMessage(int64_t handleId, std::shared_ptr<MessageEvent> event) override;

		void sendTrickleCandidate(int64_t handleId, std::shared_ptr<TrickleCandidateEvent> event) override;

		void hangup(int64_t handleId, bool sendRequest) override;

		void detach(int64_t handleId, std::shared_ptr<DetachEvent> event) override;

	protected:
		// ISfuApiClientListener

	    void onOpened() override;

		void onClosed() override;

		void onFailed(int errorCode, const std::string& reason) override;

		void onMessage(const std::string& json) override;

	private:

		void destroyHandle(int64_t handleId, std::shared_ptr<DetachEvent> event);

	private:
		void createSession(std::shared_ptr<CreateSessionEvent> event);

		void reconnectSession();

		void destroySession(std::shared_ptr<DestroySessionEvent> event);

		void startHeartbeat();

		std::shared_ptr<PluginClient> getHandler(int64_t handleId);

	private:
		std::string _server;	

		std::string _token;

		std::string _opaqueId;

		std::string _apiSecret;

		int64_t _sessionId = -1;

		bool _connected = false;

		std::unordered_map<int64_t, std::shared_ptr<PluginClient>> _pluginClientMap;

		std::shared_ptr<ISfuApiClient> _client;

		std::shared_ptr<TaskScheduler> _heartbeatTaskScheduler;

		uint64_t _heartbeatTaskId = 0;

		SessionStatus _sessionStatus = SessionStatus::DISCONNECTED;

		rtc::Thread* _eventHandlerThread;
	};
}

