/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "stdafx.h"
#include "pnp_setup.h"
#include "log.h"

#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <newdev.h>
#include <regstr.h>

#include <strsafe.h>





typedef struct _ENUM_DEV_CONTEXT
{
	union
	{
		PBOOL pbReboot;
		BOOL bExist;
	};
	LPWSTR lpszDeviceId;
} ENUM_DEV_CONTEXT, *PENUM_DEV_CONTEXT;

typedef DWORD(*PFN_EnumCallback)(HDEVINFO hDevs, PSP_DEVINFO_DATA pDevInfo, DWORD dwIndex, PENUM_DEV_CONTEXT lpContext);
DWORD RemoveCallback(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfo, DWORD dwIndex, PENUM_DEV_CONTEXT lpContext);
DWORD EnumDeviceAndCallback(GUID* pClassId, LPWSTR lpszDeviceId, DWORD dwFlags, PFN_EnumCallback pfnCallback, PENUM_DEV_CONTEXT lpContext);
DWORD RemoveOemInfWithClassId(LPWSTR lpszClassId);

BOOL CheckFilterService(LPWSTR lpszFilter);
BOOL CheckInstalledDevice(GUID* pClassId, LPWSTR lpszDeviceId);
DWORD RemoveFilterService(LPWSTR lpszFilter);

LPWSTR* GetDevMultiSzArray(HDEVINFO hDevSet, PSP_DEVINFO_DATA pDevInfo, DWORD dwProp);
LPWSTR* GetRegMultiSzArray(HKEY hKey, LPWSTR lpszValue);
BOOL MultiSzArrayFind(LPWSTR* ppszArray, LPWSTR lpszFind);
VOID DelMultiSzArray(LPWSTR* ppszArray);
LPWSTR* CopyMultiSzArray(LPWSTR* ppszSource);
ULONG GetMultiSzArrayCount(LPWSTR* ppszArray);
ULONG GetMultiSzLenFromArray(LPWSTR* ppszArray);


DWORD PnpInstall(LPCWSTR lpszInfPath, LPWSTR lpszDeviceId,  PBOOL pbReboot)
{
	DWORD dwRet = 0;

	HDEVINFO hDevInfoSet = INVALID_HANDLE_VALUE;
	SP_DEVINFO_DATA spDevInfoData = { 0 };
	BOOL bDevInstalled = FALSE;
	do
	{
		GUID guidClassId;
		WCHAR wszClassName[MAX_CLASS_NAME_LEN] = { 0 };
		if (!SetupDiGetINFClass(lpszInfPath, &guidClassId, wszClassName, sizeof(wszClassName) / sizeof(WCHAR), NULL))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get class info by inf %s", lpszInfPath);
			break;
		}

		if (CheckInstalledDevice(&guidClassId, lpszDeviceId))
		{
			dwRet = ERROR_ALREADY_EXISTS;
			IOMIRROR_ERROR(dwRet, L"Device %s for driver %s already exists, abort", lpszDeviceId, wszClassName);
			break;
		}

		hDevInfoSet = SetupDiCreateDeviceInfoList(&guidClassId, NULL);
		if (hDevInfoSet == INVALID_HANDLE_VALUE)
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to create device info list for class %s", wszClassName);
			break;
		}

		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		if (!SetupDiCreateDeviceInfo(hDevInfoSet, wszClassName, &guidClassId, NULL, NULL, DICD_GENERATE_ID, &spDevInfoData))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to create device info for class %s", wszClassName);
			break;
		}

		if (!SetupDiSetDeviceRegistryProperty(hDevInfoSet, &spDevInfoData, SPDRP_HARDWAREID, (LPBYTE)lpszDeviceId, (wcslen(lpszDeviceId) + 1) * sizeof(WCHAR)))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to create device hw Id %s for class %s", lpszDeviceId, wszClassName);
			break;
		}

		if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, hDevInfoSet, &spDevInfoData))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to class install device %s for class %s", lpszDeviceId, wszClassName);
			break;
		}

		bDevInstalled = TRUE;

		dwRet = PnpUpdate(lpszInfPath, lpszDeviceId, pbReboot);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to update device %s for class %s", lpszDeviceId, wszClassName);
			break;
		}
	} while (0);

	if (dwRet != 0)
	{
		if (bDevInstalled)
		{
			if (!SetupDiCallClassInstaller(DIF_REMOVE, hDevInfoSet, &spDevInfoData))
			{
				IOMIRROR_ERROR(GetLastError(), L"Failed to call class install for device %s for remove", lpszDeviceId);
			}
		}
	}

	if (hDevInfoSet != INVALID_HANDLE_VALUE) 
	{
		SetupDiDestroyDeviceInfoList(hDevInfoSet);
		hDevInfoSet = INVALID_HANDLE_VALUE;
	}

	return dwRet;
}

