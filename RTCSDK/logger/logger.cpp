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

	//std::shared_ptr<spdlog::logger> Logger::_logger;
	void Logger::startup()
	{
		//spdlog::cfg::load_env_levels();

		std::string pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#, %!] %v");

		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_level(spdlog::level::trace);
		consoleSink->set_pattern(pattern);

		auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/rtc_log.txt", 1024 * 1024 * 5, 3);
		fileSink->set_level(spdlog::level::trace);
		fileSink->set_pattern(pattern);

		spdlog::sinks_init_list sinks{ consoleSink, fileSink };
		_logger = std::make_shared<spdlog::logger>("rct_logger", sinks);
		_logger->set_level(spdlog::level::debug);
		_logger->set_pattern(pattern);
	}

	void Logger::shutdown()
	{
		spdlog::shutdown();
	}

	std::shared_ptr<spdlog::logger>& Logger::logger()
	{
		return _logger;
	}

}