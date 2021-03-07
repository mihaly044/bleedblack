#include "pch.h"
#include "logging.hpp"

std::shared_ptr<spdlog::logger> logger;

void InitializeLogger()
{
	const auto max_size = 1048576 * 5;
	const auto max_files = 3;
	logger = spdlog::rotating_logger_mt("blipcsrv", "bleedblack.log", max_size, max_files);

	spdlog::flush_every(std::chrono::seconds(5));
	spdlog::flush_on(spdlog::level::warn);

	logger->info("Logger initialized");
}
