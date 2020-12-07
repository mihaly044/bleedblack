#pragma once

class CMouseClass
{
	HANDLE m_hDevice;
	BOOLEAN m_bReady;

	explicit CMouseClass();

protected:
	static NTSTATUS OpenDriver(LPCWSTR lpName, ACCESS_MASK dwAccessMask, PHANDLE pDeviceHandleOut);
	static NTSTATUS CallDriver(HANDLE hDevice, ULONG ulCtlCode, PVOID pInput, ULONG cbInput, PVOID pOutput, ULONG cbOutput, PDWORD returnLength);

public:
	static CMouseClass* Create();
	NTSTATUS Init();
	NTSTATUS Move(ULONG_PTR pid, LONG x, LONG y) const;
	NTSTATUS Click(ULONG_PTR pid, USHORT button, ULONG ReleaseDelayInMilliseconds) const;
	BOOLEAN IsReady() const;

	~CMouseClass();
};

