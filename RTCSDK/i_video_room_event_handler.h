/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <string>
#include "api/scoped_refptr.h"

namespace vi {
	class Participant;

	struct CreateRoomResult {
		absl::optional<int64_t> roomId;
		absl::optional<std::string> description;
		absl::optional<std::string> secret;
		absl::optional<std::string> pin;
	};

	class IVideoRoomEventHandler {
	public:
		virtual ~IVideoRoomEventHandler() {}

		virtual void onCreateRoom(std::shared_ptr<CreateRoomResult> result, int32_t errorCode) = 0;

		virtual void onJoinRoom(int64_t roomId, int32_t errorCode) = 0;

		virtual void onLeaveRoom(int64_t roomId, int32_t errorCode) = 0;

	};
}