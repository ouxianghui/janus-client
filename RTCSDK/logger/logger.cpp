/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "logger.h"
#include <memory>
#include "spdlog/cfg/env.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace vi {

	void Logger::startup()
	{
		//spdlog::cfg::load_env_levels();

		std::string pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] (%!) %v");
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_level(spdlog::level::trace);
		consoleSink->set_pattern(pattern);

		auto sdkFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/sdk_log.txt", 1024 * 1024 * 5, 3);
		sdkFileSink->set_level(spdlog::level::trace);
		sdkFileSink->set_pattern(pattern);

		spdlog::sinks_init_list sinks{ consoleSink, sdkFileSink };
		_sdkLogger = std::make_shared<spdlog::logger>("sdk", sinks);
		_sdkLogger->set_level(spdlog::level::trace);
		_sdkLogger->set_pattern(pattern);

		auto rtcFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/rtc_log.txt", 1024 * 1024 * 5, 3);
		rtcFileSink->set_level(spdlog::level::trace);

		_rtcLogger = std::make_shared<spdlog::logger>("rtc", rtcFileSink);
		_rtcLogger->set_level(spdlog::level::trace);
	}

	void Logger::shutdown()
	{
		spdlog::shutdown();
	}

	std::shared_ptr<spdlog::logger>& Logger::sdkLogger()
	{
		return _sdkLogger;
	}

	std::shared_ptr<spdlog::logger>& Logger::rtcLogger()
	{
		return _rtcLogger;
	}

}