DWORD PnpUpdate(LPCWSTR lpszInfPath, LPWSTR lpszDeviceId, PBOOL pbReboot)
{
	DWORD dwRet = 0;

	DWORD dwFlags = INSTALLFLAG_FORCE;
	if (!UpdateDriverForPlugAndPlayDevices(NULL, lpszDeviceId, lpszInfPath, dwFlags, pbReboot))
	{
		dwRet = GetLastError();
		IOMIRROR_ERROR(dwRet, L"Failed to update device %s with %s", lpszDeviceId, lpszInfPath);
	}
	else if (*pbReboot)
	{
		IOMIRROR_ERROR(0, L"A reboot is required for device %s with %s", lpszDeviceId, lpszInfPath);
	}

	return dwRet;
}

DWORD PnpRemove(LPCWSTR lpszInfPath, LPWSTR lpszDeviceId, PBOOL pbReboot)
{
	DWORD dwRet = 0;
	do
	{
		GUID guidClassId;
		WCHAR wszClassName[MAX_CLASS_NAME_LEN] = { 0 };
		if (!SetupDiGetINFClass(lpszInfPath, &guidClassId, wszClassName, sizeof(wszClassName) / sizeof(WCHAR), NULL))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get class info by inf %s", lpszInfPath);
			break;
		}

		ENUM_DEV_CONTEXT edContext = { 0 };
		edContext.pbReboot = pbReboot;
		edContext.lpszDeviceId = lpszDeviceId;
		dwRet = EnumDeviceAndCallback(&guidClassId, lpszDeviceId, DIGCF_PRESENT, RemoveCallback, &edContext);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to remove device %s", lpszDeviceId);
			break;
		}

		dwRet = RemoveOemInfWithClassId(wszClassName);
		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to remove oem inf with Id %s", wszClassName);
			break;
		}

		if (*pbReboot)
		{
			IOMIRROR_INFO(L"Need to reboot after removing device %s", lpszDeviceId);
		}
	} while (0);

	return dwRet;
}

