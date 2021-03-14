#pragma once

#include "bleedblack/io.h"

class CMouseClass
{
	explicit CMouseClass() noexcept;
public:
	static CMouseClass* Create();
	static BOOL NeedIpc();
	NTSTATUS InitIpc();
	NTSTATUS Init();
	NTSTATUS Move(ULONG_PTR pid, LONG x, LONG y) const;
	NTSTATUS Click(ULONG_PTR pid, USHORT button, ULONG ReleaseDelayInMilliseconds) const;
	BOOLEAN IsReady() const;

	NTSTATUS ProcessRequest(PBLEEDBLACK_INPUT_REQUEST pRequest) const;

	~CMouseClass();

private:
	CRITICAL_SECTION m_cs {};
	PBLEEDBLACK_INPUT_REQUEST m_req;
	HANDLE m_hShmMapping;
	HANDLE m_hIpcProc;
	HANDLE m_hJob;
	HANDLE m_hReq;
	HANDLE m_hAck;
	BOOL m_bIpc;
	BOOL m_bIpcReady;
};

