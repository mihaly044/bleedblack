#include "pch.h"
#include "MouseClass.h"

#include <ntddmou.h>
#include <bleedblack/io.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "Time.h"

extern "C" void _trigger_br(PBLEEDBLACK_INPUT_REQUEST input);

CMouseClass::CMouseClass() noexcept
	: m_req(nullptr),
	  m_hShmMapping(nullptr),
	  m_hIpcProc(nullptr),
	  m_hJob(nullptr),
	  m_hReq(nullptr),
	  m_hAck(nullptr),
	  m_bIpc(FALSE),
	  m_bIpcReady(FALSE)
{
	if (!m_logger)
	{
		const auto max_size = 1048576 * 5;
		const auto max_files = 3;
		m_logger = spdlog::rotating_logger_mt("bleedblack", "bleedblack.log", max_size, max_files);

		spdlog::flush_every(std::chrono::seconds(5));
		spdlog::flush_on(spdlog::level::warn);
	}
}

CMouseClass::~CMouseClass()
{
	if (!m_bIpc)
		return;

	if (m_req)
	{
		UnmapViewOfFile(m_req);
		DeleteCriticalSection(&m_cs);
	}

	if (m_hReq)
		CloseHandle(m_hReq);

	if (m_hAck)
		CloseHandle(m_hAck);

	if (m_hShmMapping)
		CloseHandle(m_hShmMapping);

	if (m_hJob)
		CloseHandle(m_hJob);
	
	if (m_hIpcProc)
		CloseHandle(m_hIpcProc);
}

CMouseClass* CMouseClass::Create()
{
	return new CMouseClass;
}

