#pragma once
#include "common.h"

PVOID NTAPI PsGetProcessWow64Process(
    _In_ PEPROCESS Process
);

NTSTATUS NTAPI PsAcquireProcessExitSynchronization(
    _In_ PEPROCESS Process
);

VOID NTAPI PsReleaseProcessExitSynchronization(
    _In_ PEPROCESS Process
);