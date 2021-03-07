// blipcsrv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>

#include <bleedblack/io.h>
#include <bleedblack/bleedblack.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

class BLEEDBLACK_API CMouseClass
{
public:
	NTSTATUS ProcessRequest(PBLEEDBLACK_INPUT_REQUEST pRequest) const;
};

CMouseClass* g_pMouseClass = nullptr;
HANDLE g_hShmMapping = nullptr;
PBLEEDBLACK_IPC g_ipc = nullptr;

void handle_exit()
{
	if (g_ipc)
		UnmapViewOfFile(g_ipc);

	if (g_hShmMapping)
		CloseHandle(g_hShmMapping);

	if (g_ipc->hReady)
		CloseHandle(g_ipc->hReady);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	const auto plogger = spdlog::rotating_logger_mt("blipcsrv", "bleedblack.log", 1048576 * 5, 3);
	auto* logger = plogger.get();
	spdlog::flush_on(spdlog::level::info);

	logger->info("Starting up");
	
	int result = std::atexit(handle_exit);
	if (result != 0)
	{
		logger->critical("Failed to register exit handler {}", result);
		return EXIT_FAILURE;
	}

	g_pMouseClass = static_cast<CMouseClass*>(Bleedblack_Create());
	if (!g_pMouseClass)
	{
		logger->critical("Failed to initialize bleedblack", result);
		return EXIT_FAILURE;
	}

	g_hShmMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, BLEEDBLACK_IPC_ENDPOINT);
	if (g_hShmMapping == nullptr)
	{
		logger->critical("Failed to open memory mapping {}", GetLastError());
		return EXIT_FAILURE;
	}

	g_ipc = static_cast<BLEEDBLACK_IPC*>(MapViewOfFile(g_hShmMapping,
		FILE_MAP_ALL_ACCESS, 0, 0, sizeof BLEEDBLACK_IPC));

	if (g_ipc == nullptr)
	{
		logger->critical("Failed to map memory {}", GetLastError());
		return EXIT_FAILURE;
	}

	NTSTATUS status;
	while (true)
	{
		result = WaitForSingleObject(g_ipc->hReady, INFINITE);
		if (result != WAIT_OBJECT_0)
		{
			logger->critical("Waiting has failed {}", result);
			return EXIT_FAILURE;
		}

		status = g_pMouseClass->ProcessRequest(&g_ipc->Req);
		if(!NT_SUCCESS(status))
			logger->warn("Waiting has failed {:08x}", status);

		ResetEvent(g_ipc->hReady);
	}

	return EXIT_SUCCESS;
}
