/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "plugin_client.h"
#include "i_video_room_listener.h"

namespace vi {
	class Participant
	{
	public:
		Participant(int64_t id, const std::string& displayName);

		~Participant();

		uint64_t getId() { return _id; }

		const std::string& displayName() { return _displayName; }

	private:
		int64_t _id;
		std::string _displayName;
	};
}


