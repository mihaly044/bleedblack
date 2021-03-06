#pragma once
#include "bleedblack/io.h"

class __declspec(dllexport) CMouseClass
{
	explicit CMouseClass();
public:
	static CMouseClass* Create();
	NTSTATUS Init();
	NTSTATUS Move(ULONG_PTR pid, LONG x, LONG y) const;
	NTSTATUS Click(ULONG_PTR pid, USHORT button, ULONG ReleaseDelayInMilliseconds) const;
	BOOLEAN IsReady() const;

	NTSTATUS ProcessRequest(PBLEEDBLACK_INPUT_REQUEST pRequest) const;

	~CMouseClass();

private:
	CRITICAL_SECTION m_cs {};
	PBLEEDBLACK_IPC m_ipc;
	HANDLE m_hShmMapping;
	HANDLE m_hIpcProc;
	HANDLE m_hIpcProcThreadHandle;
	HANDLE m_hJob;
	BOOL m_bIpc;
	BOOL m_bIpcReady;
};

