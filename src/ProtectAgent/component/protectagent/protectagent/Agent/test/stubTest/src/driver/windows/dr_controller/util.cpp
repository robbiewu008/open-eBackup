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
#include "util.h"
#include "log.h"



DWORD CreateDirectories(const CString& strPath)
{
	DWORD dwRet = 0;

	CString strSub;
	CString strCall = strPath;
	CString strGo;
	while (!strCall.IsEmpty())
	{
		int nPos = strCall.Find(L"\\");
		if (nPos > 0)
		{
			strSub = strCall.Left(nPos);
			strCall = strCall.Right(strCall.GetLength() - nPos - 1);
		}
		else
		{
			strSub = strCall;
			strCall.Empty();
		}

		strGo += strSub;

		DWORD dwAttrib = GetFileAttributes(strGo.GetString());
		if (dwAttrib == INVALID_FILE_ATTRIBUTES || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			if (!CreateDirectory(strGo.GetString(), NULL))
			{
				dwRet = GetLastError();
				break;
			}
		}

		strGo += L"\\";
	}

	return dwRet;
}



CString GetModulePath()
{
	CString strModule(L'\0', MAX_PATH);
	GetModuleFileName(NULL, strModule.GetBuffer(), strModule.GetLength());

	int nPos = strModule.ReverseFind(L'\\');
	if (nPos > 0)
	{
		strModule = strModule.Left(nPos);
	}

	return strModule;
}


BOOL RegDeleteKeyRecursive(HKEY hKeyRoot, const CString& strKey)
{
	DWORD dwRet = 0;
	
	do
	{
		dwRet = RegDeleteKey(hKeyRoot, strKey.GetString());
		if (dwRet == 0)
		{
			break;
		}

		HKEY hKey = NULL;
		dwRet = RegOpenKeyEx(hKeyRoot, strKey.GetString(), 0, KEY_READ, &hKey);
		if (dwRet != 0)
		{
			if (dwRet == ERROR_NOT_FOUND)
			{
				dwRet = 0;
			}
			else
			{
				IOMIRROR_ERROR(dwRet, L"Failed to open key %s", strKey.GetString());
			}

			break;
		}

		WCHAR wszSubName[MAX_PATH] = { 0 };
		DWORD dwSize = MAX_PATH;
		FILETIME ftLastWrite = { 0 };
		dwRet = RegEnumKeyEx(hKey, 0, wszSubName, &dwSize, NULL, NULL, NULL, &ftLastWrite);
		if (dwRet != 0)
		{
			RegCloseKey(hKey);
			IOMIRROR_ERROR(dwRet, L"Failed to enum key %s", strKey.GetString());
			break;
		}

		do
		{
			CString strSubKey = strKey + L"\\" + wszSubName;
			dwRet = RegDeleteKeyRecursive(hKey, strSubKey);
			if (dwRet != 0)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to delete sub key %s", strSubKey.GetString());
				break;
			}

			dwRet = RegEnumKeyEx(hKey, 0, wszSubName, &dwSize, NULL, NULL, NULL, &ftLastWrite);
			if (dwRet != 0 && dwRet != ERROR_NO_MORE_ITEMS)
			{
				IOMIRROR_ERROR(dwRet, L"Failed to enum key %s", strKey.GetString());
			}
		} while (dwRet == 0);

		RegCloseKey(hKey);

		dwRet = RegDeleteKey(hKeyRoot, strKey.GetString());
	} while (0);

	return dwRet;
}