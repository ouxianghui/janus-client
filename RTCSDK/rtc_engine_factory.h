#pragma once

#include <memory>

namespace vi {
	class IRTCEngine;
	class RTCEngineFactory
	{
	public:
		static std::shared_ptr<IRTCEngine> createEngine();
	};
}

