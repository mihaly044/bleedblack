#include "pch.h"
#include <bleedblack/bleedblack.h>
#include "MouseClass.h"

#define INSTANCE(x) static_cast<CMouseClass*>(x)

BLEEDBLACK_C_API PVOID Bleedblack_Create()
{
	return CMouseClass::Create();
}

BLEEDBLACK_C_API NTSTATUS Bleedblack_Init(
	_In_ PVOID pHandle
)
{
	if (pHandle == nullptr)
		return STATUS_INVALID_HANDLE;

	return INSTANCE(pHandle)->Init();
}

BLEEDBLACK_C_API BOOLEAN Bleedblack_IsReady(
	_In_ PVOID pHandle
)
{
	if (pHandle == nullptr)
		return FALSE;

	return INSTANCE(pHandle)->IsReady();
}

BLEEDBLACK_C_API NTSTATUS Bleedblack_Move(
	_In_ PVOID pHandle,
	_In_ ULONG_PTR ulPid,
	_In_ LONG x,
	_In_ LONG y
)
{
	if (pHandle == nullptr)
		return STATUS_INVALID_HANDLE;

	return INSTANCE(pHandle)->Move(ulPid, x, y);
}

BLEEDBLACK_C_API NTSTATUS Bleedblack_Click(
	_In_ PVOID pHandle,
	_In_ ULONG_PTR ulPid,
	_In_ USHORT ushButtonFlags,
	_In_ ULONG ulReleaseDelayInMilliseconds
)
{
	if (pHandle == nullptr)
		return STATUS_INVALID_HANDLE;

	return INSTANCE(pHandle)->Click(ulPid, ushButtonFlags, ulReleaseDelayInMilliseconds);
}

BLEEDBLACK_C_API VOID Bleedblack_Destroy(
	_In_ PVOID pHandle
)
{
	if (pHandle == nullptr)
		return;

	delete INSTANCE(pHandle);
}
