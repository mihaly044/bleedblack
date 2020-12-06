#include "bleedblack_internal.h"
#include "ob.h"
#include <kbdmou.h>

NTSTATUS BleedBlack_p_SetupMouClass()
{
	NTSTATUS status;
	UNICODE_STRING DeviceName;
	UNICODE_STRING ClassName;
	PDRIVER_OBJECT HidDriverObject = NULL;
	PDRIVER_OBJECT ClassDriverObject = NULL;
	PVOID ClassDriverStart = NULL;
	PVOID ClassDriverEnd = NULL;
	RtlInitUnicodeString(&DeviceName, U_MOUSE_HID);
	status = ObReferenceObjectByName(&DeviceName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		FILE_ANY_ACCESS,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&HidDriverObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[%s] failed to reference device %wZ 0x%08X\n", MODULE_NAME, DeviceName, status));
		return status;
	}

	RtlInitUnicodeString(&ClassName, U_MOUSE_CLASS);
	status = ObReferenceObjectByName(&ClassName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		FILE_ANY_ACCESS,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&ClassDriverObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[%s] failed to reference device %wZ 0x%08X\n", MODULE_NAME, ClassName, status));
		return status;
	}

	ClassDriverStart = (PVOID)ClassDriverObject->DriverStart;
	ClassDriverEnd = (PVOID)((SIZE_T)ClassDriverStart + ClassDriverObject->DriverSize);
	status = STATUS_INTERNAL_ERROR;

	for (PDEVICE_OBJECT HidDeviceObject = HidDriverObject->DeviceObject;
		HidDeviceObject; HidDeviceObject = HidDeviceObject->NextDevice)
	{
		LONGLONG SizeOfDevExtension = (LONGLONG)HidDeviceObject->DeviceObjectExtension
			- (LONGLONG)HidDeviceObject->DeviceExtension;

		if (SizeOfDevExtension <= 0)
			continue;

		SIZE_T DeviceExtPtrCount = SizeOfDevExtension / sizeof(PVOID) - 1;
		PVOID* DeviceExtension = (PVOID*)HidDeviceObject->DeviceExtension;
		for (PDEVICE_OBJECT ClassDeviceObject = ClassDriverObject->DeviceObject;
			ClassDeviceObject; ClassDeviceObject = ClassDeviceObject->NextDevice)
		{
			for (ULONG i = 0u; i < DeviceExtPtrCount; ++i)
			{
				if (DeviceExtension[i] == ClassDeviceObject &&
					DeviceExtension[i + 1] > ClassDriverStart &&
					DeviceExtension[i + 1] < ClassDriverEnd)
				{
					g_blContext->ClassDevice = ClassDeviceObject;
					g_blContext->ClassService = DeviceExtension[i + 1];
					status = STATUS_SUCCESS;
					break;
				}

				if (NT_SUCCESS(status))
					break;
			}

			if (NT_SUCCESS(status))
				break;
		}
	}

	if (ClassDriverObject)
		ObDereferenceObject(ClassDriverObject);

	if (HidDriverObject)
		ObDereferenceObject(HidDriverObject);

	return status;
}

VOID BleedBlack_p_DpcDeferredRoutine(
	PKDPC Dpc,
	PVOID Context,
	PVOID Arg1,
	PVOID Arg2
)
{
	UNREFERENCED_PARAMETER(Dpc);
	UNREFERENCED_PARAMETER(Arg1);
	UNREFERENCED_PARAMETER(Arg2);

	PDPC_CONTEXT DpcContext = (DPC_CONTEXT*)Context;
	PSERVICE_CALLBACK_ROUTINE ServiceCallback = (PSERVICE_CALLBACK_ROUTINE)(DpcContext->Callback);

	ServiceCallback(
		DpcContext->Device,
		DpcContext->InputData,
		DpcContext->InputDataEnd,
		&DpcContext->Consumed);

	KeSetEvent(&DpcContext->Event, IO_MOUSE_INCREMENT, FALSE);
}