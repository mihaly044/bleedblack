#pragma once

#ifdef _KERNEL_MODE
#include <ntdef.h>
#else
#include <minwindef.h>
#include <devioctl.h>
#endif

#define IOCTL_MOVE_MOUSE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1091, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

typedef struct _BLEEDLBACK_MOUSE_MOVEMENT_INPUT
{
    ULONG_PTR ProcessId;
    USHORT IndicatorFlags;
    LONG MovementX;
    LONG MovementY;
} BLEEDLBACK_MOUSE_MOVEMENT_INPUT, * PBLEEDLBACK_MOUSE_MOVEMENT_INPUT;

typedef struct _BLEEDBLACK_MOUSE_BUTTON_INPUT
{
    ULONG_PTR ProcessId;
    USHORT ButtonFlags;
    USHORT ButtonData;
} BLEEDBLACK_MOUSE_BUTTON_INPUT, PBLEEDBLACK_MOUSE_BUTTON_INPUT;