DWORD PnpAddFilter(LPWSTR lpszClassKey, LPWSTR lpszFilter)
{
	DWORD dwRet = 0;
	HKEY hClassKey = (HKEY)INVALID_HANDLE_VALUE;
	LPWSTR* ppszValueArray = NULL;
	LPWSTR* ppszTempArray = NULL;
	do
	{
		if (!CheckFilterService(lpszFilter))
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Filter service %s doesn't exist", lpszFilter);
			break;
		}

		GUID guidClass;
		DWORD dwNum = 0;
		if(!SetupDiClassGuidsFromNameEx(lpszClassKey, &guidClass, 1, &dwNum, NULL, NULL))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get guid from string %s", lpszClassKey);
			break;
		}

		hClassKey = SetupDiOpenClassRegKeyEx(&guidClass, KEY_READ | KEY_WRITE, DIOCR_INSTALLER, NULL, NULL);
		if (hClassKey == INVALID_HANDLE_VALUE)
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get open class key %s", lpszClassKey);
			break;
		}

		ppszValueArray = GetRegMultiSzArray(hClassKey, REGSTR_VAL_UPPERFILTERS);
		if (!ppszValueArray)
		{
			ppszValueArray = CopyMultiSzArray(NULL);
			if (!ppszValueArray)
			{
				dwRet = -1;
				IOMIRROR_ERROR(dwRet, L"Failed to alloc value array");
				break;
			}
		}

		int nCount = GetMultiSzArrayCount(ppszValueArray);
		ppszTempArray = new LPWSTR[nCount + 2];
		if (!ppszTempArray)
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Failed to alloc temp arrary with count %d", nCount);
			break;
		}

		for (int i = 0; i < nCount; i++)
		{
			ppszTempArray[i] = ppszValueArray[i];
		}

		ppszTempArray[nCount] = lpszFilter;
		ppszTempArray[nCount + 1] = NULL;

		LPWSTR* ppszNew = CopyMultiSzArray(ppszTempArray);
		if (!ppszNew)
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Failed to alloc new arrary");
			break;
		}

		DelMultiSzArray(ppszValueArray);
		ppszValueArray = ppszNew;
		delete[] ppszTempArray;
		ppszTempArray = NULL;

		if (ppszValueArray[0])
		{
			DWORD dwLen = GetMultiSzLenFromArray(ppszValueArray);
			dwRet = RegSetValueEx(hClassKey, REGSTR_VAL_UPPERFILTERS, 0, REG_MULTI_SZ, (LPBYTE)ppszValueArray[-1], dwLen);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to set registry value");
				break;
			}
		}
		else
		{
			dwRet = RegDeleteValue(hClassKey, REGSTR_VAL_UPPERFILTERS);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to delete registry value");
				break;
			}
		}
	} while (0);

	if (ppszTempArray)
	{
		DelMultiSzArray(ppszTempArray);
		ppszTempArray = NULL;
	}

	if (ppszValueArray)
	{
		DelMultiSzArray(ppszValueArray);
		ppszValueArray = NULL;
	}

	if (hClassKey != (HKEY)INVALID_HANDLE_VALUE) 
	{
		RegCloseKey(hClassKey);
	}

	return dwRet;
}

DWORD PnpRemoveFilter(LPWSTR lpszClassKey, LPWSTR lpszFilter)
{
	DWORD dwRet = 0;
	HKEY hClassKey = (HKEY)INVALID_HANDLE_VALUE;
	LPWSTR* ppszValueArray = NULL;
	do
	{
		GUID guidClass;
		DWORD dwNum = 0;
		if (!SetupDiClassGuidsFromNameEx(lpszClassKey, &guidClass, 1, &dwNum, NULL, NULL))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get guid from string %s", lpszClassKey);
			break;
		}

		hClassKey = SetupDiOpenClassRegKeyEx(&guidClass, KEY_READ | KEY_WRITE, DIOCR_INSTALLER, NULL, NULL);
		if (hClassKey == INVALID_HANDLE_VALUE)
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get open class key %s", lpszClassKey);
			break;
		}

		ppszValueArray = GetRegMultiSzArray(hClassKey, REGSTR_VAL_UPPERFILTERS);
		if (!ppszValueArray)
		{
			ppszValueArray = CopyMultiSzArray(NULL);
			if (!ppszValueArray)
			{
				dwRet = -1;
				IOMIRROR_ERROR(dwRet, L"Failed to alloc value array");
				break;
			}
		}

		int nIndex = 0;
		for (nIndex = 0; ppszValueArray[nIndex]; nIndex++)
		{
			if (_wcsicmp(ppszValueArray[nIndex], lpszFilter) == 0)
			{
				break;
			}
		}

		if (!ppszValueArray[nIndex])
		{
			IOMIRROR_INFO(L"Cannot find filter %s, abort", lpszFilter);
			break;
		}

		for (int i = nIndex; ppszValueArray[i]; i++)
		{
			ppszValueArray[i] = ppszValueArray[i + 1];
		}

		LPWSTR* ppszNew = CopyMultiSzArray(ppszValueArray);
		if (!ppszNew)
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Failed to alloc new arrary");
			break;
		}

		DelMultiSzArray(ppszValueArray);
		ppszValueArray = ppszNew;

		if (ppszValueArray[0])
		{
			DWORD dwLen = GetMultiSzLenFromArray(ppszValueArray);
			dwRet = RegSetValueEx(hClassKey, REGSTR_VAL_UPPERFILTERS, 0, REG_MULTI_SZ, (LPBYTE)ppszValueArray[-1], dwLen);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to set registry value");
				break;
			}
		}
		else
		{
			dwRet = RegDeleteValue(hClassKey, REGSTR_VAL_UPPERFILTERS);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to delete registry value");
				break;
			}
		}
	} while (0);

	if (ppszValueArray)
	{
		DelMultiSzArray(ppszValueArray);
		ppszValueArray = NULL;
	}

	if (hClassKey != (HKEY)INVALID_HANDLE_VALUE)
	{
		RegCloseKey(hClassKey);
	}

	return dwRet;
}

