#include "rtc_engine_factory.h"
#include "service/rtc_engine.h"

namespace vi {
	std::shared_ptr<IRTCEngine> RTCEngineFactory::createEngine()
	{
		return RTCEngine::instance();
	}
}

