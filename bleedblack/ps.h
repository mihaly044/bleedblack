#pragma once
#include "common.h"

PVOID NTAPI PsGetProcessWow64Process(
    _In_ PEPROCESS Process
);