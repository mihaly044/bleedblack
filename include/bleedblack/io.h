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
		};
	};
	
} BLEEDBLACK_INPUT_REQUEST, * PBLEEDBLACK_INPUT_REQUEST;
