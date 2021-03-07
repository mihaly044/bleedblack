#include "pch.h"
#include "cmdline.hpp"

#include <memory>

#include "logging.hpp"

std::shared_ptr<BLIPCSRV_CTX> CommandLineToContext()
{
	auto *const commandLine = GetCommandLineW();
	if (!commandLine)
	{
		logger->critical("Could not get command line, GetLastError() = {}", GetLastError());
		return nullptr;
	}

	int argc;
	auto* const argv = CommandLineToArgvW(commandLine, &argc);
	if(argc != InputParamKindMax)
	{
		logger->critical("Count of parameters seems invalid");
		return nullptr;
	}

	auto context = std::make_shared<BLIPCSRV_CTX>();
	for(auto i = 1; i < InputParamKindMax; ++i)
	{
		auto* const handle = reinterpret_cast<HANDLE>(_wcstoui64(argv[i], nullptr, 10));
		switch(i)
		{
		case ShmHandle: context->hShmMapping = handle; break;
		case ReqHandle: context->hReq = handle; break;
		case AckHandle: context->hAck = handle; break;
		default: logger->warn("Invalid handle. Type = {} Value = 0x{0:x}", i, handle);
		}
	}

	logger->info("Parsed context from the command line");
	return context;
}

void ClearCommandLineArgs()
{
	auto *p = NtCurrentPeb()->ProcessParameters;
	p->CommandLine.Buffer = nullptr;
	p->CommandLine.Length = 0;
	p->CommandLine.MaximumLength = 0;
	RtlFreeUnicodeString(&p->CommandLine);
	logger->info("Cleared command line args");
}
