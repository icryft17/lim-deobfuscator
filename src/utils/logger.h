#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#define LOG_DEBUG(msg, ...) spdlog::debug(msg, __VA_ARGS__)
#define LOG_INFO(msg, ...) spdlog::info(msg, __VA_ARGS__)
#define LOG_WARN(msg, ...) spdlog::warn(msg, __VA_ARGS__)

#endif