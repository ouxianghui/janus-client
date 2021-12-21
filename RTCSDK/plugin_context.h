/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <map>
#include "signaling_client.h"
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/dtmf_sender_interface.h"
#include <functional>
#include "absl/types/optional.h"
#include "webrtc_utils.h"
#include "signaling_events.h"
#include "signaling_client_interface.h"
#include "video_capture.h"

namespace vi {

	using CreateOfferAnswerCallback = std::function<void(bool success, const std::string& reason, const JsepConfig& jsep)>;

	struct PluginContext {
		std::string plugin;
		std::string opaqueId;
		int64_t handleId = -1;
		std::string handleToken;
		std::atomic_bool detached = false;
		std::weak_ptr<SignalingClientInterface> signalingService;


		bool unifiedPlan = true;

		std::vector<std::string> iceServers;

		static rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf;
		static std::unique_ptr<rtc::Thread> signaling;
		static std::unique_ptr<rtc::Thread> worker;
		static std::unique_ptr<rtc::Thread> network;

		std::shared_ptr<CreateOfferAnswerCallback> offerAnswerCallback;

		absl::optional<bool> trickle = true;
		std::atomic_bool iceDone = false;
		std::atomic_bool sdpSent = false;
		std::atomic_bool streamExternal = false;

		absl::optional<JsepConfig> localSdp;
		absl::optional<JsepConfig> remoteSdp;
		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc;
		std::map<std::string, rtc::scoped_refptr<webrtc::DataChannelInterface>> dataChannels;
		std::map<std::string, std::shared_ptr<DCObserver>> dataChannelObservers;
		rtc::scoped_refptr<webrtc::DtmfSenderInterface> dtmfSender;
		std::unique_ptr<DtmfObserver> dtmfObserver;
		std::vector<std::shared_ptr<webrtc::IceCandidateInterface>> candidates;
		rtc::scoped_refptr<StatsObserver> statsObserver;

		rtc::scoped_refptr<webrtc::MediaStreamInterface> localStream;
		rtc::scoped_refptr<webrtc::MediaStreamInterface> remoteStream;

		PluginContext(std::weak_ptr<SignalingClientInterface> ss) : signalingService(ss) {}

		~PluginContext() {
			pcf = nullptr; 
		}
	};
}



