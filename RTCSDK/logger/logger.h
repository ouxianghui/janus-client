/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include "utils/singleton.h"
#include "spdlog/spdlog.h"

namespace vi {

class Logger : public core::Singleton<Logger>
{
public:
	static void startup();

	static void shutdown();

	static std::shared_ptr<spdlog::logger>& logger();

private:
	static std::shared_ptr<spdlog::logger> _logger; 
};

}

#define TLOG(...) vi::Logger::logger()->trace(##__VA_ARGS__);
#define DLOG(...) vi::Logger::logger()->debug(##__VA_ARGS__);
#define ILOG(...) vi::Logger::logger()->info(##__VA_ARGS__);
#define WLOG(...) vi::Logger::logger()->warn(##__VA_ARGS__);
#define ELOG(...) vi::Logger::logger()->error(##__VA_ARGS__);
#define CLOG(...) vi::Logger::logger()->critical(##__VA_ARGS__);