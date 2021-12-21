#pragma once

#include "signaling_client_status.h"

namespace vi {
	
	class ISignalingClientObserver {
	public:
		virtual ~ISignalingClientObserver() = default;

		virtual void onSessionStatus(SessionStatus status) = 0;
	};

}