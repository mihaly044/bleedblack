#pragma once
#include "common.h"
//#define U_KBD_HID L"\\Driver\\kbdhid"
//#define U_KDB_CLASS L"\\Driver\\kbdclass"
#define U_MOUSE_HID L"\\Driver\\mouhid"
#define U_MOUSE_CLASS L"\\Driver\\mouclass"

typedef struct _DPC_CONTEXT
{
	KDPC            Dpc;
	KEVENT          Event;
	PVOID           Callback;
	PDEVICE_OBJECT  Device;
	PVOID           InputData;
	PVOID           InputDataEnd;
	ULONG           Consumed;
} DPC_CONTEXT, * PDPC_CONTEXT;

typedef struct _BLEEDBLACK_CONTEXT
{
	PDEVICE_OBJECT ClassDevice;
	PVOID ClassService;
	BOOLEAN Initialized;
	ERESOURCE Resource;
	HANDLE PnpCallbackRegistrationHandle;
	BOOLEAN ResourceAcquired;
} BLEEDBLACK_CONTEXT, * PBLEEDBLACK_CONTEXT;

NTSTATUS BleedBlack_p_SetupMouClass();
VOID BleedBlack_p_DpcDeferredRoutine(
	PKDPC Dpc,
	PVOID Context,
	PVOID Arg1,
	PVOID Arg2
);

extern PBLEEDBLACK_CONTEXT g_blContext;