DWORD GetServiceBinaryPath(SC_HANDLE hService, wstring& strPath)
{
	DWORD dwRet = 0;
	
	do
	{
		DWORD dwNeeded = sizeof(QUERY_SERVICE_CONFIG);
		vector<BYTE> vecBuffer;
		LPQUERY_SERVICE_CONFIG pConfig = NULL;

		do
		{
			vecBuffer.resize(dwNeeded);
			pConfig = (LPQUERY_SERVICE_CONFIG)&vecBuffer[0];
			if (QueryServiceConfig(hService, pConfig, vecBuffer.size(), &dwNeeded))
			{
				dwRet = 0;
				break;
			}

			dwRet = GetLastError();
		} while (dwRet == ERROR_INSUFFICIENT_BUFFER);

		if (dwRet != 0)
		{
			IOMIRROR_ERROR(dwRet, L"Failed to get service config");
			break;
		}
		
		if (!pConfig->lpBinaryPathName)
		{
			dwRet = -1;
			IOMIRROR_ERROR(dwRet, L"Failed to service binary path is empty");
			break;
		}

		WCHAR wszSearch[MAX_PATH] = { 0 };
		if (!GetWindowsDirectory(wszSearch, ARRAYSIZE(wszSearch)))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get windows directory");
			break;
		}

		strPath = pConfig->lpBinaryPathName;
		strPath.replace(0, wstring(L"\\SystemRoot").length(), wszSearch);
	} while (0);

	return dwRet;
}

DWORD RemoveFilterService(LPWSTR lpszFilter)
{
	DWORD dwRet = 0;

	SC_HANDLE hSevMgr = NULL;
	SC_HANDLE hService = NULL;
	wstring strBinaryPath;
	do
	{
		hSevMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (!hSevMgr)
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to open service manager");
			break;
		}

		hService = OpenService(hSevMgr, lpszFilter, SC_MANAGER_ALL_ACCESS);
		if (!hService)
		{
			dwRet = GetLastError();
			if (dwRet == ERROR_SERVICE_DOES_NOT_EXIST)
			{
				dwRet = 0;
				IOMIRROR_INFO(L"Service %s does not exist", lpszFilter);
			}
			else
			{
				IOMIRROR_ERROR(dwRet, L"Failed to open service %s", lpszFilter);
			}

			break;
		}

		GetServiceBinaryPath(hService, strBinaryPath);

		if (!DeleteService(hService))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to delete service %s", lpszFilter);
			break;
		}
	} while (0);

	if (hService)
	{
		CloseServiceHandle(hService);
		hService = NULL;
	}

	if (hSevMgr)
	{
		CloseServiceHandle(hSevMgr);
		hSevMgr = NULL;
	}

	if (dwRet != 0)
	{
		return dwRet;
	}

	IOMIRROR_INFO(L"Wait system to remove service %s", lpszFilter);
	do
	{
		Sleep(1000);
	} while (CheckFilterService(lpszFilter));
	IOMIRROR_INFO(L"Service %s is removed", lpszFilter);

	if (!strBinaryPath.empty())
	{
		if (!DeleteFile(strBinaryPath.c_str()))
		{
			IOMIRROR_ERROR(GetLastError(), L"Failed to delete service binary %s", strBinaryPath.c_str());
		}
	}

	return dwRet;
}

