#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <Windows.h>
#include <winternl.h>
#pragma comment(lib, "ntdll")
#include <ntddmou.h>
#include <bleedblack/io.h>

//
// Undefined NT api for delaying execution
//
NTSYSAPI
NTSTATUS
NTAPI
NtDelayExecution(
	_In_ BOOLEAN Alertable,
	_In_opt_ PLARGE_INTEGER DelayInterval);

//
// Establish connection to the BlackBleed driver
//
void Connect(PHANDLE pHandleOut)
{
	UNICODE_STRING usDeviceLink;
	OBJECT_ATTRIBUTES objectAttributes;
	IO_STATUS_BLOCK ioStatusBlock;

	LPCWSTR szDeviceLink = L"\\DosDevices\\" U_BLEEDBLACK_DRIVER;
	RtlInitUnicodeString(&usDeviceLink, szDeviceLink);
	InitializeObjectAttributes(&objectAttributes, &usDeviceLink, OBJ_CASE_INSENSITIVE, NULL, NULL)

		NTSTATUS status = NtCreateFile(pHandleOut,
			GENERIC_READ | GENERIC_WRITE,
			&objectAttributes,
			&ioStatusBlock,
			NULL,
			0,
			0,
			FILE_OPEN,
			0,
			NULL,
			0);

	if (!NT_SUCCESS(status))
	{
		printf("Error: Failed to connect to the driver, status 0x%08lx\r\n", status);
	}
}

//
// Performs mouse movement
//
void Move(HANDLE device, DWORD pid, int x, int y)
{
	IO_STATUS_BLOCK ioStatusBlock;
	ZeroMemory(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));

	BLEEDBLACK_INPUT_REQUEST request = {
			pid, TRUE, 0, x, y
	};
	
	NTSTATUS status = NtDeviceIoControlFile(device,
	                                        NULL,
	                                        NULL,
	                                        NULL,
	                                        &ioStatusBlock,
	                                        IOCTL_BLEEDBLACK_INPUT,
	                                        &request,
	                                        sizeof(request),
	                                        NULL,
	                                        0);

	if(!NT_SUCCESS(status))
	{
		printf("Move: Error when communicating with the driver: 0x%08lx\r\n", status);
	}
}

//
// Perform left and right clicks
//
void Click(HANDLE device, DWORD pid, DWORD button, ULONG releaseDelayInMilliseconds)
{
	IO_STATUS_BLOCK ioStatusBlock;
	LARGE_INTEGER delayInterval;
	USHORT releaseButton;
	BLEEDBLACK_INPUT_REQUEST request;

	if(button == MOUSE_LEFT_BUTTON_DOWN)
	{
		releaseButton = MOUSE_LEFT_BUTTON_UP;
	} else if(button == MOUSE_RIGHT_BUTTON_DOWN)
	{
		releaseButton = MOUSE_RIGHT_BUTTON_UP;
	}
	else
	{
		printf("Unknown releaseButton for mouse button %lu\r\n", button);
		return;
	}
	
	ZeroMemory(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));
	ZeroMemory(&request, sizeof(BLEEDBLACK_INPUT_REQUEST));
	ZeroMemory(&delayInterval, sizeof(LARGE_INTEGER));

	NTSTATUS status = NtDeviceIoControlFile(device,
		NULL,
		NULL,
		NULL,
		&ioStatusBlock,
		IOCTL_BLEEDBLACK_INPUT,
		&request,
		sizeof(request),
		NULL,
		0);

	if (!NT_SUCCESS(status))
	{
		printf("click: Error when communicating with the driver: 0x%08lx\r\n", status);
	}

	delayInterval.QuadPart = -(releaseDelayInMilliseconds * 10000000LL);
	NtDelayExecution(FALSE, &delayInterval);

	request.ButtonFlags = releaseButton;
	status = NtDeviceIoControlFile(device,
		NULL,
		NULL,
		NULL,
		&ioStatusBlock,
		IOCTL_BLEEDBLACK_INPUT,
		&request,
		sizeof(request),
		NULL,
		0);

	if (!NT_SUCCESS(status))
	{
		printf("click: Error when communicating with the driver: 0x%08lx\r\n", status);
	}
}

int main()
{
	HANDLE device;
	LARGE_INTEGER delayInterval;

	Connect(&device);
	
	if(!device)
	{
		return 0;
	}
	
	ZeroMemory(&delayInterval, sizeof(LARGE_INTEGER));
	delayInterval.QuadPart = -(5 * 10000000LL);
	
	DWORD myPid = GetCurrentProcessId();
	const int radius = 15;

	printf("Watch the cursor do a full circle\r\n");
	for(double i = 0; i < 2 * M_PI; i += 0.1)
	{
		Move(device, myPid, (int)(radius * cos(i)), (int)(radius * sin(i)));
		NtDelayExecution(FALSE, &delayInterval);
	}

	delayInterval.QuadPart = -(3000 * 10000000LL);

	printf("Left click in 3 seconds...\r\n");
	NtDelayExecution(FALSE, &delayInterval);
	Click(device, myPid, MOUSE_LEFT_BUTTON_DOWN, 30);

	printf("Right click in 3 seconds...\r\n");
	NtDelayExecution(FALSE, &delayInterval);
	Click(device, myPid, MOUSE_RIGHT_BUTTON_DOWN, 30);
	
	NtClose(device);
	return 0;
}
