/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-16
 **/

#pragma once

#include "rtc_base/logging.h"

namespace vi
{
	class RTCLogSink : public rtc::LogSink
	{
	public:
		void OnLogMessage(const std::string& msg, rtc::LoggingSeverity severity, const char* tag) override;

		void OnLogMessage(const std::string& message, rtc::LoggingSeverity severity) override;

		void OnLogMessage(const std::string& message) override;
	};

}