DWORD EnumDeviceAndCallback(GUID* pClassId, LPWSTR lpszDeviceId, DWORD dwFlags, PFN_EnumCallback pfnCallback, PENUM_DEV_CONTEXT lpContext)
{
	DWORD dwRet = 0;

	HDEVINFO hDevInfoSet = INVALID_HANDLE_VALUE;
	do
	{
		hDevInfoSet = SetupDiGetClassDevsEx(pClassId, NULL, NULL, (pClassId ? 0 : DIGCF_ALLCLASSES) | dwFlags, NULL, NULL, NULL);
		if (hDevInfoSet == INVALID_HANDLE_VALUE)
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get device info from class");
			break;
		}

		SP_DEVINFO_LIST_DETAIL_DATA dldData = { 0 };
		dldData.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
		if (!SetupDiGetDeviceInfoListDetail(hDevInfoSet, &dldData))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get device info list detail");
			break;
		}

		SP_DEVINFO_DATA devInfo = {0};
		devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
		BOOL bFound = FALSE;
		DWORD dwIndex = 0;
		while (!bFound)
		{
			if (!SetupDiEnumDeviceInfo(hDevInfoSet, dwIndex, &devInfo))
			{
				break;
			}

			LPWSTR* ppszHwIds = GetDevMultiSzArray(hDevInfoSet, &devInfo, SPDRP_HARDWAREID);
			LPWSTR* ppszCompatIds = GetDevMultiSzArray(hDevInfoSet, &devInfo, SPDRP_COMPATIBLEIDS);
			if (MultiSzArrayFind(ppszHwIds, lpszDeviceId) || MultiSzArrayFind(ppszCompatIds, lpszDeviceId))
			{
				bFound = TRUE;
			}

			DelMultiSzArray(ppszHwIds);
			DelMultiSzArray(ppszCompatIds);
			dwIndex++;
		}
		
		if (!bFound)
		{
			dwRet = ERROR_NOT_FOUND;
			IOMIRROR_ERROR(dwRet, L"Failed to find instance with device id %s", lpszDeviceId);
			break;
		}

		dwRet = pfnCallback(hDevInfoSet, &devInfo, dwIndex, lpContext);
	} while (0);


	if (hDevInfoSet != INVALID_HANDLE_VALUE)
	{
		SetupDiDestroyDeviceInfoList(hDevInfoSet);
		hDevInfoSet = INVALID_HANDLE_VALUE;
	}

	return dwRet;
}

DWORD RemoveCallback(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfo, DWORD dwIndex, PENUM_DEV_CONTEXT lpContext)
{
	DWORD dwRet = 0;
	do
	{
		SP_DEVINFO_LIST_DETAIL_DATA dldData = { 0 };
		dldData.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
		if (!SetupDiGetDeviceInfoListDetail(hDevInfoSet, &dldData))
		{
			IOMIRROR_ERROR(GetLastError(), L"Failed to find instance with device id %s", lpContext->lpszDeviceId);
			break;
		}

		SP_REMOVEDEVICE_PARAMS rmParam = { 0 };
		rmParam.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		rmParam.ClassInstallHeader.InstallFunction = DIF_REMOVE;
		rmParam.Scope = DI_REMOVEDEVICE_GLOBAL;
		if (!SetupDiSetClassInstallParams(hDevInfoSet, pDevInfo, &rmParam.ClassInstallHeader, sizeof(rmParam)))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to set class install param for device %s", lpContext->lpszDeviceId);
			break;
		}

		if (!SetupDiCallClassInstaller(DIF_REMOVE, hDevInfoSet, pDevInfo))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to call class install for device %s", lpContext->lpszDeviceId);
			break;
		}

		SP_DEVINSTALL_PARAMS diParam = { 0 };
		diParam.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
		if (!SetupDiGetDeviceInstallParams(hDevInfoSet, pDevInfo, &diParam))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get device install param for device %s", lpContext->lpszDeviceId);
			break;
		}

		if (diParam.Flags & (DI_NEEDRESTART | DI_NEEDREBOOT))
		{
			*lpContext->pbReboot = TRUE;
			IOMIRROR_INFO(L"Need to reboot for device %s", lpContext->lpszDeviceId);
		}
		else
		{
			*lpContext->pbReboot = FALSE;
		}
	} while (0);

	return dwRet;
}

