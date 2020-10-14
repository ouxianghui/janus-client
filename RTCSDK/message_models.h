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

	struct DestroyRequest : public JanusRequest {
		int64_t session_id;

		XTOSTRUCT(I(JanusRequest), O(session_id));
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

	struct RegisterRequest {
		std::string request;
		int64_t room;
		std::string ptype;
		std::string display;
		XTOSTRUCT(O(request, room, ptype, display));
	};

	struct SubscribeRequest {
		std::string request;
		int64_t room;
		std::string ptype;
		int64_t feed;
		int64_t private_id;
		XTOSTRUCT(O(request, room, ptype, feed, private_id));
	};

	struct ConfigAudioVideoRequest {
		std::string request = "configure";
		bool audio;
		bool video;
		XTOSTRUCT(O(request, audio, video));
	};

	struct UnpublishRequest {
		std::string request = "unpublish";
		XTOSTRUCT(O(request));
	};

	struct StartRequest {
		std::string request = "start";
		int64_t room;
		XTOSTRUCT(O(request, room));
	};

	struct Jsep {
		std::string type;
		std::string sdp;
		XTOSTRUCT(O(type, sdp));
	};

	struct ConfigBitrateRequest {
		std::string request;
		int64_t bitrate;
		XTOSTRUCT(O(request, bitrate));
	};

	struct JanusError {
		int64_t code;
		std::string reason;

		XTOSTRUCT(O(code), O(reason));
	};

	struct PublisherData {
		int64_t id;
		std::string display;
		std::string audio_codec;
		std::string video_codec;

		XTOSTRUCT(O(id, display, audio_codec, video_codec));
	};

	struct EventData {
		std::string videoroom;
		int64_t room;
		std::string description;
		int64_t id;
		int64_t private_id;
		std::vector<PublisherData> publishers;
		int64_t unpublished;
		int64_t leaving;
		int64_t error_code;
		std::string audio_codec;
		std::string video_codec;
		std::string display;
		XTOSTRUCT(O(videoroom, room, description, id, private_id, publishers, leaving, error_code, audio_codec, video_codec, display));
	};

	struct PluginData {
		std::string plugin;
		EventData data;
		XTOSTRUCT(O(data, plugin));
	};

	struct JanusData {
		int64_t id;

		XTOSTRUCT(O(id));
	};

	struct JanusResponse {
		std::string janus;
		int64_t session_id = -1;
		int64_t sender = -1;
		CandidateData candidate;
		std::string reason;
		std::string type;
		bool receiving;
		bool uplink;
		bool lost;
		PluginData plugindata;
		JanusError error;
		JanusData data;
		Jsep jsep;
		std::string transaction;

		XTOSTRUCT(O(janus),
			O(session_id),
			O(sender),
			O(candidate),
			O(reason),
			O(type),
			O(receiving),
			O(uplink),
			O(lost),
			O(plugindata),
			O(error),
			O(data),
			O(jsep),
			O(transaction));
	};
}