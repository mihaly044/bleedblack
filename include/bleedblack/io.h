#pragma once

//
// Allow nameless struct/unions
//
#pragma warning(disable: 4201)

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
            ULONG  ReleaseDelayInMillis;
		};
	};
	
} BLEEDBLACK_INPUT_REQUEST, * PBLEEDBLACK_INPUT_REQUEST;

typedef struct _BLEEDBLACK_IPC
{
    BOOLEAN ParentExiting;
    BLEEDBLACK_INPUT_REQUEST Req;
} BLEEDBLACK_IPC, *PBLEEDBLACK_IPC;

#define BLEEDBLACK_IPC_ENDPOINT "blipcsrv"
#define BLEEDBLACK_IPC_EVT_NAME "Global\\blipc"
