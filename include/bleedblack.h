#pragma once

#ifdef _KERNEL_MODE
#include <ntdef.h>
#else
#include <minwindef.h>
#include <devioctl.h>
#endif

#define IOCTL_MOVE_MOUSE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1091, METHOD_IN_DIRECT, FILE_SPECIAL_ACCESS)

typedef struct _BLEEDBLACK_MOVE_MOUSE
{
    ULONG_PTR ProcessId;
    USHORT IndicatorFlags;
    LONG MovementX;
    LONG MovementY;
} BLEEDBLACK_MOVE_MOUSE, *PBLEEDBLACK_MOVE_MOUSE;

