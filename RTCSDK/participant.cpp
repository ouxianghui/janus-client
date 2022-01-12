/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "participant.h"
#include "logger/logger.h"
#include "video_room_models.h"

namespace vi {
	Participant::Participant(int64_t id, const vr::Publisher& context)
		: _id(id)
		, _context(context)
	{

	}

	Participant::~Participant()
	{

	}
}