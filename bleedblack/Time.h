#pragma once
#include "pch.h"

//
// NT time is measured in units of 100-nanosecond intervals.
//
#define SYSTEM_TIME_UNIT_MICROSECOND    ((LONGLONG)(10))
#define SYSTEM_TIME_UNIT_MILLISECOND    ((LONGLONG)(10000))
#define SYSTEM_TIME_UNIT_SECOND         ((LONGLONG)(10000000))

#define RELATIVE_INTERVAL(Interval)     (-Interval)

FORCEINLINE
VOID
MakeRelativeIntervalSeconds(
    _Inout_ PLARGE_INTEGER pInterval,
    _In_ LONGLONG Seconds
)
{
    pInterval->QuadPart = RELATIVE_INTERVAL(Seconds * SYSTEM_TIME_UNIT_SECOND);
}

FORCEINLINE
VOID
MakeRelativeIntervalMilliseconds(
    _Inout_ PLARGE_INTEGER pInterval,
    _In_ LONGLONG Milliseconds
)
{
    pInterval->QuadPart =
        RELATIVE_INTERVAL(Milliseconds * SYSTEM_TIME_UNIT_MILLISECOND);
}
