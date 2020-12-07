#pragma once

#ifdef BLEEDBLACK_EXPORTS
#define BLEEDBLACK_API __declspec(dllexport)
#else
#define BLEEDBLACK_API __declspec(dllimport)
#endif
#define BLEEDBLACK_C_API EXTERN_C BLEEDBLACK_API

BLEEDBLACK_C_API PVOID Bleedblack_Create();

BLEEDBLACK_C_API NTSTATUS Bleedblack_Init(
	_In_ PVOID pHandle
);

BLEEDBLACK_C_API BOOLEAN Bleedblack_IsReady(
	_In_ PVOID pHandle
);

BLEEDBLACK_C_API NTSTATUS Bleedblack_Move(
	_In_ PVOID pHandle,
	_In_ ULONG_PTR ulPid,
	_In_ LONG x,
	_In_ LONG y
);

BLEEDBLACK_C_API NTSTATUS Bleedblack_Click(
	_In_ PVOID pHandle,
	_In_ ULONG_PTR ulPid,
	_In_ USHORT ushButtonFlags,
	_In_ ULONG ulReleaseDelayInMilliseconds
);

BLEEDBLACK_C_API VOID Bleedblack_Destroy(
	_In_ PVOID pHandle
);
