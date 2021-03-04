#include "dispatch.h"
#include <bleedblack/io.h>
#include "input.h"

BOUND_CALLBACK_STATUS Dispatch(
	VOID
)
{
	PETHREAD Thread = PsGetCurrentThread();
	PKTRAP_FRAME TrapFrame = PsGetTrapFrame(Thread);

	ULONG Upper = *(ULONG*)(TrapFrame->Rsp + 4);
	ULONG Lower = *(ULONG*)(TrapFrame->Rsp + 8);
	ULONG_PTR Addr = (ULONG_PTR)Upper << 32 | Lower;

	MiiSendInput((PBLEEDBLACK_INPUT_REQUEST)Addr);

	TrapFrame->Rip += 4;
	return BoundExceptionHandled;
}
