#include "input.h"
#include "ob.h"
#include <kbdmou.h>

//#define U_KBD_HID L"\\Driver\\kbdhid"
//#define U_KDB_CLASS L"\\Driver\\kbdclass"
#define U_MOUSE_HID L"\\Driver\\mouhid"
#define U_MOUSE_CLASS L"\\Driver\\mouclass"

PDEVICE_OBJECT g_ClassDeviceObject = NULL;
PVOID g_ClassService = NULL;

NTSTATUS InitializeDevice(VOID)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
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
					g_ClassDeviceObject = ClassDeviceObject;
					g_ClassService = DeviceExtension[i + 1];
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

VOID DpcDeferredRoutine(PKDPC Dpc, PVOID Context, PVOID Arg1, PVOID Arg2)
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

NTSTATUS CallService(
	_In_ PVOID Input,
	_In_ PVOID InputEnd,
	_Out_opt_ UINT32* Consumed
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDPC_CONTEXT Context = NULL;

	for(;;)
	{
		Context = ExAllocatePool(NonPagedPool, sizeof(DPC_CONTEXT));
		if (!Context)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		Context->Callback = g_ClassService;
		Context->Device = g_ClassDeviceObject;
		Context->InputData = Input;
		Context->InputDataEnd = InputEnd;
		Context->Consumed = 0;

		KeInitializeEvent(&Context->Event, NotificationEvent, FALSE);
		KeInitializeDpc(&Context->Dpc, (PKDEFERRED_ROUTINE)DpcDeferredRoutine, Context);

		if (!KeInsertQueueDpc(&Context->Dpc, NULL, NULL))
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		status = KeWaitForSingleObject(&Context->Event, Executive, KernelMode, FALSE, NULL);
		if (!NT_SUCCESS(status))
		{
			break;
		}

		if (Consumed)
		{
			*Consumed = Context->Consumed;
		}

		break;
	}

	if (Context) {
		ExFreePool(Context);
	}

	return status;
}

