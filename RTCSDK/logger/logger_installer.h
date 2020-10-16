/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-16
 **/

#pragma once

#include <memory>
#include "utils/singleton.h"

namespace vi
{	
	class RTCLogSink;
	class LoggerInstaller : public core::Singleton<LoggerInstaller>
	{
	public:
		LoggerInstaller();

		void install();

		void uninstall();

	private:
		std::unique_ptr<RTCLogSink> _rtcLogSink;
	};

}

