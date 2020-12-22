#include "common.h"
#include "dispatch.h"
#include "input.h"

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

UNICODE_STRING g_uszDeviceName = RTL_CONSTANT_STRING(L"\\Device\\{E80A5F57-345E-4E03-9B1A-5DEB83174A8D}");
UNICODE_STRING g_uszSymlink = RTL_CONSTANT_STRING(L"\\??\\{E80A5F57-345E-4E03-9B1A-5DEB83174A8D}");

NTSTATUS DriverEntry(
	_In_	PDRIVER_OBJECT DriverObject,
	_In_	PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	PDEVICE_OBJECT DeviceObject = NULL;
	NTSTATUS status;


	DbgPrint("Loading %s\n", MODULE_NAME);
	DriverObject->DriverUnload = DriverUnload;

	status = MiiInitializeDevice();
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[%s] failed to init device: 0x%08X\n", MODULE_NAME, status);
		return STATUS_DEVICE_FEATURE_NOT_SUPPORTED;
	}

	status = IoCreateDevice(DriverObject,
		0,
		&g_uszDeviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&DeviceObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[%s] failed to create device: 0x%08X\n", MODULE_NAME, status);
		return status;
	}

	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	DeviceObject->Flags |= DO_DIRECT_IO;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = GenericDispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = GenericDispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Dispatch;

	status = IoCreateSymbolicLink(&g_uszSymlink, &g_uszDeviceName);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[%s] Failed to create symbolic link 0x%08X\n", MODULE_NAME, status);
		return status;
	}

	DbgPrint("Loaded %s\n", MODULE_NAME);
	return status;
}

VOID DriverUnload(
	_In_ DRIVER_OBJECT* DriverObject)
{
	DbgPrint("Unloading %s\n", MODULE_NAME);
	MiiDestroyDevice();
	IoDeleteSymbolicLink(&g_uszSymlink);
	
	if (DriverObject->DeviceObject)
	{
		IoDeleteDevice(DriverObject->DeviceObject);
	}

	DbgPrint("Unloaded %s\n", MODULE_NAME);
}
