#pragma once

class CMouseClass
{
	explicit CMouseClass();
public:
	static CMouseClass* Create();
	static NTSTATUS Init();
	static NTSTATUS Move(ULONG_PTR pid, LONG x, LONG y);
	static NTSTATUS Click(ULONG_PTR pid, USHORT button, ULONG ReleaseDelayInMilliseconds);
	static BOOLEAN IsReady();

	~CMouseClass();
};

