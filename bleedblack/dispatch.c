#include "common.h"
#include "dispatch.h"
#include "ps.h"
#pragma warning(disable: 28252 28253)

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
NTSTATUS Dispatch(
	_In_	PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS status;
	ULONG ReturnLength;
	//PVOID buffer;

	const PIO_STACK_LOCATION ioStack = IoGetCurrentIrpStackLocation(Irp);
	const ULONG ioControlCode = ioStack->Parameters.DeviceIoControl.IoControlCode;
	//const ULONG outputBufferLength = ioStack->Parameters.DeviceIoControl.OutputBufferLength;
	//const ULONG inputBufferLength = ioStack->Parameters.DeviceIoControl.InputBufferLength;
	ReturnLength = 0;

	//
	// Don't process IOCTLs coming from x86 processes
	//
	PEPROCESS Process = IoGetRequestorProcess(Irp);
	if (PsGetProcessWow64Process(Process) != NULL)
	{
		status = STATUS_INVALID_IMAGE_FORMAT;
		goto ControlEnd;
	}

	switch (ioControlCode)
	{
	default:
		//
		// The specified I/O control code is unrecognized by this driver.
		//
		KdPrint(("[%s] Invalid request 0x%X", MODULE_NAME, ioStack->Parameters.DeviceIoControl.IoControlCode));
		status = STATUS_INVALID_DEVICE_REQUEST;
	}

	ControlEnd:
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = ReturnLength;
	IofCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
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
