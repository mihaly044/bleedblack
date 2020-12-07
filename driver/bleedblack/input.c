#include "input.h"
#include <kbdmou.h>

//#define U_KBD_HID L"\\Driver\\kbdhid"
//#define U_KDB_CLASS L"\\Driver\\kbdclass"
#define U_MOUSE_HID L"\\Driver\\mouhid"
#define U_MOUSE_CLASS L"\\Driver\\mouclass"

PMII_CONTEXT g_Context;

NTSTATUS MiipInitializeContext(
	_Inout_ PMII_CONTEXT* ppMiiContext
)
{
	NTSTATUS Status;
	PMII_CONTEXT Context;

	if(!*ppMiiContext)
	{
		DbgPrint("[%s] MiipInitializeContext: Invalid context parameter.", MODULE_NAME);
		Status = STATUS_INVALID_PARAMETER;
		return Status;
	}
	
	if(*ppMiiContext && (*ppMiiContext)->DeviceStackInitialized)
	{
		DbgPrint("[%s] MiipInitializeContext: Device stack context already initialized.", MODULE_NAME);
		Status = STATUS_ALREADY_INITIALIZED;
		return Status;
	}

	Context = ExAllocatePool(NonPagedPool, sizeof(MII_CONTEXT));
	if(!Context)
	{
		DbgPrint("[%s] MiipInitializeContext: Memory allocation for device stack context failed.", MODULE_NAME);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlZeroMemory(Context, sizeof(MII_CONTEXT));
	*ppMiiContext = Context;
	return STATUS_SUCCESS;
}

NTSTATUS MiiInitializeDevice(VOID)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	UNICODE_STRING DeviceName;
	UNICODE_STRING ClassName;
	PDRIVER_OBJECT HidDriverObject = NULL;
	PDRIVER_OBJECT ClassDriverObject = NULL;
	PVOID ClassDriverStart;
	PVOID ClassDriverEnd;

	Status = MiipInitializeContext(&g_Context);
	if(!NT_SUCCESS(Status))
	{
		DbgPrint("[%s] MiiInitializeDevice: MiipInitializeContext has failed with code 0x%08X", MODULE_NAME, Status);
	}

	RtlInitUnicodeString(&DeviceName, U_MOUSE_HID);
	Status = ObReferenceObjectByName(&DeviceName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		FILE_ANY_ACCESS,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&HidDriverObject);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("[%s] Failed to reference device %wZ 0x%08X\n", MODULE_NAME, DeviceName, Status));
		return Status;
	}

	RtlInitUnicodeString(&ClassName, U_MOUSE_CLASS);
	Status = ObReferenceObjectByName(&ClassName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		FILE_ANY_ACCESS,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&ClassDriverObject);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("[%s] Failed to reference device %wZ 0x%08X\n", MODULE_NAME, ClassName, Status));
		return Status;
	}

	ClassDriverStart = (PVOID)ClassDriverObject->DriverStart;
	ClassDriverEnd = (PVOID)((SIZE_T)ClassDriverStart + ClassDriverObject->DriverSize);
	Status = STATUS_INTERNAL_ERROR;

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
					g_Context->ClassDeviceObject = ClassDeviceObject;
					g_Context->ClassService = DeviceExtension[i + 1];
					
					Status = STATUS_SUCCESS;
					break;
				}

				if (NT_SUCCESS(Status))
					break;
			}

			if (NT_SUCCESS(Status))
				break;
		}
	}

	if (ClassDriverObject)
		ObDereferenceObject(ClassDriverObject);

	if (HidDriverObject)
		ObDereferenceObject(HidDriverObject);

	return Status;
}

