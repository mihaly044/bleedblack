// blipcsrv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include "cmdline.hpp"
#include "ipc.hpp"

int APIENTRY wWinMain(
	_In_		HINSTANCE hInstance,
	_In_opt_	HINSTANCE hPrevInstance,
	_In_		LPWSTR lpCmdLine,
	_In_		INT nCmdShow)
{
	CreateDirectory(L"logs", nullptr);
	const auto max_size = 1048576 * 5;
	const auto max_files = 3;
	plog::init(plog::info, "logs\\blipcsrv.log", max_size, max_files);
	
	PLOG_INFO << "Starting up";
	
	ClearCommandLineArgs();

	auto context = IpcInitialize();
	context->hBleedblack = Bleedblack_Create();

	IpcRunSynchronous(context.get(), [context]() -> bool
	{
		const auto status = Bleedblack_Process(context->hBleedblack, context->sharedData);
		if(!NT_SUCCESS(status))
		{
			PLOG_WARNING.printf("Bleedblack_Process status %08x", status);
		}

		return true;
		
	}, INFINITE);


	Bleedblack_Destroy(context->hBleedblack);
	return EXIT_SUCCESS;
}
