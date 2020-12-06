#include "bleedblack.h"
#include "bleedblack_internal.h"
#pragma warning(disable: 4996)

PBLEEDBLACK_CONTEXT g_blContext = NULL;

NTSTATUS BleedBlack_Init()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if(g_blContext && g_blContext->Initialized)
	{
		KdPrint(("[%s] Already initialized\n", MODULE_NAME));
		status = STATUS_ALREADY_INITIALIZED;
		return status;
	}

	g_blContext = ExAllocatePool(NonPagedPool, sizeof(BLEEDBLACK_CONTEXT));
	if(!g_blContext)
	{
		KdPrint(("[%s] Failed to allocate space for g_blContext, 0x%08X\n", MODULE_NAME, status));
		status = STATUS_INSUFFICIENT_RESOURCES;
		return status;
	}

	status = ExInitializeResourceLite(&g_blContext->Resource);
	if(!NT_SUCCESS(status))
	{
		KdPrint(("[%s] Failed to initialize resource, 0x%08X\n", MODULE_NAME, status));
		return status;
	}

	//ExEnterCriticalRegionAndAcquireResourceExclusive(&g_blContext->Resource);
	//ExReleaseResourceAndLeaveCriticalRegion(&g_blContext->Resource);

	status = BleedBlack_p_SetupMouClass();
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[%s] BleedBlack_p_SetupMouClass has failed with code 0x%08X\n", MODULE_NAME, status));
		return status;
	}

	return status;
}


NTSTATUS BleedBlack_CallService(
	PVOID Input,
	PVOID InputEnd,
	UINT32* Consumed)
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

		Context->Callback = g_blContext->ClassService;
		Context->Device = g_blContext->ClassDevice;
		Context->InputData = Input;
		Context->InputDataEnd = InputEnd;
		Context->Consumed = 0;

		KeInitializeEvent(&Context->Event, NotificationEvent, FALSE);
		KeInitializeDpc(&Context->Dpc, (PKDEFERRED_ROUTINE)BleedBlack_p_DpcDeferredRoutine, Context);

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

VOID BleedBlack_Shutdown()
{
	if (g_blContext)
	{
		if (g_blContext->ResourceAcquired)
			ExReleaseResourceAndLeaveCriticalRegion(&g_blContext->Resource);

		ExDeleteResourceLite(&g_blContext->Resource);
		ExFreePool(g_blContext);
	}
}