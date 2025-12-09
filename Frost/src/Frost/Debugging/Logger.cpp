#include "Frost/Debugging/Logger.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/wincolor_sink.h>

namespace Frost
{
    constexpr const char* LOG_FILE_NAME = "Frost.log";
    constexpr const char* ENGINE_LOGGER_NAME = "FROST";
    constexpr const char* GAME_LOGGER_NAME = "GAME";

    std::shared_ptr<spdlog::logger> Logger::_engineLogger;
    std::shared_ptr<spdlog::logger> Logger::_gameLogger;

    void Logger::Init()
    {
        auto console_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
        console_sink->set_pattern("%^[%H:%M:%S.%e] [%n]: %v%$");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(LOG_FILE_NAME, true);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%l] [%n]: %v");

        std::vector<spdlog::sink_ptr> engine_sinks = { console_sink, file_sink };
        std::vector<spdlog::sink_ptr> game_sinks = { console_sink };

        _engineLogger = std::make_shared<spdlog::logger>(ENGINE_LOGGER_NAME, begin(engine_sinks), end(engine_sinks));
        spdlog::register_logger(_engineLogger);
        _engineLogger->set_level(spdlog::level::trace);
        _engineLogger->flush_on(spdlog::level::trace);

        _gameLogger = std::make_shared<spdlog::logger>(GAME_LOGGER_NAME, begin(game_sinks), end(game_sinks));
        spdlog::register_logger(_gameLogger);
        _gameLogger->set_level(spdlog::level::trace);
        _gameLogger->flush_on(spdlog::level::trace);

        spdlog::set_default_logger(_engineLogger);
    }

    std::shared_ptr<spdlog::logger>& Logger::GetEngineLogger()
    {
        return _engineLogger;
    }

    std::shared_ptr<spdlog::logger>& Logger::GetGameLogger()
    {
        return _gameLogger;
    }
} // namespace Frost
