#pragma once

//
// Include the right kind of header files
// depending on if we're in kernel or user mode
//
#ifdef _KERNEL_MODE
#include <ntdef.h>
#else
#include <devioctl.h>
#endif

//
// Allow nameless struct/unions
//
#pragma warning(disable: 4201)

//
// User-mode symlink path to the driver
//
#define U_BLEEDBLACK_DRIVER L"\\\\.\\{E80A5F57-345E-4E03-9B1A-5DEB83174A8D}"

//
// IOCTL codes
//
#define IOCTL_BLEEDBLACK_INPUT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x900, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

typedef struct _BLEEDBLACK_INPUT_REQUEST
{
    ULONG_PTR ProcessId;
    BOOLEAN Move;

	union
	{
        struct
        {
            USHORT IndicatorFlags;
            LONG MovementX;
            LONG MovementY;
        };

		struct
		{
            USHORT ButtonFlags;
            USHORT ButtonData;
		};
	};
	
} BLEEDBLACK_INPUT_REQUEST, * PBLEEDBLACK_INPUT_REQUEST;
