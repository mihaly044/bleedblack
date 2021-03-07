// blipcsrv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "cmdline.hpp"
#include "ipc.hpp"

int APIENTRY wWinMain(
	_In_		HINSTANCE hInstance,
	_In_opt_	HINSTANCE hPrevInstance,
	_In_		LPWSTR lpCmdLine,
	_In_		INT nCmdShow)
{
	auto context = IpcInitialize();

	ClearCommandLineArgs();

	context->hBleedblack = Bleedblack_Create();

	IpcRunSynchronous(context.get(), [context]() -> bool
	{
		const auto status = Bleedblack_Process(context->hBleedblack, context->sharedData);
		if(!NT_SUCCESS(status))
		{
			return false;
		}

		return true;
		
	}, INFINITE);


	Bleedblack_Destroy(context->hBleedblack);
	return EXIT_SUCCESS;
}