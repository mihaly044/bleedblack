#include "pch.h"
#include "MouseClass.h"

#include <ntddmou.h>
#include <strsafe.h>
#include <bleedblack/io.h>

#include "Time.h"

extern "C" void _trigger_br(PBLEEDBLACK_INPUT_REQUEST input);

CMouseClass::CMouseClass()
	: m_ipc(nullptr),
	  m_hShmMapping(nullptr),
	  m_hShmEvt(nullptr),
	  m_hIpcProc(nullptr),
	  m_hIpcProcThreadHandle(nullptr),
	  m_bIpc(FALSE),
	  m_bIpcReady(FALSE)
{
}

CMouseClass::~CMouseClass()
{
	if (!m_bIpc)
		return;
	
	if (m_ipc)
	{
		m_ipc->ParentExiting = TRUE;
		SetEvent(m_hShmEvt);
		
		UnmapViewOfFile(m_ipc);
		DeleteCriticalSection(&m_cs);
	}

	if (m_hShmMapping)
		CloseHandle(m_hShmMapping);

	if (m_hShmEvt)
		CloseHandle(m_hShmEvt);
	
	if (m_hIpcProc)
		CloseHandle(m_hIpcProcThreadHandle);
	
	if (m_hIpcProc)
		CloseHandle(m_hIpcProc);
}

CMouseClass* CMouseClass::Create()
{
	return new CMouseClass;
}

NTSTATUS CMouseClass::Init()
{
	auto* dosh = static_cast<IMAGE_DOS_HEADER*>(NtCurrentPeb()->ImageBaseAddress);
	auto* nth = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<PUCHAR>(dosh) + dosh->e_lfanew);

	if (nth->FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE)
	{
		// Init IPC
		m_bIpc = TRUE;

		SECURITY_DESCRIPTOR sd;
		ZeroMemory(&sd, sizeof(sd));
		if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
			return __NTSTATUS_FROM_WIN32(GetLastError());
		
		if (!SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE))
			return __NTSTATUS_FROM_WIN32(GetLastError());

		SECURITY_ATTRIBUTES sa;
		ZeroMemory(&sa, sizeof sa);
		sa.nLength = sizeof sa;
		sa.lpSecurityDescriptor = &sd;
		sa.bInheritHandle = FALSE;
		
		m_hShmMapping = CreateFileMappingA(INVALID_HANDLE_VALUE,
			&sa, PAGE_READWRITE, 0, sizeof BLEEDBLACK_IPC, BLEEDBLACK_IPC_ENDPOINT);

		if (!m_hShmMapping)
			return __NTSTATUS_FROM_WIN32(GetLastError());

		auto* shm = MapViewOfFile(m_hShmMapping, 
			FILE_MAP_ALL_ACCESS, 0, 0, sizeof BLEEDBLACK_IPC);
		if(!shm)
			return __NTSTATUS_FROM_WIN32(GetLastError());
		
		m_ipc = static_cast<BLEEDBLACK_IPC*>(shm);
		
		ZeroMemory(m_ipc, sizeof BLEEDBLACK_IPC);
		
		m_hShmEvt = CreateEventA(&sa, TRUE, FALSE, BLEEDBLACK_IPC_EVT_NAME);
		if (!m_hShmEvt)
			return __NTSTATUS_FROM_WIN32(GetLastError());

		InitializeCriticalSection(&m_cs);

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof si);
		ZeroMemory(&pi, sizeof pi);
		si.cb = sizeof si;

		if(!CreateProcess(L"blipcsrv.exe", 
			nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
		{
			MessageBox(nullptr, L"Failed to start blipcsrv", L"Error", MB_ICONERROR | MB_OK);
			return __NTSTATUS_FROM_WIN32(GetLastError());
		}

		m_hIpcProc = pi.hProcess;
		m_hIpcProcThreadHandle = pi.hThread;
		
		m_bIpcReady = TRUE;
	}

	return STATUS_SUCCESS;
}

NTSTATUS CMouseClass::ProcessRequest(PBLEEDBLACK_INPUT_REQUEST pRequest) const
{
	if(m_ipc)
	{
		EnterCriticalSection(const_cast<PCRITICAL_SECTION>(&m_cs));
		RtlCopyMemory(&m_ipc->Req, pRequest, sizeof BLEEDBLACK_INPUT_REQUEST);
		const auto result = SetEvent(m_hShmEvt);
		LeaveCriticalSection(const_cast<PCRITICAL_SECTION>(&m_cs));
		return result ? STATUS_SUCCESS : STATUS_EVENT_PENDING;
	}
	
	if(pRequest->Move)
	{
		_trigger_br(pRequest);
		return STATUS_SUCCESS;
	}

	USHORT releaseButton;
	switch (pRequest->ButtonFlags)
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

	LARGE_INTEGER delayInterval = {};
	MakeRelativeIntervalMilliseconds(
		&delayInterval,
		pRequest->ReleaseDelayInMillis);

	_trigger_br(pRequest);

	const auto status = NtDelayExecution(FALSE, &delayInterval);
	if (!NT_SUCCESS(status))
		return status;

	pRequest->Move = FALSE;
	pRequest->ButtonFlags = releaseButton;

	_trigger_br(pRequest);
	return status;
}

NTSTATUS CMouseClass::Move(ULONG_PTR pid, LONG x, LONG y) const
{
	BLEEDBLACK_INPUT_REQUEST request;
	RtlZeroMemory(&request, sizeof request);

	request.Move = TRUE;
	request.MovementX = x;
	request.MovementY = y;

	return ProcessRequest(&request);
}

NTSTATUS CMouseClass::Click(ULONG_PTR pid, USHORT button, ULONG ReleaseDelayInMilliseconds) const
{
	BLEEDBLACK_INPUT_REQUEST request;
	RtlZeroMemory(&request, sizeof request);

	request.Move = FALSE;
	request.ButtonFlags = button;
	request.ReleaseDelayInMillis = ReleaseDelayInMilliseconds;

	return ProcessRequest(&request);
}

BOOLEAN CMouseClass::IsReady() const
{
	if (m_bIpc)
		return static_cast<BOOL>(m_bIpcReady);
	return TRUE;
}
