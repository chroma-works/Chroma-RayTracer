#pragma once
#include <spdlog/spdlog.h>

namespace CH_Editor {
	class Logger

	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }
	private:
		static std::shared_ptr<spdlog::logger> s_logger;
	};
}
#define CH_TRACE(...)    ::CH_Editor::Logger::GetLogger()->trace(__VA_ARGS__)
#define CH_INFO(...)     ::CH_Editor::Logger::GetLogger()->info(__VA_ARGS__)
#define CH_WARN(...)     ::CH_Editor::Logger::GetLogger()->warn(__VA_ARGS__)
#define CH_ERROR(...)    ::CH_Editor::Logger::GetLogger()->error(__VA_ARGS__)
#define CH_FATAL(...)    ::CH_Editor::Logger::GetLogger()->critical(__VA_ARGS__)
#define CH_ASSERT(x, ...) { if(!(x)) { CH_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
