/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <string>
#include "api/scoped_refptr.h"

namespace webrtc {
	class MediaStreamInterface;
}

namespace vi {
	class Participant;
	class IVideoRoomListener {
	public:
		virtual ~IVideoRoomListener() {}

		virtual void onMediaState(bool isActive, const std::string& reason) = 0;

		virtual void onCreateParticipant(std::shared_ptr<Participant> participant) = 0;

		virtual void onUpdateParticipant(std::shared_ptr<Participant> participant) = 0;

		virtual void onRemoveParticipant(std::shared_ptr<Participant> participant) = 0;

		virtual void onCreateStream(uint64_t pid, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) = 0;

		virtual void onRemoveStream(uint64_t pid, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) = 0;
	};
}