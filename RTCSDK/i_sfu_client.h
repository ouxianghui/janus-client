/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <string>
#include <functional>
#include "i_message_transport.h"
#include "i_sfu_client_listener.h"

namespace vi {
	class CandidateData;
	class ISFUClient {
	public:
		virtual ~ISFUClient() {}

		virtual void addListener(std::shared_ptr<ISFUClientListener> listener) = 0;

		virtual void removeListener(std::shared_ptr<ISFUClientListener> listener) = 0;

		virtual void init() = 0;

		virtual void createSession(std::shared_ptr<JCCallback> callback) = 0;

		virtual void destroySession(int64_t sessionId, std::shared_ptr<JCCallback> callback) = 0;

		virtual void reconnectSession(int64_t sessionId, std::shared_ptr<JCCallback> callback) = 0;

		virtual void keepAlive(int64_t sessionId, std::shared_ptr<JCCallback> callback) = 0;

		virtual void attach(int64_t sessionId, const std::string& plugin,const std::string& opaqueId, std::shared_ptr<JCCallback> callback) = 0;

		virtual void detach(int64_t sessionId, int64_t handleId, std::shared_ptr<JCCallback> callback) = 0;

		virtual void sendMessage(int64_t sessionId, int64_t handleId, const std::string& message, const std::string& jsep, std::shared_ptr<JCCallback> callback) = 0;

		virtual void sendTrickleCandidate(int64_t sessionId, int64_t handleId, const CandidateData& candidate, std::shared_ptr<JCCallback> callback) = 0;

		virtual void hangup(int64_t sessionId, int64_t handleId, std::shared_ptr<JCCallback> callback) = 0;
	};
}