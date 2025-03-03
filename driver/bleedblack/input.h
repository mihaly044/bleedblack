#pragma once
#include "common.h"
#include <bleedblack/io.h>

typedef struct _MOU_DPC_CONTEXT
{
    KDPC            Dpc;
    KEVENT          Event;
    PVOID           Callback;
    PDEVICE_OBJECT  Device;
    PVOID           InputData;
    PVOID           InputDataEnd;
    ULONG           Consumed;
} MOU_DPC_CONTEXT, *PMOU_DPC_CONTEXT;

typedef struct _MII_CONTEXT
{
	// TODO: Make use of InputLock
    FAST_MUTEX InputLock;
    PDEVICE_OBJECT ClassDeviceObject;
    PVOID ClassService;
    USHORT UnitId;
    BOOLEAN DeviceStackInitialized;
} MII_CONTEXT, *PMII_CONTEXT;


NTSTATUS MiiInitializeDevice(VOID);

NTSTATUS MiiSendInput(
    _In_ PBLEEDBLACK_INPUT_REQUEST pInput
);

NTSTATUS MiipCallService(
    _In_ PVOID Input, 
    _In_ PVOID InputEnd,
    _Out_opt_ UINT32* Consumed
);

VOID MiiDestroyDevice(VOID);
