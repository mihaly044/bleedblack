#pragma once
#include "common.h"

FORCEINLINE KTRAP_FRAME* PsGetTrapFrame(PETHREAD Thread)
{
    return *(KTRAP_FRAME**)((PUCHAR)Thread + 0x90);
}

PVOID NTAPI PsGetProcessWow64Process(
    _In_ PEPROCESS Process
);

NTSTATUS NTAPI PsAcquireProcessExitSynchronization(
    _In_ PEPROCESS Process
);

VOID NTAPI PsReleaseProcessExitSynchronization(
    _In_ PEPROCESS Process
);