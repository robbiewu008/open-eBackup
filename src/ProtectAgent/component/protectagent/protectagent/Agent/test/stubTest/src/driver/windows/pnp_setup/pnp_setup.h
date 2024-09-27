#pragma once

#include <windows.h>


DWORD PnpInstall(LPCWSTR lpszInfPath, LPWSTR lpszDeviceId, PBOOL pbReboot);
DWORD PnpUpdate(LPCWSTR lpszInfPath, LPWSTR lpszDeviceId, PBOOL pbReboot);
DWORD PnpRemove(LPCWSTR lpszInfPath, LPWSTR lpszDeviceId, PBOOL pbReboot);

DWORD PnpAddFilter(LPWSTR lpszClassKey, LPWSTR lpszFilter);
DWORD PnpRemoveFilter(LPWSTR lpszClassKey, LPWSTR lpszFilter);

DWORD RemoveFilterService(LPWSTR lpszFilter);

