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

	KdPrint(("Loading %s\n", MODULE_NAME));
	PDEVICE_OBJECT DeviceObject = NULL;
	DriverObject->DriverUnload = DriverUnload;

	NTSTATUS Status = IoCreateDevice(DriverObject,
		0,
		&g_uszDeviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&DeviceObject);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("[%s] failed to create device: 0x%08X\n", MODULE_NAME, Status));
		return Status;
	}

	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	DeviceObject->Flags |= DO_DIRECT_IO;
	
	DriverObject->MajorFunction[IRP_MJ_CREATE] = GenericDispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = GenericDispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Dispatch;

	Status = IoCreateSymbolicLink(&g_uszSymlink, &g_uszDeviceName);
	if (!NT_SUCCESS(Status)) {
		KdPrint(("[%s] Failed to create symbolic link 0x%08X\n", MODULE_NAME, Status));
		IoDeleteDevice(DeviceObject);
		return Status;
	}

	Status = InitializeDevice();
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("[%s] failed to init device: 0x%08X\n", MODULE_NAME, Status));
		return Status;
	}

	KdPrint(("Loaded %s\n", MODULE_NAME));
	return Status;
}

VOID DriverUnload(
	_In_ DRIVER_OBJECT* DriverObject)
{
	KdPrint(("Unloading %s\n", MODULE_NAME));
	ShutdownInput();
	IoDeleteSymbolicLink(&g_uszSymlink);
	IoDeleteDevice(DriverObject->DeviceObject);
	KdPrint(("Unloaded %s\n", MODULE_NAME));
}
