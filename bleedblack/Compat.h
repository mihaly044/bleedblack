#pragma once

NTSTATUS GetNonDefaultUpperFilters(std::vector<std::wstring>& filters);
NTSTATUS UninstallUpperFilter(LPWSTR lpFilterName);
