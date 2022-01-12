/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "plugin_client.h"
#include "i_video_room_event_handler.h"
#include "video_room_models.h"

namespace vi {
	class Participant
	{
	public:
		Participant(int64_t id, const vr::Publisher& context);

		~Participant();

		uint64_t id() { return _id; }

		vr::Publisher& context() { return _context; }

	private:
		int64_t _id;

		vr::Publisher _context;
	};
}