DWORD RemoveOemInfWithClassId(LPWSTR lpszClassId)
{
	DWORD dwRet = 0;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	do
	{
		WCHAR wszSearch[MAX_PATH] = { 0 };
		if (!GetWindowsDirectory(wszSearch, ARRAYSIZE(wszSearch)))
		{
			dwRet = GetLastError();
			IOMIRROR_ERROR(dwRet, L"Failed to get windows directory");
			break;
		}

		dwRet = StringCchCat(wszSearch, ARRAYSIZE(wszSearch), L"\\INF\\OEM*.INF");
		if (FAILED(dwRet))
		{
			IOMIRROR_ERROR(dwRet, L"Failed to cat string");
			break;
		}

		WIN32_FIND_DATA fData = { 0 };
		hFind = FindFirstFile(wszSearch, &fData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			IOMIRROR_INFO(L"No oem inf file");
			break;
		}

		do
		{
			GUID guidClassId;
			WCHAR wszClassName[MAX_CLASS_NAME_LEN] = { 0 };
			if (!SetupDiGetINFClass(fData.cFileName, &guidClassId, wszClassName, sizeof(wszClassName) / sizeof(WCHAR), NULL))
			{
				IOMIRROR_ERROR(GetLastError(), L"Failed to get class info by inf %s", fData.cFileName);
				continue;
			}

			if (_wcsicmp(wszClassName, lpszClassId) == 0)
			{
				IOMIRROR_INFO(L"Removing oem file %s", fData.cFileName);

				if (!SetupUninstallOEMInf(fData.cFileName, 0, NULL))
				{
					IOMIRROR_ERROR(GetLastError(), L"Failed to remove inf %s", fData.cFileName);
					continue;
				}
			}
		} while (FindNextFile(hFind, &fData));

		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	} while (0);

	return dwRet;
}

LPWSTR* GetMultiSzArray(LPTSTR lpszMultiSz)
{
	LPWSTR* ppszRet = NULL;
	int nCount = 0;
	LPTSTR lpszPos = lpszMultiSz;
	while (lpszPos[0])
	{
		lpszPos += wcslen(lpszPos) + 1;
		nCount++;
	}

	ppszRet = new LPWSTR[nCount + 2];
	if (!ppszRet)
	{
		IOMIRROR_ERROR(-1, L"Failed to alloc ret string, size %d", nCount + 2);
		return ppszRet;
	}

	lpszPos = lpszMultiSz;
	ppszRet[0] = lpszMultiSz;
	ppszRet++;
	nCount = 0;
	while (lpszPos[0])
	{
		ppszRet[nCount] = lpszPos;
		lpszPos += wcslen(lpszPos) + 1;
		nCount++;
	}

	ppszRet[nCount] = NULL;

	return ppszRet;
}

