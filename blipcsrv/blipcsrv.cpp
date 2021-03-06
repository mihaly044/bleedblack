// blipcsrv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>

#include <bleedblack/io.h>
#include <bleedblack/bleedblack.h>

class BLEEDBLACK_API CMouseClass
{
public:
	NTSTATUS ProcessRequest(PBLEEDBLACK_INPUT_REQUEST pRequest) const;
};

CMouseClass* g_pMouseClass = nullptr;
HANDLE g_hShmMapping = nullptr;
HANDLE g_hShmEvt = nullptr;
PBLEEDBLACK_IPC g_ipc = nullptr;

void handle_exit()
{
	if (g_ipc)
		UnmapViewOfFile(g_ipc);

	if (g_hShmMapping)
		CloseHandle(g_hShmMapping);

	if (g_hShmEvt)
		CloseHandle(g_hShmEvt);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	int result = std::atexit(handle_exit);
	if (result != 0)
	{
		std::cerr << "Registering exit handler failed (" << result << ")" << std::endl;
		return EXIT_FAILURE;
	}

	g_pMouseClass = static_cast<CMouseClass*>(Bleedblack_Create());
	if (!g_pMouseClass)
	{
		std::cerr << "Bleedblack_Create has failed" << std::endl;
		return EXIT_FAILURE;
	}

	g_hShmMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, BLEEDBLACK_IPC_ENDPOINT);
	if (g_hShmMapping == nullptr)
	{
		std::cerr << "Failed to open file mapping" << std::endl;
		return EXIT_FAILURE;
	}

	g_ipc = static_cast<BLEEDBLACK_IPC*>(MapViewOfFile(g_hShmMapping,
		PAGE_READWRITE, 0, 0, sizeof BLEEDBLACK_IPC));

	if (g_ipc == nullptr)
	{
		std::cerr << "Failed to open file mapping" << std::endl;
		return EXIT_FAILURE;
	}

	g_hShmEvt = OpenEventA(SYNCHRONIZE | EVENT_MODIFY_STATE, 
		FALSE, BLEEDBLACK_IPC_EVT_NAME);
	if (g_hShmEvt == nullptr)
	{
		std::cerr << "Failed to open global event" << std::endl;
		return EXIT_FAILURE;
	}

	while (true)
	{
		result = WaitForSingleObject(g_hShmEvt, INFINITE);
		if (result != WAIT_OBJECT_0)
		{
			std::cerr << "Waiting has failed (" << result << ")" << std::endl;
			return EXIT_FAILURE;
		}

		if (g_ipc->ParentExiting)
			return EXIT_SUCCESS;

		g_pMouseClass->ProcessRequest(&g_ipc->Req);
		ResetEvent(g_hShmEvt);
	}

	return EXIT_SUCCESS;
}
