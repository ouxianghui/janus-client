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
	class VideoTrackInterface;
}

namespace vi {
	class Participant;
	class IVideoRoomEventHandler {
	public:
		virtual ~IVideoRoomEventHandler() {}

		virtual void onCreateRoom(int32_t errorCode) = 0;

		virtual void onJoinRoom(int32_t errorCode) = 0;

		virtual void onLeaveRoom(int32_t errorCode) = 0;

	};
}