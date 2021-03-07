#pragma once
#include <memory>

#include "context.hpp"

enum INPUT_PARAM_KIND
{
	None = 0,
	ShmHandle,
	ReqHandle,
	AckHandle,
	InputParamKindMax
};

std::shared_ptr<BLIPCSRV_CTX> CommandLineToContext();
void ClearCommandLineArgs();
