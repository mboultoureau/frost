#pragma once

#include <spdlog/spdlog.h>

#include <memory>

namespace Frost
{
	class Logger
	{
	public:
		static void Init();

		static const std::shared_ptr<spdlog::logger>& GetEngineLogger() { return s_EngineLogger; }
		static const std::shared_ptr<spdlog::logger>& GetGameLogger() { return s_GameLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_EngineLogger;
		static std::shared_ptr<spdlog::logger> s_GameLogger;
	};

#ifdef FT_DEBUG
	template <typename... Args>
	void FT_ENGINE_TRACE(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Logger::GetEngineLogger()->trace(fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void FT_ENGINE_INFO(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Logger::GetEngineLogger()->info(fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void FT_ENGINE_WARN(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Logger::GetEngineLogger()->warn(fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void FT_ENGINE_ERROR(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Logger::GetEngineLogger()->error(fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void FT_ENGINE_CRITICAL(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Logger::GetEngineLogger()->critical(fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void FT_TRACE(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Logger::GetGameLogger()->trace(fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void FT_INFO(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Logger::GetGameLogger()->info(fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void FT_WARN(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Logger::GetGameLogger()->warn(fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void FT_ERROR(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Logger::GetGameLogger()->error(fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void FT_CRITICAL(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Logger::GetGameLogger()->critical(fmt, std::forward<Args>(args)...);
	}

#else

	template <typename... Args>
	void FT_ENGINE_TRACE(fmt::format_string<Args...> fmt, Args&&... args) {}
	
	template <typename... Args>
	void FT_ENGINE_INFO(fmt::format_string<Args...> fmt, Args&&... args) {}

	template <typename... Args>
	void FT_ENGINE_WARN(fmt::format_string<Args...> fmt, Args&&... args) {}

	template <typename... Args>
	void FT_ENGINE_ERROR(fmt::format_string<Args...> fmt, Args&&... args) {}

	template <typename... Args>
	void FT_ENGINE_CRITICAL(fmt::format_string<Args...> fmt, Args&&... args) {}

	template <typename... Args>
	void FT_TRACE(fmt::format_string<Args...> fmt, Args&&... args) {}

	template <typename... Args>
	void FT_INFO(fmt::format_string<Args...> fmt, Args&&... args) {}

	template <typename... Args>
	void FT_WARN(fmt::format_string<Args...> fmt, Args&&... args) {}

	template <typename... Args>
	void FT_ERROR(fmt::format_string<Args...> fmt, Args&&... args) {}

	template <typename... Args>
	void FT_CRITICAL(fmt::format_string<Args...> fmt, Args&&... args) {}

#endif
}

