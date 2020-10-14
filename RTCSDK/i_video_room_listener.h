/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <string>

namespace vi {
	class Participant;
	class IVideoRoomListener {
	public:
		virtual ~IVideoRoomListener() {}

		virtual void onCreateParticipant(std::shared_ptr<Participant> participant) = 0;

		virtual void onUpdateParticipant(std::shared_ptr<Participant> participant) = 0;

		virtual void onDeleteParticipant(std::shared_ptr<Participant> participant) = 0;

		virtual void onCreateStream(int64_t pid, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) = 0;

		virtual void onDeleteStream(int64_t pid, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) = 0;
	};
}