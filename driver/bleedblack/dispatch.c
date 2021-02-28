#include "dispatch.h"
#include <bleedblack/io.h>
#include "input.h"

#pragma warning(disable: 28252 28253)

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
NTSTATUS Dispatch(
	_In_	PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS Status;
	PVOID Buffer;

	PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
	ULONG IoControlCode = IoStack->Parameters.DeviceIoControl.IoControlCode;
	//const ULONG outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG inputBufferLength = IoStack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG ReturnLength = 0;

	//
	// Don't process IOCTLs coming from x86 processes
	//
	PEPROCESS Process = IoGetRequestorProcess(Irp);
	if (PsGetProcessWow64Process(Process) != NULL)
	{
		Status = STATUS_INVALID_IMAGE_FORMAT;
		goto ControlEnd;
	}

	switch (IoControlCode)
	{
	case IOCTL_BLEEDBLACK_INPUT:
		Buffer = Irp->AssociatedIrp.SystemBuffer;
		
		if(Buffer && inputBufferLength == sizeof(BLEEDBLACK_INPUT_REQUEST))
		{
			Status = MiiSendInput((PBLEEDBLACK_INPUT_REQUEST)Buffer);
			ReturnLength = sizeof(PBLEEDBLACK_INPUT_REQUEST);
		}
		else
		{
			Status = STATUS_INVALID_DEVICE_REQUEST;
		}

		break;
	default:
		//
		// The specified I/O control code is unrecognized by this driver.
		//
		DbgPrint("[%s] Invalid request 0x%X", MODULE_NAME, IoStack->Parameters.DeviceIoControl.IoControlCode);
		Status = STATUS_INVALID_DEVICE_REQUEST;
	}

	ControlEnd:
	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = ReturnLength;
	IofCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CLOSE)
NTSTATUS GenericDispatch(
	_In_	PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
