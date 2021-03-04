#include "common.h"
#include "dispatch.h"
#include "input.h"

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

HANDLE g_hBoundCallbackRegistration = NULL;

NTSTATUS DriverEntry(
	_In_	PDRIVER_OBJECT DriverObject,
	_In_	PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status;

	DbgPrint("Loading %s\n", MODULE_NAME);
	DriverObject->DriverUnload = DriverUnload;

	g_hBoundCallbackRegistration = KeRegisterBoundCallback(Dispatch);
	if (!g_hBoundCallbackRegistration)
	{
		DbgPrint("[%s] Failed registering callback\n", MODULE_NAME);
		return STATUS_CALLBACK_BYPASS;
	}

	status = MiiInitializeDevice();
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[%s] failed to init device: 0x%08X\n", MODULE_NAME, status);
		return status;
	}

	
	DbgPrint("Loaded %s\n", MODULE_NAME);
	return status;
}

VOID DriverUnload(
	_In_ DRIVER_OBJECT* DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	
	DbgPrint("Unloading %s\n", MODULE_NAME);
	MiiDestroyDevice();

	if (NULL != g_hBoundCallbackRegistration)
		KeDeregisterBoundCallback(g_hBoundCallbackRegistration);

	DbgPrint("Unloaded %s\n", MODULE_NAME);
}
