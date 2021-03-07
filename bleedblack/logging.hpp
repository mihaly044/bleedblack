#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

extern std::shared_ptr<spdlog::logger> logger;

void InitializeLogger();
