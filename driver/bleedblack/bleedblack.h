#pragma once
#include "common.h"

NTSTATUS BleedBlack_Init();
NTSTATUS BleedBlack_CallService(PVOID Input, PVOID InputEnd, UINT32* Consumed);
VOID BleedBlack_Shutdown();
