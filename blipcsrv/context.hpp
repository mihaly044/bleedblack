#pragma once
#include "pch.h"

typedef struct _BLIPCSRV_CTX
{
	HANDLE hBleedblack = nullptr;
	HANDLE hShmMapping = nullptr;
	HANDLE hReq;
	HANDLE hAck;
	PBLEEDBLACK_INPUT_REQUEST sharedData = nullptr;

	~_BLIPCSRV_CTX()
	{
		if (hReq)
			CloseHandle(hReq);

		if (hAck)
			UnmapViewOfFile(hAck);

		if (hShmMapping)
			CloseHandle(hShmMapping);
	}
} BLIPCSRV_CTX, * PBLIPCSRV_CTX;
