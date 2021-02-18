/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <string>
#include "x2struct.hpp"

namespace vi {
	using namespace x2struct;

	struct JanusRequest {
		std::string janus;
		std::string token;
		std::string apisecret;
		std::string transaction;

		XTOSTRUCT(O(janus, token, apisecret, transaction));
	};

	struct JanusResponse {
		std::string janus;
		std::string transaction;		
		int64_t session_id = -1;
		int64_t sender = -1;
		XTOSTRUCT(O(janus, transaction, session_id, sender));
	};

	struct Jsep {
		std::string type;
		std::string sdp;

		XTOSTRUCT(O(type, sdp));
	};

	struct JanusError {
		int64_t code;
		std::string reason;

		XTOSTRUCT(O(code), O(reason));
	};

	struct JanusData {
		std::string videoroom;

		XTOSTRUCT(O(videoroom));
	};

	struct PluginData {
		std::string plugin;
		JanusData data;

		XTOSTRUCT(O(plugin, data));
	};

	struct ReconnectRequest : public JanusRequest {
		int64_t session_id;

		XTOSTRUCT(I(JanusRequest), O(session_id));
	};

	struct KeepAliveRequest : public JanusRequest {
		int64_t session_id;

		XTOSTRUCT(I(JanusRequest), O(session_id));
	};

	struct AttachRequest : public JanusRequest {
		int64_t session_id;
		std::string plugin;
		std::string opaque_id;
		XTOSTRUCT(I(JanusRequest), O(session_id, plugin, opaque_id));
	};

	struct AttachData {
		int64_t id;
		XTOSTRUCT(O(id));
	};

	struct AttachResponse : public JanusResponse {
		AttachData data;
		XTOSTRUCT(I(JanusResponse), O(data));
	};

	struct DetachRequest : public JanusRequest {
		int64_t session_id;
		int64_t handle_id;
		XTOSTRUCT(I(JanusRequest), O(session_id, handle_id));
	};

	struct HangupRequest : public JanusRequest {
		int64_t session_id;
		int64_t handle_id;
		XTOSTRUCT(I(JanusRequest), O(session_id, handle_id));
	};

	struct HangupResponse : public JanusResponse {
		std::string reason;
		XTOSTRUCT(I(JanusResponse), O(reason));
	};

	struct MediaResponse : public JanusResponse {
		std::string type;
		bool receiving;
		std::string mid;
		XTOSTRUCT(I(JanusResponse), O(type, receiving, mid));
	};

	struct SlowlinkResponse : public JanusResponse {
		bool uplink;
		bool lost;
		std::string mid;
		XTOSTRUCT(I(JanusResponse), O(uplink, lost, mid));
	};

	struct JanusEvent : public JanusResponse {
		PluginData plugindata;
		Jsep jsep;
		XTOSTRUCT(I(JanusResponse), O(plugindata, jsep));
	};

	struct DestroyRequest : public JanusRequest {
		int64_t session_id;

		XTOSTRUCT(I(JanusRequest), O(session_id));
	};

	struct CreateSessionData {
		int64_t id;
		XTOSTRUCT(O(id));
	};

	struct CreateSessionResponse : public JanusResponse {
		CreateSessionData data;
		XTOSTRUCT(I(JanusResponse), O(data));
	};

	struct CandidateData {
		std::string candidate;
		std::string sdpMid;
		int64_t sdpMLineIndex;
		bool completed;
		XTOSTRUCT(O(candidate, sdpMid, sdpMLineIndex, completed));
	};

	struct TrickleRequest : public JanusRequest {
		int64_t session_id;
		int64_t handle_id;
		CandidateData candidate;
		XTOSTRUCT(I(JanusRequest), O(session_id, handle_id, candidate));
	};

	struct TrickleResponse : public JanusResponse {
		CandidateData candidate;
		XTOSTRUCT(I(JanusResponse), O(candidate));

	};
	struct MessageRequest : public JanusRequest {
		int64_t session_id;
		int64_t handle_id;
		std::string body;
		XTOSTRUCT(I(JanusRequest), O(session_id, handle_id, body));
	};

	struct JsepRequest : public JanusRequest {
		int64_t session_id;
		int64_t handle_id;
		std::string body;
		std::string jsep;
		XTOSTRUCT(I(JanusRequest), O(session_id, handle_id, body, jsep));
	};
}