NTSTATUS MiiSendInput(
	_In_ PBLEEDBLACK_INPUT_REQUEST pInput
)
{
	KAPC_STATE ApcState;
	PEPROCESS Process = NULL;
	BOOLEAN SynchronizationLockAcquired = FALSE;
	NTSTATUS Status;

	if(pInput->ProcessId)
	{
		if (pInput->ProcessId < 4)
		{
			KdPrint(("[%s] Invalid PID %lu\n", MODULE_NAME, pInput->ProcessId));
			Status = STATUS_INVALID_PARAMETER_1;
			return Status;
		}
		
		Status = PsLookupProcessByProcessId((HANDLE)pInput->ProcessId, &Process);
		if(!NT_SUCCESS(Status))
		{
			KdPrint(("[%s] PsLookupProcessByProcessId has failed with code 0x%08X\n",
				MODULE_NAME, Status));
			return Status;
		}

		Status = PsAcquireProcessExitSynchronization(Process);
		if (!NT_SUCCESS(Status))
		{
			KdPrint(("[%s] PsAcquireProcessExitSynchronization has failed with code 0x%08X\n",
				MODULE_NAME, Status));
			return Status;
		}

		SynchronizationLockAcquired = TRUE;
	}

	// Allocate mmry from the nonpaged pool to prevent our data from being paged out
	PMOUSE_INPUT_DATA InputData = (PMOUSE_INPUT_DATA)ExAllocatePool(NonPagedPool, sizeof(MOUSE_INPUT_DATA));
	if(!InputData)
	{
		KdPrint(("[%s] Failed to allocate memory for InputData\n", MODULE_NAME));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlZeroMemory(InputData, sizeof(MOUSE_INPUT_DATA));
	if(pInput->Move)
	{
		InputData->Flags = pInput->IndicatorFlags;
		InputData->LastX = pInput->MovementX;
		InputData->LastY = pInput->MovementY;
	}
	else
	{
		InputData->ButtonData = pInput->ButtonData;
		InputData->ButtonFlags = pInput->ButtonFlags;
	}

	// Assume we have only one pointing device
	InputData->UnitId = 1;
	
	if(SynchronizationLockAcquired)
	{
		__try
		{
			__try
			{
				KeStackAttachProcess(Process, &ApcState);
				Status = MiipCallService((PVOID)InputData, (PVOID)(InputData + 1), NULL);
			}
			__finally
			{
				KeUnstackDetachProcess(&ApcState);
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			KdPrint(("[%s] Unexpected exception: 0x%X\n", MODULE_NAME, GetExceptionCode()));
			Status = STATUS_DRIVER_INTERNAL_ERROR;
		}
	}
	else
	{
		__try
		{
			Status = MiipCallService((PVOID)InputData, (PVOID)(InputData + 1), NULL);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			KdPrint(("[%s] Unexpected exception: 0x%X\n", MODULE_NAME, GetExceptionCode()));
			Status = STATUS_DRIVER_INTERNAL_ERROR;
		}	
	}

	if (InputData)
		ExFreePool(InputData);
	
	if(SynchronizationLockAcquired)
		PsReleaseProcessExitSynchronization(Process);

	if (Process)
		ObDereferenceObject(Process);
	
	return Status;
}

VOID MiipMouseDpcRoutine(PKDPC Dpc, PVOID Context, PVOID Arg1, PVOID Arg2)
{
	UNREFERENCED_PARAMETER(Dpc);
	UNREFERENCED_PARAMETER(Arg1);
	UNREFERENCED_PARAMETER(Arg2);

	PMOU_DPC_CONTEXT DpcContext = (MOU_DPC_CONTEXT*)Context;
	PSERVICE_CALLBACK_ROUTINE ServiceCallback = (PSERVICE_CALLBACK_ROUTINE)(DpcContext->Callback);

	ServiceCallback(
		DpcContext->Device,
		DpcContext->InputData,
		DpcContext->InputDataEnd,
		&DpcContext->Consumed);

	KeSetEvent(&DpcContext->Event, IO_MOUSE_INCREMENT, FALSE);
}

NTSTATUS MiipCallService(
	_In_ PVOID Input,
	_In_ PVOID InputEnd,
	_Out_opt_ UINT32* Consumed
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PMOU_DPC_CONTEXT Context = NULL;

	for(;;)
	{
		Context = ExAllocatePool(NonPagedPool, sizeof(MOU_DPC_CONTEXT));
		if (!Context)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		Context->Callback = g_Context->ClassService;
		Context->Device = g_Context->ClassDeviceObject;
		Context->InputData = Input;
		Context->InputDataEnd = InputEnd;
		Context->Consumed = 0;

		KeInitializeEvent(&Context->Event, NotificationEvent, FALSE);
		KeInitializeDpc(&Context->Dpc, (PKDEFERRED_ROUTINE)MiipMouseDpcRoutine, Context);

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

VOID MiiDestroyDevice(VOID)
{
	if (g_Context)
	{
		ExFreePool(g_Context);
	}
}