LPWSTR* GetDevMultiSzArray(HDEVINFO hDevSet, PSP_DEVINFO_DATA pDevInfo, DWORD dwProp)
{
	LPWSTR* lpszRet = NULL;
	LPWSTR lpszProp = NULL;
	DWORD dwError = 0;
	DWORD dwSize = 4096;
	do
	{
		lpszProp = new WCHAR[dwSize / sizeof(WCHAR) + 2];
		if (lpszProp == NULL)
		{
			dwError = -1;
			IOMIRROR_ERROR(dwError, L"Failed to alloc buffer");
			break;
		}

		RtlZeroMemory(lpszProp, dwSize + sizeof(WCHAR) * 2);

		DWORD dwDataType = 0;
		while (!SetupDiGetDeviceRegistryProperty(hDevSet, pDevInfo, dwProp, &dwDataType, (LPBYTE)lpszProp, dwSize, &dwSize))
		{
			dwError = GetLastError();
			if (dwError != ERROR_INSUFFICIENT_BUFFER)
			{
				IOMIRROR_ERROR(dwError, L"Failed to get device property %d", dwProp);
				break;
			}

			if (dwDataType != REG_MULTI_SZ)
			{
				dwError = -1;
				IOMIRROR_ERROR(dwError, L"Data type for device is incorrect, %d", dwDataType);
				break;
			}

			dwError = 0;

			delete[] lpszProp;
			lpszProp = new WCHAR[dwSize / sizeof(WCHAR) + 2];
			if (lpszProp == NULL)
			{
				dwError = -1;
				IOMIRROR_ERROR(dwError, L"Failed to alloc buffer with size %d", dwSize);
				break;
			}

			RtlZeroMemory(lpszProp, dwSize + sizeof(WCHAR) * 2);
		}

		if (dwError != 0)
		{
			break;
		}

		lpszRet = GetMultiSzArray(lpszProp);
	} while (0);

	if (dwError != 0)
	{
		if (lpszProp)
		{
			delete[] lpszProp;
			lpszProp = NULL;
		}
	}

	return lpszRet;
}

BOOL MultiSzArrayFind(LPWSTR* ppszArray, LPWSTR lpszFind)
{
	BOOL bRet = FALSE;
	if (!ppszArray)
	{
		return bRet;
	}

	while (ppszArray[0])
	{
		if (_wcsicmp(ppszArray[0], lpszFind) == 0)
		{
			bRet = TRUE;
			break;
		}

		ppszArray++;
	}

	return bRet;
}

VOID DelMultiSzArray(LPWSTR* ppszArray)
{
	if (ppszArray)
	{
		ppszArray--;
		if (ppszArray[0])
		{
			delete[] ppszArray[0];
		}

		delete[] ppszArray;
	}
}

LPWSTR* GetRegMultiSzArray(HKEY hKey, LPWSTR lpszValue)
{
	LPWSTR* lpszRet = NULL;
	LPWSTR lpszBuffer = NULL;
	DWORD dwError = 0;
	DWORD dwSize = 4096;

	do
	{
		lpszBuffer = new WCHAR[dwSize / sizeof(WCHAR) + 2];
		if (lpszBuffer == NULL)
		{
			dwError = -1;
			IOMIRROR_ERROR(dwError, L"Failed to alloc buffer with size %d", dwSize);
			break;
		}

		RtlZeroMemory(lpszBuffer, dwSize + sizeof(WCHAR) * 2);

		DWORD dwDataType = 0;
		dwError = RegQueryValueEx(hKey, lpszValue, NULL, &dwDataType, (LPBYTE)lpszBuffer, &dwSize);
		while (dwError != 0)
		{
			if (dwError != ERROR_MORE_DATA)
			{
				IOMIRROR_ERROR(dwError, L"Failed to query registry value %s", lpszValue);
				break;
			}

			delete[] lpszBuffer;
			lpszBuffer = new WCHAR[dwSize / sizeof(WCHAR) + 2];
			if (lpszBuffer == NULL)
			{
				dwError = -1;
				IOMIRROR_ERROR(dwError, L"Failed to alloc buffer with size %d", dwSize);
				break;
			}

			RtlZeroMemory(lpszBuffer, dwSize + sizeof(WCHAR) * 2);

			dwError = RegQueryValueEx(hKey, lpszValue, NULL, &dwDataType, (LPBYTE)lpszBuffer, &dwSize);
		}

		if (dwError != 0)
		{
			break;
		}

		lpszRet = GetMultiSzArray(lpszBuffer);
	} while (0);

	if (dwError != 0)
	{
		delete[] lpszBuffer;
		lpszBuffer = NULL;
	}

	return lpszRet;
}

