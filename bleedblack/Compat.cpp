#include "pch.h"
#include "Compat.h"

static NTSTATUS OpenControlKey(ACCESS_MASK desiredAccess, HANDLE* phkControl)
{
	UNICODE_STRING szKey;
	OBJECT_ATTRIBUTES obja;
	RtlInitUnicodeString(&szKey, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\");
	InitializeObjectAttributes(&obja, &szKey, OBJ_CASE_INSENSITIVE, nullptr, nullptr)
	return NtOpenKey(phkControl, desiredAccess, &obja);
}

static NTSTATUS OpenSubkey(ACCESS_MASK desiredAccess, HANDLE hkParent, UNICODE_STRING* subkeyName, HANDLE* phkSubkey)
{
	OBJECT_ATTRIBUTES obja;
	InitializeObjectAttributes(&obja, subkeyName, OBJ_CASE_INSENSITIVE, hkParent, nullptr)
	return NtOpenKey(phkSubkey, desiredAccess, &obja);
}

static NTSTATUS QueryPartialInformation(LPCWSTR lpValueName, HANDLE hkKey, std::vector<BYTE>& data)
{
	ULONG resultLength;
	UNICODE_STRING szValueName;
	RtlInitUnicodeString(&szValueName, lpValueName);

	auto status = NtQueryValueKey(hkKey, 
	                              &szValueName, KeyValuePartialInformation, nullptr, 0, &resultLength);
	if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
	{
		// TODO: Is this needed?
		resultLength += sizeof KEY_VALUE_PARTIAL_INFORMATION;

		data.resize(resultLength);
		status = NtQueryValueKey(hkKey, 
			&szValueName, KeyValuePartialInformation, &data[0], resultLength, &resultLength);
		if(NT_SUCCESS(status))
			return status;

		PLOG_ERROR.printf("Got unexpected status from NtQueryValueKey NTSTATUS %08X", status);
	}

	PLOG_ERROR.printf("Got unexpected status from NtQueryValueKey NTSTATUS %08X", status);
	return status;
}

NTSTATUS GetNonDefaultUpperFilters(std::vector<std::wstring>& filters)
{
	HANDLE hkControl = nullptr;
	auto status = OpenControlKey(KEY_ALL_ACCESS, &hkControl);
	
	if(!NT_SUCCESS(status))
	{
		PLOG_WARNING.printf("OpenControlKey() failed NTSTATUS %08X", status);
		return {};
	}

	UNICODE_STRING szMouseClass;
	HANDLE hkMouseClass;
	RtlInitUnicodeString(&szMouseClass, L"Class\\{4d36e96f-e325-11ce-bfc1-08002be10318}");
	status = OpenSubkey(KEY_ALL_ACCESS, hkControl, &szMouseClass, &hkMouseClass);

	if (NT_SUCCESS(status))
	{
		std::vector<BYTE> data;
		status = QueryPartialInformation(L"Class", hkMouseClass, data);

		if (NT_SUCCESS(status))
		{
			const auto* classNameInfo = reinterpret_cast<KEY_VALUE_PARTIAL_INFORMATION*>(&data[0]);
			const auto* classNameValue = reinterpret_cast<LPCWSTR>(classNameInfo->Data);

			if (_wcsicmp(classNameValue, L"Mouse") == 0)
			{
				status = QueryPartialInformation(L"UpperFilters", hkMouseClass, data);
				
				const auto* upperFiltersInfo = reinterpret_cast<KEY_VALUE_PARTIAL_INFORMATION*>(&data[0]);
				auto* upperFiltersValue = reinterpret_cast<const wchar_t*>(upperFiltersInfo->Data);

				if (NT_SUCCESS(status))
				{
					const wchar_t* ptr = upperFiltersValue;
					while (*ptr != L'\0')
					{
						const size_t len = wcslen(ptr);

						if (wcscmp(ptr, L"mouclass") != 0)
						{
							filters.push_back(std::wstring(ptr, len));
						}
						
						ptr += len + 1;
					}
				}
				else
				{
					PLOG_WARNING.printf("Failed to query for UpperFilters NTSTATUS %08X", status);
				}
			}
			else
			{
				PLOG_ERROR << "Got unexpected class name " << classNameValue;
			}
		}
		else
		{
			PLOG_ERROR << "Failed to query class name information";
		}

		NtClose(hkMouseClass);
	}
	else
	{
		PLOG_WARNING.printf("Failed to open class sub key NTSTATUS %08X", status);
	}

	NtClose(hkControl);
	return status;
}

NTSTATUS UninstallUpperFilter(LPWSTR lpFilterName)
{
	return STATUS_NOT_IMPLEMENTED;
}