NTSTATUS CMouseClass::Init()
{
	auto *logger = m_logger.get();
	logger->info("Starting up");

	PIMAGE_DOS_HEADER dosh = static_cast<IMAGE_DOS_HEADER*>(NtCurrentPeb()->ImageBaseAddress);
	PIMAGE_NT_HEADERS nth = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<PUCHAR>(dosh) + dosh->e_lfanew);

	if (nth->FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE)
	{
		logger->info("Enabling IPC proxy");
		
		// Init IPC
		m_bIpc = TRUE;

		SECURITY_DESCRIPTOR sd;
		ZeroMemory(&sd, sizeof(sd));
		DWORD dwError;
		if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
		{
			dwError = GetLastError();
			logger->critical("Failed to initialize security descriptor, error {}", dwError);
			return __NTSTATUS_FROM_WIN32(dwError);
		}
			
		if (!SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE))
		{
			dwError = GetLastError();
			logger->critical("Failed to set security descriptor, error {}", dwError);
			return __NTSTATUS_FROM_WIN32(dwError);
		}

		SECURITY_ATTRIBUTES sa;
		ZeroMemory(&sa, sizeof sa);
		sa.nLength = sizeof sa;
		sa.lpSecurityDescriptor = &sd;
		sa.bInheritHandle = TRUE;
		
		m_hShmMapping = CreateFileMappingA(INVALID_HANDLE_VALUE,
			&sa, PAGE_READWRITE, 0, sizeof BLEEDBLACK_INPUT_REQUEST, nullptr);

		if (!m_hShmMapping)
		{
			dwError = GetLastError();
			logger->critical("Failed to create shared memory mapping, error {}", dwError);
			return __NTSTATUS_FROM_WIN32(dwError);
		}
		
		PVOID shm = MapViewOfFile(m_hShmMapping, 
			FILE_MAP_ALL_ACCESS, 0, 0, sizeof BLEEDBLACK_INPUT_REQUEST);
		if(!shm)
		{
			dwError = GetLastError();
			logger->critical("Failed to open shared memory, error {}", dwError);
			return __NTSTATUS_FROM_WIN32(dwError);
		}
		
		m_req = static_cast<BLEEDBLACK_INPUT_REQUEST*>(shm);
		
		ZeroMemory(m_req, sizeof BLEEDBLACK_INPUT_REQUEST);
		
		m_hReq = CreateEventA(&sa, TRUE, FALSE, nullptr);
		if (!m_hReq)
		{
			dwError = GetLastError();
			logger->critical("Failed to create req event, error {}", dwError);
			return __NTSTATUS_FROM_WIN32(dwError);
		}

		m_hAck = CreateEventA(&sa, TRUE, FALSE, nullptr);
		if (!m_hAck)
		{
			dwError = GetLastError();
			logger->critical("Failed to create ack event, error {}", dwError);
			return __NTSTATUS_FROM_WIN32(dwError);
		}

		InitializeCriticalSection(&m_cs);

		STARTUPINFOW si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof si);
		ZeroMemory(&pi, sizeof pi);
		si.cb = sizeof si;

		BOOL bIsProcessInJob;
		if(!IsProcessInJob(GetCurrentProcess(), nullptr, &bIsProcessInJob))
		{
			dwError = GetLastError();
			logger->critical("IsProcessInJob error {}", dwError);
			return __NTSTATUS_FROM_WIN32(dwError);
		}

		DWORD dwCreationFlags = CREATE_SUSPENDED;
		if (bIsProcessInJob)
			dwCreationFlags |= CREATE_BREAKAWAY_FROM_JOB;

		WCHAR buffer[MAX_PATH];
		wsprintfW(buffer, L"blipcsrv.exe %I64u %I64u %I64u", (ULONG_PTR)m_hShmMapping, (ULONG_PTR)m_hReq, (ULONG_PTR)m_hAck);
		
		if(!CreateProcess(L"blipcsrv.exe", 
			buffer, nullptr, nullptr, TRUE, dwCreationFlags, nullptr, nullptr, &si, &pi))
		{
			dwError = GetLastError();
			logger->critical("Failed to spawn proxy process {}", dwError);
			
			return __NTSTATUS_FROM_WIN32(dwError);
		}

		m_hJob = CreateJobObjectA(nullptr, nullptr);
		if(!m_hJob)
		{
			dwError = GetLastError();
			logger->critical("Failed to create job object {}", dwError);

			return __NTSTATUS_FROM_WIN32(dwError);
		}
		
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
		jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		if(!SetInformationJobObject(m_hJob, 
			JobObjectExtendedLimitInformation, &jeli, sizeof jeli))
		{
			dwError = GetLastError();
			logger->critical("SetInformationJobObject error {}", dwError);

			return __NTSTATUS_FROM_WIN32(dwError);
		}

		if(!AssignProcessToJobObject(m_hJob, pi.hProcess))
		{
			dwError = GetLastError();
			logger->critical("AssignProcessToJobObject error {}", dwError);

			return __NTSTATUS_FROM_WIN32(dwError);
		}

		ResumeThread(pi.hThread);

		m_hIpcProc = pi.hProcess;
		
		m_bIpcReady = TRUE;
	}

	return STATUS_SUCCESS;
}

NTSTATUS CMouseClass::ProcessRequest(PBLEEDBLACK_INPUT_REQUEST pRequest) const
{
	if(m_req)
	{
		EnterCriticalSection(const_cast<PCRITICAL_SECTION>(&m_cs));
		RtlCopyMemory(m_req, pRequest, sizeof BLEEDBLACK_INPUT_REQUEST);
		const auto result = SetEvent(m_hReq);

		const auto dwResult = WaitForSingleObject(m_hAck, 2000);
		if(dwResult != WAIT_OBJECT_0)
			m_logger->warn("Waiting for ack has failed {}", result);
		
		LeaveCriticalSection(const_cast<PCRITICAL_SECTION>(&m_cs));
		ResetEvent(m_hAck);
		return result && dwResult == STATUS_WAIT_0 ? STATUS_SUCCESS : STATUS_EVENT_PENDING;
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

	request.ProcessId = pid;
	request.Move = TRUE;
	request.MovementX = x;
	request.MovementY = y;

	return ProcessRequest(&request);
}

NTSTATUS CMouseClass::Click(ULONG_PTR pid, USHORT button, ULONG ReleaseDelayInMilliseconds) const
{
	BLEEDBLACK_INPUT_REQUEST request;
	RtlZeroMemory(&request, sizeof request);

	request.ProcessId = pid;
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
