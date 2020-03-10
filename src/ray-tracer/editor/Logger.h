#pragma once
#include <spdlog/spdlog.h>
#include <thirdparty/glm/glm/gtx/string_cast.hpp>

namespace Chroma {
	class Logger

	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }
	private:
		static std::shared_ptr<spdlog::logger> s_logger;
	};
}
#define CH_TRACE(...)    ::Chroma::Logger::GetLogger()->trace(__VA_ARGS__)
#define CH_INFO(...)     ::Chroma::Logger::GetLogger()->info(__VA_ARGS__)
#define CH_WARN(...)     ::Chroma::Logger::GetLogger()->warn(__VA_ARGS__)
#define CH_ERROR(...)    ::Chroma::Logger::GetLogger()->error(__VA_ARGS__)
#define CH_FATAL(...)    ::Chroma::Logger::GetLogger()->critical(__VA_ARGS__)
#define CH_ASSERT(x, ...) { if(!(x)) { CH_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
