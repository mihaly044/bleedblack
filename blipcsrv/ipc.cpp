#include "pch.h"

#include <memory>

#include "cmdline.hpp"
#include "context.hpp"

std::shared_ptr<BLIPCSRV_CTX> IpcInitialize()
{
	auto ctx = CommandLineToContext();
	if (!ctx)
		return nullptr;

	ctx->sharedData = static_cast<BLEEDBLACK_INPUT_REQUEST*>(MapViewOfFile(ctx->hShmMapping,
		FILE_MAP_ALL_ACCESS, 0, 0, sizeof BLEEDBLACK_INPUT_REQUEST));
	if(!ctx->sharedData)
	{
		return nullptr;
	}

	return ctx;
}
