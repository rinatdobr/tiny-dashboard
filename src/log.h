#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#define LOG_FUNC_ENTRY() spdlog::trace("{0}", __PRETTY_FUNCTION__)
#define LOG_FUNC_ENTRY_MSG(message) spdlog::trace("{0}:" message, __PRETTY_FUNCTION__)
#define LOG_FUNC_ENTRY_MSG_ARGS(message, ...) \
    spdlog::trace("{0}:" #message, __PRETTY_FUNCTION__, __VA_ARGS__)
