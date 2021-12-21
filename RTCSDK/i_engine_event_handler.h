#pragma once

#include <memory>
#include <string>

namespace vi {

	enum class EngineStatus {
		CONNECTED = 0,
		DISCONNECTED
	};

	class IEngineEventHandler
	{
	public:
		virtual ~IEngineEventHandler() {}

		virtual void onStatus(EngineStatus status) = 0;

		virtual void onError(int32_t code) = 0;
	};
}
