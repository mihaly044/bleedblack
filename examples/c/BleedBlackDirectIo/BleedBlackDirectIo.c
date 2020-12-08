#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>
#include <Windows.h>
#include <winternl.h>
#include <bleedblack/io.h>
#pragma comment(lib, "ntdll")

int main()
{
	UNICODE_STRING usDeviceLink;
	OBJECT_ATTRIBUTES objectAttributes;
	IO_STATUS_BLOCK ioStatusBlock;
	HANDLE hDevice;
	
	LPCWSTR szDeviceLink = L"\\DosDevices\\" U_BLEEDBLACK_DRIVER;
	RtlInitUnicodeString(&usDeviceLink, szDeviceLink);
	InitializeObjectAttributes(&objectAttributes, &usDeviceLink, OBJ_CASE_INSENSITIVE, NULL, NULL)

	NTSTATUS status = NtCreateFile(&hDevice,
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

	if(!NT_SUCCESS(status))
	{
		printf("Error: Failed to connect to the driver, status %x\r\n", status);
		return -1;
	}

	printf("Watch the cursor do a full circle\r\n");
	
	DWORD dwMyPid = GetCurrentProcessId();
	BLEEDBLACK_INPUT_REQUEST request = {
			dwMyPid, TRUE
	};
	
	const int radius = 15;
	for(double i = 0; i < 2 * M_PI; i += 0.1)
	{
		RtlSecureZeroMemory(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));
		
		request.MovementX = (int)(radius * cos(i));
		request.MovementY = (int)(radius * sin(i));
		
		NtDeviceIoControlFile(hDevice,
			NULL,
			NULL,
			NULL,
			&ioStatusBlock,
			IOCTL_BLEEDBLACK_INPUT,
			&request,
			sizeof(request),
			NULL,
			0);

		Sleep(5);
	}
	
	NtClose(hDevice);
	return 0;
}
