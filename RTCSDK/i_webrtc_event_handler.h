/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <string>
#include "api/peer_connection_interface.h"
#include "api/media_stream_interface.h"
#include "api/stats/rtc_stats_report.h"
#include "plugin_context.h"

namespace vi {
	class IWebRTCEventHandler
	{
	public:
		virtual ~IWebRTCEventHandler() {}

		virtual void onAttached(bool success) = 0;

		virtual void onHangup() = 0;

		virtual void onIceState(webrtc::PeerConnectionInterface::IceConnectionState iceState) = 0;

		virtual void onMediaState(const std::string& media, bool on, const std::string& mid) = 0;

		virtual void onWebrtcState(bool isActive, const std::string& reason) = 0;

		virtual void onSlowLink(bool uplink, bool lost, const std::string& mid) = 0;

		virtual void onMessage(const std::string& data, const std::string& jsep) = 0;

		virtual void onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, bool on) = 0;

		virtual void onRemoteTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, const std::string& mid, bool on) = 0;

		virtual void onData(const std::string& data, const std::string& label) = 0;

		virtual void onDataOpen(const std::string& label) = 0;

		virtual void onCleanup() = 0;

		virtual void onDetached() = 0;

		virtual void onStatsReport(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) = 0;
	};
}
