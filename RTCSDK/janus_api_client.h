/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <unordered_map>
#include "i_sfu_api_client.h"
#include "Websocket/websocket_endpoint.h"
#include "i_message_transport_listener.h"
#include "service/observable.h"

namespace rtc {
	class Thread;
}

namespace vi {
	class IMessageTransportor;
	class JanusApiClient
		: public ISfuApiClient
		, public IMessageTransportListener
		, public core::Observable
		, public std::enable_shared_from_this<JanusApiClient>
	{
	public:
		JanusApiClient(rtc::Thread* callbackThread = nullptr);

		~JanusApiClient() override;

		void setToken(const std::string& token);

		void setApiSecret(const std::string& apisecret);

		// ISfuApiClient
		void addListener(std::shared_ptr<ISfuApiClientListener> listener) override;

		void removeListener(std::shared_ptr<ISfuApiClientListener> listener) override;

		void init() override;

		void connect(const std::string& url) override;

		void createSession(std::shared_ptr<JCCallback> callback) override;

		void destroySession(int64_t sessionId, std::shared_ptr<JCCallback> callback) override;

		void reconnectSession(int64_t sessionId, std::shared_ptr<JCCallback> callback) override;

		void keepAlive(int64_t sessionId, std::shared_ptr<JCCallback> callback) override;

		void attach(int64_t sessionId, const std::string& plugin, const std::string& opaqueId, std::shared_ptr<JCCallback> callback) override;

		void detach(int64_t sessionId, int64_t handleId, std::shared_ptr<JCCallback> callback) override;

		void sendMessage(int64_t sessionId, int64_t handleId, const std::string& message, const std::string& jsep, std::shared_ptr<JCCallback> callback) override;

		void sendTrickleCandidate(int64_t sessionId, int64_t handleId, const CandidateData& candidate, std::shared_ptr<JCCallback> callback) override;

		void hangup(int64_t sessionId, int64_t handleId, std::shared_ptr<JCCallback> callback) override;

	protected:
		// IMessageTransportListener
		void onOpened() override;

		void onClosed() override;

		void onFailed(int errorCode, const std::string& reason) override;

		void onMessage(const std::string& json) override;

	private:
		std::shared_ptr<JCCallback> wrapAsyncCallback(std::shared_ptr<JCCallback> callback);

	private:
		std::string _url;
		rtc::Thread* _thread;
		std::string _token;
		std::string _apisecret;
		std::shared_ptr<IMessageTransport> _transport;
		std::vector<std::weak_ptr<ISfuApiClientListener>> _listeners;
	};
}