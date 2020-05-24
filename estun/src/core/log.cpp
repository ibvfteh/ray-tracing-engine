#include "core/log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include "spdlog/sinks/rotating_file_sink.h"

namespace estun
{

std::shared_ptr<spdlog::logger> Log::coreLogger;
std::shared_ptr<spdlog::logger> Log::clientLogger;

void Log::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
	sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>("logs/logfile.txt", 1048576 * 5, 3));
	coreLogger = std::make_shared<spdlog::logger>("ESTUN", begin(sinks), end(sinks));
	coreLogger->set_level(spdlog::level::trace);
	coreLogger->set_pattern("%^[%T] %n: %v%$");

	clientLogger = spdlog::stdout_color_mt("APP");
	clientLogger->set_level(spdlog::level::trace);
}

} // namespace estun
