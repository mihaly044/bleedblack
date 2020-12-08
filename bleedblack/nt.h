#pragma once
#include "pch.h"
#pragma comment(lib, "ntdll")

EXTERN_C
NTSYSAPI
NTSTATUS
NTAPI
NtDelayExecution(
    _In_ BOOLEAN Alertable,
    _In_opt_ PLARGE_INTEGER DelayInterval);