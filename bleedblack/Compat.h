#pragma once

std::vector<std::wstring> GetNonDefaultUpperFilters();
NTSTATUS UninstallUpperFilter(LPWSTR lpFilterName);
