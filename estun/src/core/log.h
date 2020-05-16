#pragma once

#include "core/core.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <memory>

namespace estun
{

class Log
{
public:
	static void Init();

	static std::shared_ptr<spdlog::logger> &GetCoreLogger() { return coreLogger; }
	static std::shared_ptr<spdlog::logger> &GetClientLogger() { return clientLogger; }

private:
	static std::shared_ptr<spdlog::logger> coreLogger;
	static std::shared_ptr<spdlog::logger> clientLogger;
};

} // namespace estun

// Core log macros
#define ES_CORE_TRACE(...) ::estun::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ES_CORE_INFO(...) ::estun::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ES_CORE_WARN(...) ::estun::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ES_CORE_ERROR(...) ::estun::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ES_CORE_CRITICAL(...) ::estun::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define ES_TRACE(...) ::estun::Log::GetClientLogger()->trace(__VA_ARGS__)
#define ES_INFO(...) ::estun::Log::GetClientLogger()->info(__VA_ARGS__)
#define ES_WARN(...) ::estun::Log::GetClientLogger()->warn(__VA_ARGS__)
#define ES_ERROR(...) ::estun::Log::GetClientLogger()->error(__VA_ARGS__)
#define ES_CRITICAL(...) ::estun::Log::GetClientLogger()->critical(__VA_ARGS__)