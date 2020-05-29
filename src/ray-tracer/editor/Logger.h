#pragma once
#include <spdlog/spdlog.h>
#include <thirdparty/glm/glm/gtx/string_cast.hpp>

namespace CHR {
	class Logger

	{
	public:
		static void Init(std::string ver = "1.0");

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }
	private:
		static std::shared_ptr<spdlog::logger> s_logger;
	};
}
#define CH_TRACE(...)    ::CHR::Logger::GetLogger()->trace(__VA_ARGS__)
#define CH_INFO(...)     ::CHR::Logger::GetLogger()->info(__VA_ARGS__)
#define CH_WARN(...)     ::CHR::Logger::GetLogger()->warn(__VA_ARGS__)
#define CH_ERROR(...)    ::CHR::Logger::GetLogger()->error(__VA_ARGS__)
#define CH_FATAL(...)    ::CHR::Logger::GetLogger()->critical(__VA_ARGS__)
#define CH_ASSERT(x, ...) { if(!(x)) { CH_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
