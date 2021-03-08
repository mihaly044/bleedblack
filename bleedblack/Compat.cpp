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

static std::unique_ptr<BYTE[]> QueryPartialInformation(LPCWSTR lpValueName, HANDLE hkKey)
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

		auto buf = std::make_unique<BYTE[]>(resultLength);
		status = NtQueryValueKey(hkKey, 
			&szValueName, KeyValuePartialInformation, static_cast<PVOID>(buf.get()), resultLength, &resultLength);
		if(NT_SUCCESS(status))
		{
			return buf;
		}

		PLOG_ERROR.printf("Got unexpected status from NtQueryValueKey NTSTATUS %08X", status);
	}

	PLOG_ERROR.printf("Got unexpected status from NtQueryValueKey NTSTATUS %08X", status);
	return {};
}

std::vector<std::wstring> GetNonDefaultUpperFilters()
{
	std::vector<std::wstring> results;
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
	
	if(NT_SUCCESS(status))
	{
		const auto classPartialInformationBuf = QueryPartialInformation(L"Class", hkMouseClass);
		if(classPartialInformationBuf)
		{
			const auto *classPartialInformation = reinterpret_cast<KEY_VALUE_PARTIAL_INFORMATION*>(classPartialInformationBuf.get());
			const auto *classValue = reinterpret_cast<LPCWSTR>(classPartialInformation->Data);

			if(_wcsicmp(classValue, L"Mouse") == 0)
			{
				const auto upperFiltersInformationBuf = QueryPartialInformation(L"UpperFilters", hkMouseClass);
				if(upperFiltersInformationBuf)
				{
					const auto* upperFiltersPartialInformation = reinterpret_cast<KEY_VALUE_PARTIAL_INFORMATION*>(upperFiltersInformationBuf.get());
					const auto* upperFiltersValue = reinterpret_cast<LPCWSTR>(upperFiltersPartialInformation->Data);

					std::wistringstream stream(upperFiltersValue);
					std::wstring line;
					while (std::getline(stream, line))
						if (!line.empty() && line != std::wstring(L"mouclass"))
							results.push_back(line);
				}
				else
				{
					PLOG_ERROR << "Failed to query upper filters information";
				}
			}
			else
			{
				PLOG_ERROR << "Got unexpected class name " << classValue;
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
	return results;
}

NTSTATUS UninstallUpperFilter(LPWSTR lpFilterName)
{
	return STATUS_NOT_IMPLEMENTED;
}

