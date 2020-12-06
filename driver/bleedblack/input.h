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

typedef struct _BLEEDBLACK_CONTEXT
{
    FAST_MUTEX InputLock;
    PDEVICE_OBJECT ClassDeviceObject;
    PVOID ClassService;
    USHORT UnitId;
    BOOLEAN Initialized;
} BLEEDBLACK_CONTEXT, *PBLEEDBLACK_CONTEXT;

NTSTATUS InitializeDevice(VOID);
NTSTATUS CallService(
    _In_ PVOID Input, 
    _In_ PVOID InputEnd,
    _Out_opt_ UINT32* Consumed
);
VOID ShutdownInput(VOID);
