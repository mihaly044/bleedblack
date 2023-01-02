#include "pch.h"
#include "cmdline.hpp"

#include <memory>

std::shared_ptr<BLIPCSRV_CTX> CommandLineToContext()
{
	auto *const commandLine = GetCommandLineW();

	if(!LOG_LAST_ERROR_IF_NULL_MSG(commandLine, "Could not get command line"))
		return nullptr;

	int argc;
	auto* const argv = CommandLineToArgvW(commandLine, &argc);
	if(argc != InputParamKindMax)
	{
		LOG_WIN32(ERROR_INVALID_PARAMETER);
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
		default: LOG_WIN32_MSG(ERROR_INVALID_PARAMETER, "Type = %d Value = %08x", i, handle);
		}
	}
	
	return context;
}

void ClearCommandLineArgs()
{
	auto *p = NtCurrentPeb()->ProcessParameters;
	p->CommandLine.Buffer = nullptr;
	p->CommandLine.Length = 0;
	p->CommandLine.MaximumLength = 0;
	RtlFreeUnicodeString(&p->CommandLine);
}
