#include "Frost/Debugging/Logger.h"
#include <spdlog/sinks/wincolor_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Frost
{
	std::shared_ptr<spdlog::logger> Logger::s_EngineLogger = nullptr;
	std::shared_ptr<spdlog::logger> Logger::s_GameLogger = nullptr;

    void Logger::Init()
    {
		auto console_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
		console_sink->set_pattern("%^[%H:%M:%S.%e] [%n]: %v%$");

		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("Frost.log", true);
		file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%l] [%n]: %v");

		std::vector<spdlog::sink_ptr> engine_sinks = { console_sink, file_sink };
		std::vector<spdlog::sink_ptr> game_sinks = { console_sink };

		s_EngineLogger = std::make_shared<spdlog::logger>("FROST", begin(engine_sinks), end(engine_sinks));
		spdlog::register_logger(s_EngineLogger);
		s_EngineLogger->set_level(spdlog::level::trace);
		s_EngineLogger->flush_on(spdlog::level::trace);

		s_GameLogger = std::make_shared<spdlog::logger>("GAME", begin(game_sinks), end(game_sinks));
		spdlog::register_logger(s_GameLogger);
		s_GameLogger->set_level(spdlog::level::trace);
		s_GameLogger->flush_on(spdlog::level::trace);

		spdlog::set_default_logger(s_EngineLogger);
    }
}
