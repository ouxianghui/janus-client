#pragma once

#include <stdint.h>

namespace vi {
	enum class SessionStatus : uint32_t {
		CONNECTED = 0,
		DISCONNECTED
	};
}