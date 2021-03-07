#pragma once
#include <memory>

#include "context.hpp"

std::shared_ptr<BLIPCSRV_CTX> IpcInitialize();

template<typename T>
void IpcRunSynchronous(BLIPCSRV_CTX* ctx, T&& callback, DWORD dwTimeout = INFINITE)
{
	while (true)
	{
		const auto result = WaitForSingleObject(ctx->hReq, dwTimeout);
		if (result != WAIT_OBJECT_0)
		{
			SetEvent(ctx->hAck);
			return;
		}

		if (!callback())
		{
			SetEvent(ctx->hAck);
			return;
		}

		ResetEvent(ctx->hReq);
		SetEvent(ctx->hAck);
	}
}