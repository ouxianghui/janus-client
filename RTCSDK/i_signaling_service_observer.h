#pragma once

#include "signaling_service_status.h"

namespace vi {
	
	class ISignalingServiceObserver {
	public:
		virtual ~ISignalingServiceObserver() = default;

		virtual void onSessionStatus(SessionStatus status) = 0;
	};

}