LPWSTR* CopyMultiSzArray(LPWSTR* ppszSource)
{
	LPWSTR* ppszRet = NULL;
	LPWSTR lpszMultiSz = NULL;
	int nSize = 0;
	DWORD dwError = 0;

	do
	{
		if (ppszSource)
		{
			for (int i = 0; ppszSource[i]; i++)
			{
				nSize += wcslen(ppszSource[i]) + 1;
			}
		}

		nSize += 1;

		lpszMultiSz = new WCHAR[nSize];
		if (!lpszMultiSz)
		{
			dwError = -1;
			IOMIRROR_ERROR(dwError, L"Failed to alloc buffer with size %d", nSize);
			break;
		}

		RtlZeroMemory(lpszMultiSz, sizeof(WCHAR) * nSize);

		int nPos = 0;
		if (ppszSource)
		{
			for (int i = 0; ppszSource[i]; i++)
			{
				dwError = StringCchCopy(lpszMultiSz + nPos, nSize - nPos, ppszSource[i]);
				if (FAILED(dwError))
				{
					IOMIRROR_ERROR(dwError, L"Failed to copy string");
					break;
				}

				nPos += wcslen(lpszMultiSz + nPos) + 1;
			}

			if (dwError != 0)
			{
				break;
			}
		}

		ppszRet = GetMultiSzArray(lpszMultiSz);
	} while (0);

	if (dwError != 0)
	{
		if (lpszMultiSz)
		{
			delete[] lpszMultiSz;
			lpszMultiSz = NULL;
		}
	}

	return ppszRet;
}

ULONG GetMultiSzArrayCount(LPWSTR* ppszArray)
{
	ULONG ulRet = 0;
	for (ulRet = 0; ppszArray[ulRet]; ulRet++)
	{
	}

	return ulRet;
}

ULONG GetMultiSzLenFromArray(LPWSTR* ppszArray)
{
	LPWSTR lpszMultiSz = ppszArray[-1];
	LPWSTR lpszPos = lpszMultiSz;
	while (wcslen(lpszPos) > 0)
	{
		lpszPos += wcslen(lpszPos) + 1;
	}

	lpszPos++;

	ULONG ulLen = (lpszPos - lpszMultiSz) * sizeof(WCHAR);

	return ulLen;
}

BOOL CheckFilterService(LPWSTR lpszFilter)
{
	BOOL bRet = FALSE;
	SC_HANDLE hSevMgr = NULL;
	SC_HANDLE hService = NULL;
	DWORD dwError = 0;
	do
	{
		hSevMgr = OpenSCManager(NULL, NULL, GENERIC_READ);
		if (!hSevMgr)
		{
			dwError = GetLastError();
			IOMIRROR_ERROR(dwError, L"Failed to open service manager");
			break;
		}

		hService = OpenService(hSevMgr, lpszFilter, GENERIC_READ);
		if (hService)
		{
			bRet = TRUE;
		}
	} while (0);

	if (hService)
	{
		CloseServiceHandle(hService);
		hService = NULL;
	}

	if (hSevMgr)
	{
		CloseServiceHandle(hSevMgr);
		hSevMgr = NULL;
	}

	return bRet;
}

DWORD CheckInstalledCallback(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfo, DWORD dwIndex, PENUM_DEV_CONTEXT lpContext)
{
	lpContext->bExist = TRUE;
	return 0;
}

BOOL CheckInstalledDevice(GUID* pClassId, LPWSTR lpszDeviceId)
{
	ENUM_DEV_CONTEXT edContext = { 0 };
	edContext.bExist = FALSE;
	edContext.lpszDeviceId = lpszDeviceId;
	DWORD dwError = EnumDeviceAndCallback(pClassId, lpszDeviceId, DIGCF_PRESENT, CheckInstalledCallback, &edContext);
	if (dwError != 0 && dwError != ERROR_NOT_FOUND)
	{
		IOMIRROR_ERROR(dwError, L"Failed to enum device %s", lpszDeviceId);
		return TRUE;
	}

	return edContext.bExist;
}