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
#include "webrtc_service.h"
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/dtmf_sender_interface.h"
#include <functional>
#include "absl/types/optional.h"
#include "webrtc_utils.h"

namespace vi {

	struct WebrtcContext {
		std::atomic_bool started = false;
		rtc::scoped_refptr<webrtc::MediaStreamInterface> myStream;
		std::atomic_bool streamExternal = false;
		absl::optional<JsepConfig> mySdp;
		absl::optional<JsepConfig> remoteSdp;
		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc;
		std::unique_ptr<PCObserver> pcObserver;
		std::map<std::string, rtc::scoped_refptr<webrtc::DataChannelInterface>> dataChannels;
		std::map<std::string, std::shared_ptr<DCObserver>> dataChannelObservers;
		rtc::scoped_refptr<webrtc::DtmfSenderInterface> dtmfSender;
		std::unique_ptr<DtmfObserver> dtmfObserver;
		absl::optional<bool> trickle = true;
		std::atomic_bool iceDone = false;
		std::atomic_bool sdpSent = false;
		std::vector<std::shared_ptr<webrtc::IceCandidateInterface>> candidates;
		rtc::scoped_refptr<StatsObserver> statsObserver;
	};

	struct PluginContext {
		std::weak_ptr<WebRTCServiceInterface> webrtcService;
		std::string plugin;
		std::string opaqueId;
		int64_t handleId;
		std::string handleToken;
		std::atomic_bool detached = false;
		std::shared_ptr<WebrtcContext> webrtcContext;

		PluginContext(std::weak_ptr<WebRTCServiceInterface> wrs) : webrtcService(wrs)
		{
			webrtcContext = std::make_shared<WebrtcContext>();
		}
	};
}



