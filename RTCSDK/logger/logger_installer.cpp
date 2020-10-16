/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-16
 **/

#include "logger_installer.h"
#include "logger.h"
#include "rtc_log_sink.h"
#include "rtc_base/logging.h"

namespace vi
{
	LoggerInstaller::LoggerInstaller()
	{
	}

	void LoggerInstaller::install()
	{
		Logger::instance()->startup();

		if (!_rtcLogSink) {
			_rtcLogSink = std::make_unique<RTCLogSink>();
			rtc::LogMessage::SetLogToStderr(false);
			rtc::LogMessage::AddLogToStream(_rtcLogSink.get(), rtc::LS_VERBOSE);
		}
	}

	void LoggerInstaller::uninstall()
	{
		if (_rtcLogSink) {
			rtc::LogMessage::RemoveLogToStream(_rtcLogSink.get());
		}

		Logger::instance()->shutdown();
	}

}