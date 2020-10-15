/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include "utils/singleton.h"

#if !defined(SPDLOG_ACTIVE_LEVEL)
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include "spdlog/spdlog.h"

namespace vi {

class Logger : public core::Singleton<Logger>
{
public:
	void startup();

	void shutdown();

	std::shared_ptr<spdlog::logger>& logger();

private:
	std::shared_ptr<spdlog::logger> _logger; 
};

}

#define TLOG(...) SPDLOG_LOGGER_TRACE(vi::Logger::instance()->logger(), __VA_ARGS__)
#define DLOG(...) SPDLOG_LOGGER_DEBUG(vi::Logger::instance()->logger(), __VA_ARGS__)
#define ILOG(...) SPDLOG_LOGGER_INFO(vi::Logger::instance()->logger(), __VA_ARGS__)
#define WLOG(...) SPDLOG_LOGGER_WARN(vi::Logger::instance()->logger(), __VA_ARGS__)
#define ELOG(...) SPDLOG_LOGGER_ERROR(vi::Logger::instance()->logger(), __VA_ARGS__)
#define CLOG(...) SPDLOG_LOGGER_CRITICAL(vi::Logger::instance()->logger(), __VA_ARGS__)