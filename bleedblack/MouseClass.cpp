#include "pch.h"
#include "MouseClass.h"

#include <ntddmou.h>
#include <strsafe.h>
#include <bleedblack/io.h>

#include "Time.h"

extern "C" void _trigger_br(PBLEEDBLACK_INPUT_REQUEST input);

CMouseClass::CMouseClass() = default;
CMouseClass::~CMouseClass() = default;

CMouseClass* CMouseClass::Create()
{
	return new CMouseClass;
}

NTSTATUS CMouseClass::Init()
{
	// Return value might be inaccurate
	return STATUS_SUCCESS;
}

NTSTATUS CMouseClass::Move(ULONG_PTR pid, LONG x, LONG y)
{
	BLEEDBLACK_INPUT_REQUEST request;
	RtlZeroMemory(&request, sizeof request);

	request.Move = TRUE;
	request.MovementX = x;
	request.MovementY = y;

	_trigger_br(&request);

	// Return value might be inaccurate
	return STATUS_SUCCESS;
}

NTSTATUS CMouseClass::Click(ULONG_PTR pid, USHORT button, ULONG ReleaseDelayInMilliseconds)
{
	LARGE_INTEGER delayInterval = {};
	USHORT releaseButton;
	NTSTATUS status;
	BLEEDBLACK_INPUT_REQUEST request;
	RtlZeroMemory(&request, sizeof request);

	switch (button)
	{
	case MOUSE_LEFT_BUTTON_DOWN:
		releaseButton = MOUSE_LEFT_BUTTON_UP;
		break;

	case MOUSE_RIGHT_BUTTON_DOWN:
		releaseButton = MOUSE_RIGHT_BUTTON_UP;
		break;

	case MOUSE_MIDDLE_BUTTON_DOWN:
		releaseButton = MOUSE_MIDDLE_BUTTON_UP;
		break;

	case MOUSE_BUTTON_4_DOWN:
		releaseButton = MOUSE_BUTTON_4_UP;
		break;

	case MOUSE_BUTTON_5_DOWN:
		releaseButton = MOUSE_BUTTON_5_UP;
		break;

	default:
		return STATUS_INVALID_PARAMETER_2;
	}

	MakeRelativeIntervalMilliseconds(
		&delayInterval,
		ReleaseDelayInMilliseconds);

	request.Move = FALSE;
	request.ButtonFlags = button;
	request.ProcessId = pid;

	_trigger_br(&request);

	status = NtDelayExecution(FALSE, &delayInterval);
	if (!NT_SUCCESS(status))
		return status;

	request.Move = FALSE;
	request.ButtonFlags = releaseButton;
	
	_trigger_br(&request);
	return status;
}

BOOLEAN CMouseClass::IsReady()
{
	// Return value might be inaccurate
	return TRUE;
}
