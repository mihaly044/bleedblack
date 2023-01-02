#include "pch.h"

#include <memory>

#include "cmdline.hpp"
#include "context.hpp"
#include <wil/common.h>

std::shared_ptr<BLIPCSRV_CTX> IpcInitialize()
{
	auto ctx = CommandLineToContext();
	if (!ctx)
		return nullptr;

	// TODO: use unique_mapview_ptr here
	ctx->sharedData = static_cast<BLEEDBLACK_INPUT_REQUEST*>(MapViewOfFile(ctx->hShmMapping,
		FILE_MAP_ALL_ACCESS, 0, 0, sizeof BLEEDBLACK_INPUT_REQUEST));

	if (!LOG_LAST_ERROR_IF_NULL(ctx->sharedData))
		return nullptr;
	
	return ctx;
}
