/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <string>
#include "json/jsonable.hpp"
#include "absl/types/optional.h"


namespace vi {

	struct JanusRequest {
		absl::optional<std::string> janus;
		absl::optional<std::string> token;
		absl::optional<std::string> apisecret;
		absl::optional<std::string> transaction;

		FIELDS_MAP("janus", janus, "token", token, "apisecret", apisecret, "transaction", transaction);
	};

	struct JanusResponse {
		absl::optional<std::string> janus;
		absl::optional<std::string> transaction;		
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> sender;
		
		FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender);
	};

	struct Jsep {
		absl::optional<std::string> type;
		absl::optional<std::string> sdp;

		FIELDS_MAP("type", type, "sdp", sdp);
	};

	struct JanusError {
		absl::optional<int64_t> code;
		absl::optional<std::string> reason;

		FIELDS_MAP("code", code, "reason", reason);
	};

	struct JanusData {
		absl::optional<std::string> videoroom;

		FIELDS_MAP("videoroom", videoroom);
	};

	struct PluginData {
		absl::optional<std::string> plugin;
		absl::optional<JanusData> data;

		FIELDS_MAP("plugin", plugin, "data", data);
	};

	struct ReconnectRequest {
		absl::optional<std::string> janus;
		absl::optional<std::string> token;
		absl::optional<std::string> apisecret;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;

		FIELDS_MAP("janus", janus, "token", token, "apisecret", apisecret, "transaction", transaction, "session_id", session_id);
	};

	struct KeepAliveRequest {
		absl::optional<std::string> janus;
		absl::optional<std::string> token;
		absl::optional<std::string> apisecret;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;

		FIELDS_MAP("janus", janus, "token", token, "apisecret", apisecret, "transaction", transaction, "session_id", session_id);
	};

	struct AttachRequest {
		absl::optional<std::string> janus;
		absl::optional<std::string> token;
		absl::optional<std::string> apisecret;
		absl::optional<std::string> transaction;

		absl::optional<int64_t> session_id;
		absl::optional<std::string> plugin;
		absl::optional<std::string> opaque_id;

		FIELDS_MAP("janus", janus, "token", token, "apisecret", apisecret, "transaction", transaction, "session_id", session_id, "plugin", plugin, "opaque_id", opaque_id);
	};

	struct AttachData {
		absl::optional<int64_t> id;

		FIELDS_MAP("id", id);
	};

	struct AttachResponse {
		absl::optional<std::string> janus;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> sender;
		absl::optional<AttachData> data;

		FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "data", data);
	};

	struct DetachRequest {
		absl::optional<std::string> janus;
		absl::optional<std::string> token;
		absl::optional<std::string> apisecret;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> handle_id;
		
		FIELDS_MAP("janus", janus, "token", token, "apisecret", apisecret, "transaction", transaction, "session_id", session_id, "handle_id", handle_id);
	};

	struct HangupRequest {
		absl::optional<std::string> janus;
		absl::optional<std::string> token;
		absl::optional<std::string> apisecret;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> handle_id;
		
		FIELDS_MAP("janus", janus, "token", token, "apisecret", apisecret, "transaction", transaction, "session_id", session_id, "handle_id", handle_id);
	};

	struct HangupResponse {
		absl::optional<std::string> janus;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> sender;
		absl::optional<std::string> reason;

		FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "reason", reason);
	};

	struct MediaResponse {
		absl::optional<std::string> janus;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> sender;
		absl::optional<std::string> type;
		absl::optional<bool> receiving;
		absl::optional<std::string> mid;
		
		FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "type", type, "receiving", receiving, "mid", mid);
	};

	struct SlowlinkResponse {
		absl::optional<std::string> janus;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> sender;
		absl::optional<bool> uplink;
		absl::optional<bool> lost;
		absl::optional<std::string> mid;
		
		FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "uplink", uplink, "lost", lost, "mid", mid);
	};

	struct JanusEvent {
		absl::optional<std::string> janus;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> sender;
		absl::optional<PluginData> plugindata;
		absl::optional<Jsep> jsep;
		
		FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "plugindata", plugindata, "jsep", jsep);
	};

	struct DestroyRequest {
		absl::optional<std::string> janus;
		absl::optional<std::string> token;
		absl::optional<std::string> apisecret;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;

		FIELDS_MAP("janus", janus, "token", token, "apisecret", apisecret, "transaction", transaction, "session_id", session_id);
	};

	struct CreateSessionData {
		absl::optional<int64_t> id;
		
		FIELDS_MAP("id", id);
	};

	struct CreateSessionResponse {
		absl::optional<std::string> janus;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> sender;
		absl::optional<CreateSessionData> data;
		
		FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "data", data);
	};

	struct CandidateData {
		absl::optional<std::string> candidate;
		absl::optional<std::string> sdpMid;
		absl::optional<int64_t> sdpMLineIndex;
		absl::optional<bool> completed;
		
		FIELDS_MAP("candidate", candidate, "sdpMid", sdpMid, "sdpMLineIndex", sdpMLineIndex, "completed", completed);
	};

	struct TrickleRequest {
		absl::optional<std::string> janus;
		absl::optional<std::string> token;
		absl::optional<std::string> apisecret;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> handle_id;
		absl::optional<CandidateData> candidate;
		
		FIELDS_MAP("janus", janus, "token", token, "apisecret", apisecret, "transaction", transaction, "session_id", session_id, "handle_id", handle_id, "candidate", candidate);
	};

	struct TrickleResponse {
		absl::optional<std::string> janus;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> sender;
		absl::optional<CandidateData> candidate;
		
		FIELDS_MAP("janus", janus, "transaction", transaction, "session_id", session_id, "sender", sender, "candidate", candidate);

	};
	struct MessageRequest {
		absl::optional<std::string> janus;
		absl::optional<std::string> token;
		absl::optional<std::string> apisecret;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> handle_id;
		absl::optional<std::string> body;
		
		FIELDS_MAP("janus", janus, "token", token, "apisecret", apisecret, "transaction", transaction, "session_id", session_id, "handle_id", handle_id, "body", body);
	};

	struct JsepRequest {
		absl::optional<std::string> janus;
		absl::optional<std::string> token;
		absl::optional<std::string> apisecret;
		absl::optional<std::string> transaction;
		absl::optional<int64_t> session_id;
		absl::optional<int64_t> handle_id;
		absl::optional<std::string> body;
		absl::optional<std::string> jsep;
		
		FIELDS_MAP("janus", janus, "token", token, "apisecret", apisecret, "transaction", transaction, "session_id", session_id, "handle_id", handle_id, "body", body, "jsep", jsep);
	};
}