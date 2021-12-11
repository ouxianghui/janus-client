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

	class ISignalingEventHandler
	{
	public:
		virtual ~ISignalingEventHandler() {}

		virtual void onAttached(bool success) = 0;

		virtual void onMediaStatus(const std::string& media, bool on, const std::string& mid) = 0;

		virtual void onWebrtcStatus(bool isActive, const std::string& desc) = 0;

		virtual void onSlowLink(bool uplink, bool lost, const std::string& mid) = 0;

		virtual void onTrickle(const std::string& trickle) = 0;

		virtual void onMessage(const std::string& data, const std::string& jsep) = 0;

		virtual void onTimeout() = 0;

		virtual void onError(const std::string& desc) = 0;

		virtual void onHangup() = 0;

		virtual void onCleanup() = 0;

		virtual void onDetached() = 0;
	};
}
