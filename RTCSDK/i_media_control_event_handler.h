#pragma once

#include <memory>
#include <string>
#include "api/scoped_refptr.h"

namespace webrtc {
	class VideoTrackInterface;
}

namespace vi {

	class Participant;

	class IMediaControlEventHandler {
	public:
		virtual ~IMediaControlEventHandler() = default;

		virtual void onMediaStatus(bool isActive, const std::string& reason)  {}

		virtual void onCreateVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track) {}

		virtual void onRemoveVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track) {}

		virtual void onLocalAudioMuted(bool muted) {}

		virtual void onLocalVideoMuted(bool muted) {}

		virtual void onRemoteAudioMuted(const std::string& pid, bool muted) {}

		virtual void onRemoteVideoMuted(const std::string& pid, bool muted) {}
	};

}