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

		virtual void onCreate() = 0;

		virtual void onJoin() = 0;

		virtual void onLeave() = 0;

	};
}