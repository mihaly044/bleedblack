#pragma once
#include "common.h"

typedef struct _DPC_CONTEXT
{
    KDPC            Dpc;
    KEVENT          Event;
    PVOID           Callback;
    PDEVICE_OBJECT  Device;
    PVOID           InputData;
    PVOID           InputDataEnd;
    ULONG           Consumed;
} DPC_CONTEXT, *PDPC_CONTEXT;

NTSTATUS InitializeDevice();
NTSTATUS CallService(PVOID Input, PVOID InputEnd, UINT32* Consumed);
