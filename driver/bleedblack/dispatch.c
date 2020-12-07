#include "dispatch.h"
#include <bleedblack.h>
#include <ntddmou.h>
#include "mi.h"

#pragma warning(disable: 28252 28253)

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
NTSTATUS Dispatch(
	_In_	PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS Status;
	ULONG ReturnLength;
	PVOID Buffer;

	const PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
	const ULONG IoControlCode = IoStack->Parameters.DeviceIoControl.IoControlCode;
	//const ULONG outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;
	const ULONG inputBufferLength = IoStack->Parameters.DeviceIoControl.InputBufferLength;
	ReturnLength = 0;

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
	case IOCTL_MOVE_MOUSE:
		Buffer = Irp->AssociatedIrp.SystemBuffer;
		
		if(Buffer && inputBufferLength == sizeof(BLEEDLBACK_MOUSE_MOVEMENT_INPUT))
		{
			Status = MiiSendInput((PBLEEDLBACK_MOUSE_MOVEMENT_INPUT)Buffer);
			ReturnLength = sizeof(BLEEDLBACK_MOUSE_MOVEMENT_INPUT);
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
		KdPrint(("[%s] Invalid request 0x%X", MODULE_NAME, IoStack->Parameters.DeviceIoControl.IoControlCode));
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
