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
	ClearCommandLineArgs();

	auto context = IpcInitialize();
	context->hBleedblack = Bleedblack_Create();

	IpcRunSynchronous(context.get(), [context]() -> bool
	{
		const auto status = Bleedblack_Process(context->hBleedblack, context->sharedData);
		LOG_IF_NTSTATUS_FAILED(status);
		return true;
	}, INFINITE);


	Bleedblack_Destroy(context->hBleedblack);
	return EXIT_SUCCESS;
}
