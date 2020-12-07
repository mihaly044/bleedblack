#include "pch.h"
#include "MouseClass.h"

#include <ntddmou.h>
#include <strsafe.h>
#include <bleedblack/io.h>

#include "Time.h"

CMouseClass::CMouseClass()
	: m_hDevice(INVALID_HANDLE_VALUE),
	m_bReady(FALSE)
{

}

NTSTATUS CMouseClass::OpenDriver(LPCWSTR lpName, ACCESS_MASK dwAccessMask, PHANDLE pDeviceHandleOut)
{
	NTSTATUS status;

	UNICODE_STRING usDeviceLink;
	OBJECT_ATTRIBUTES obja;
	IO_STATUS_BLOCK iost;

	TCHAR szDeviceLink[MAX_PATH + 1];

	// assume failure
	if (pDeviceHandleOut)
		*pDeviceHandleOut = INVALID_HANDLE_VALUE;
	else
		return STATUS_INVALID_PARAMETER_2;

	if (lpName)
	{
		RtlSecureZeroMemory(szDeviceLink, sizeof(szDeviceLink));

		if (StringCchPrintf(szDeviceLink,
			MAX_PATH,
			TEXT("\\DosDevices\\%wS"),
			lpName) != ERROR_SUCCESS)
		{
			return STATUS_INVALID_PARAMETER_1;
		}

		RtlInitUnicodeString(&usDeviceLink, szDeviceLink);
		InitializeObjectAttributes(&obja, &usDeviceLink, OBJ_CASE_INSENSITIVE, NULL, NULL)

		status = NtCreateFile(pDeviceHandleOut,
		                      dwAccessMask,
		                      &obja,
		                      &iost,
		                      nullptr,
		                      0,
		                      0,
		                      FILE_OPEN,
		                      0,
		                      nullptr,
		                      0);

	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
	}

	return status;
}

NTSTATUS CMouseClass::CallDriver(HANDLE hDevice, ULONG ulCtlCode, PVOID pInput, ULONG cbInput, PVOID pOutput, ULONG cbOutput, PDWORD returnLength)
{
	IO_STATUS_BLOCK ioStatus;
	const NTSTATUS status = NtDeviceIoControlFile(hDevice, nullptr, nullptr, nullptr, &ioStatus, ulCtlCode,

		pInput, cbInput,
		pOutput, cbOutput);
	if (returnLength)
		*returnLength = static_cast<DWORD>(ioStatus.Information);

	return status;
}

CMouseClass* CMouseClass::Create()
{
	return new CMouseClass;
}

NTSTATUS CMouseClass::Init()
{
	NTSTATUS status = OpenDriver(U_BLEEDBLACK_DRIVER, GENERIC_READ | GENERIC_WRITE, &m_hDevice);
	if(!NT_SUCCESS(status))
	{
		return status;
	}
	
	m_bReady = TRUE;
	return status;
}

NTSTATUS CMouseClass::Move(ULONG_PTR pid, LONG x, LONG y) const
{
	if (!m_bReady)
		return STATUS_REINITIALIZATION_NEEDED;

	BLEEDBLACK_INPUT_REQUEST request;
	RtlZeroMemory(&request, sizeof request);

	request.Move = TRUE;
	request.MovementX = x;
	request.MovementY = y;

	return CallDriver(m_hDevice, IOCTL_BLEEDBLACK_INPUT, &request, sizeof request, nullptr, 0, nullptr);
}

NTSTATUS CMouseClass::Click(ULONG_PTR pid, USHORT button, ULONG ReleaseDelayInMilliseconds) const
{
	if (!m_bReady)
		return STATUS_REINITIALIZATION_NEEDED;
	
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

	status = CallDriver(m_hDevice, IOCTL_BLEEDBLACK_INPUT, &request, sizeof(request), nullptr, 0, nullptr);
	if (!NT_SUCCESS(status))
		return status;

	status = NtDelayExecution(FALSE, &delayInterval);
	if (!NT_SUCCESS(status))
		return status;

	request.Move = FALSE;
	request.ButtonFlags = releaseButton;
	status = CallDriver(m_hDevice, IOCTL_BLEEDBLACK_INPUT, &request, sizeof(request), nullptr, 0, nullptr);
	return status;
}

BOOLEAN CMouseClass::IsReady() const
{
	return m_bReady;
}

CMouseClass::~CMouseClass()
{
	if (m_hDevice != INVALID_HANDLE_VALUE)
		NtClose(m_hDevice);
}
