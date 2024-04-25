#pragma once
#include<memory>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <iostream>

namespace NESI_NEXT
{
	class Log
	{
	public:
        //inline static const std::string LOG_FORMAT = "[%Y-%m-%d %H:%M:%S] [%l] %v";
        inline static const std::string LOG_FORMAT = "[%Y-%m-%d %H:%M:%S] [%l] [Thread %t] %v %$";

		static void init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
        static void setLevel(std::string str_level) {
            spdlog::level::level_enum level = spdlog::level::info;
            if (str_level == "trace") {
                level = spdlog::level::trace;
            }
            else if (str_level == "debug") {
                level = spdlog::level::debug;
            }
            else if (str_level == "info") {
                level = spdlog::level::info;
            }
            else if (str_level == "warn") {
                level = spdlog::level::warn;
            }
            else if (str_level == "error") {
                level = spdlog::level::err;
            }
            else if (str_level == "critical") {
                level = spdlog::level::critical;
            }
            else {
                std::cerr << "Unknown log level: " << str_level << std::endl;
                std::cerr << "Proceeding with log level: " << level << std::endl;
            }
            spdlog::set_level(level);
        }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};
}

// core log macros
#define NESI_CORE_ERROR(...)  ::NESI_NEXT::Log::GetCoreLogger()->error(__VA_ARGS__)
#define NESI_CORE_WARN(...)	::NESI_NEXT::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define NESI_CORE_TRACE(...)	::NESI_NEXT::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define NESI_CORE_INFO(...)	::NESI_NEXT::Log::GetCoreLogger()->info(__VA_ARGS__)
#define NESI_CORE_FATAL(...)	::NESI_NEXT::Log::GetCoreLogger()->critical(__VA_ARGS__)

// client log macros		 
#define NESI_ERROR(...)		::NESI_NEXT::Log::GetClientLogger()->error(__VA_ARGS__)
#define NESI_WARN(...)		::NESI_NEXT::Log::GetClientLogger()->warn(__VA_ARGS__)
#define NESI_TRACE(...)		::NESI_NEXT::Log::GetClientLogger()->trace(__VA_ARGS__)
#define NESI_INFO(...)		::NESI_NEXT::Log::GetClientLogger()->info(__VA_ARGS__)
#define NESI_FATAL(...)		::NESI_NEXT::Log::GetClientLogger()->critical(__VA_ARGS__)

