/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <stdint.h>

namespace vi {
	enum class ServiceStauts : uint8_t {
		UP = 0,
		DOWN,
		RESTARTING
	};

	class IWebRTCServiceListener {
	public:
		virtual ~IWebRTCServiceListener() {}

		virtual void onStatus(ServiceStauts status) = 0;
	};
}