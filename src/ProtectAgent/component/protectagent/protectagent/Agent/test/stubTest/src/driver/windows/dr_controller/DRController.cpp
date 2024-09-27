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

#include "log.h"

#include "util.h"

#include "controller.h"
#include "..\pnp_setup\pnp_setup.h"



typedef struct _PRAMETER
{
	int nOption;
	START_MIRROR_PARAM stParam;
	vector<ADD_DEL_PARTITION_PARAM> vecAddParam;
	INSTALL_PARAM itParam;
} PRAMETER, *PPRAMETER;


#define OM_AGENT_ID						L"696A65ED-4C61-487B-A188-09D749FD4613"
#define OM_WRITE_AGENT_ID			L"26BF22B2-5DA8-49C0-AA23-FE035653FF0B"
WCHAR* OM_VOL_ID[] = { L"6D850B37-4F3B-4141-A093-15E492B0C988", L"F022E007-AB51-4BFE-9E60-47ADFE0CD158" };

#define OM_DEVICE_KEY					L"root\\DRDriver"
#define OM_DISK_CLASS_NAME		L"DiskDrive"
#define OM_DEV_FILTER					L"DRDriver"



VOID ReadParameter(int argc, _TCHAR* argv[], PRAMETER& param);
VOID InstallDRDriver(const CString& strInfPath);
VOID RemoveDriver(const CString& strInfPath);
VOID UpdateDriver(const CString& strInfPath);
BOOL IsIoMirrorRunning();

void GuidToStream(const GUID* pSrc, char* result)
{
	unsigned long l_val = 0;
	unsigned short s_val = 0;

	l_val = htonl(pSrc->Data1);
	memcpy(result, &l_val, sizeof(l_val));
	result += sizeof(l_val);

	s_val = htons(pSrc->Data2);
	memcpy(result, &s_val, sizeof(s_val));
	result += sizeof(s_val);

	s_val = htons(pSrc->Data3);
	memcpy(result, &s_val, sizeof(s_val));
	result += sizeof(s_val);

	memcpy(result, pSrc->Data4, sizeof(pSrc->Data4));
}

int _tmain(int argc, _TCHAR* argv[])
{
	getchar();

	CString strLogPath = GetModulePath() + L"\\Log";
	CreateDirectories(strLogPath.GetString());

	SYSTEMTIME sysTime = { 0 };
	GetSystemTime(&sysTime);

	CString strLogFile;
	strLogFile.Format(L"%s\\%04d-%02d-%02d-%02d-%02d-%02d-%03d.log", strLogPath, sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
	SetLogPath(strLogFile.GetString());

	PRAMETER param = {0};
	ReadParameter(argc, argv, param);

	if (param.nOption == 1)
	{
		CDrController drCtl;
		UUID uuid = { 0 };
		UuidFromString((RPC_WSTR)OM_AGENT_ID, &uuid);
		GuidToStream(&uuid, param.stParam.szOsId);

		UuidFromString((RPC_WSTR)OM_WRITE_AGENT_ID, &uuid);
		GuidToStream(&uuid, param.stParam.szOmaId);

		param.stParam.sType = NORMAL_START;
		param.stParam.ulMaxSpeed = 128;
		drCtl.StartIoMirror(param.stParam);

		for (size_t i = 0; i < param.vecAddParam.size(); i++)
		{
			drCtl.AddDelPartition(param.vecAddParam[i], TRUE);
			if (param.stParam.bDelayAdd)
			{
				Sleep(1000);
			}
		}
	}
	else if (param.nOption == 2)
	{
		CDrController drCtl;
		drCtl.StopIoMirror();
	}
	else if (param.nOption == 3)
	{
		CDrController drCtl;
		drCtl.PauseIoMirror();
	}
	else if (param.nOption == 5)
	{
		CDrController drCtl;
		drCtl.ResumeIoMirror();
	}
	else if (param.nOption == 6)
	{
		CDrController drCtl;
		UUID uuid = { 0 };
		UuidFromString((RPC_WSTR)OM_AGENT_ID, &uuid);
		GuidToStream(&uuid, param.stParam.szOsId);

		UuidFromString((RPC_WSTR)OM_WRITE_AGENT_ID, &uuid);
		GuidToStream(&uuid, param.stParam.szOmaId);

		param.stParam.sType = MODIFY_START;
		param.stParam.ulMaxSpeed = 128;
		drCtl.StartIoMirror(param.stParam);
	}
	else if (param.nOption == 7)
	{
		CDrController drCtl;
		UUID uuid = { 0 };
		UuidFromString((RPC_WSTR)OM_AGENT_ID, &uuid);
		GuidToStream(&uuid, param.stParam.szOsId);

		UuidFromString((RPC_WSTR)OM_WRITE_AGENT_ID, &uuid);
		GuidToStream(&uuid, param.stParam.szOmaId);

		param.stParam.sType = VERIFY_START;
		param.stParam.ulMaxSpeed = 128;
		drCtl.StartIoMirror(param.stParam);

		for (size_t i = 0; i < param.vecAddParam.size(); i++)
		{
			drCtl.AddDelPartition(param.vecAddParam[i], TRUE);
			if (param.stParam.bDelayAdd)
			{
				Sleep(1000);
			}
		}
	}
	else if (param.nOption == 8)
	{
		InstallDRDriver(param.itParam.strInfPath);
	}
	else if (param.nOption == 9)
	{
		if (!param.itParam.bForce)
		{
			if (IsIoMirrorRunning())
			{
				wprintf(L"IoMirror is running, abort\n");
				return 0;
			}
		}

		UpdateDriver(param.itParam.strInfPath);
	}
	else if (param.nOption == 10)
	{
		if (IsIoMirrorRunning())
		{
			wprintf(L"IoMirror is running, abort\n");
			return 0;
		}

		RemoveDriver(param.itParam.strInfPath);
	}

    return 0;
}


VOID ReadParameter(int argc, _TCHAR* argv[], PRAMETER& param)
{
	if (argc >= 2)
	{
		if (wcsncmp(argv[1], L"-start", 6) == 0)
		{
			param.nOption = 1;
		}
		else if (wcsncmp(argv[1], L"-stop", 5) == 0)
		{
			param.nOption = 2;
		}
		else if (wcsncmp(argv[1], L"-pause", 6) == 0)
		{
			param.nOption = 3;
		}
		else if (wcsncmp(argv[1], L"-resume", 7) == 0)
		{
			param.nOption = 5;
		}
		else if (wcsncmp(argv[1], L"-modify", 7) == 0)
		{
			param.nOption = 6;
		}
		else if (wcsncmp(argv[1], L"-verify", 7) == 0)
		{
			param.nOption = 7;
		}
		else if (wcsncmp(argv[1], L"-install", 8) == 0)
		{
			param.nOption = 8;
		}
		else if (wcsncmp(argv[1], L"-update", 7) == 0)
		{
			param.nOption = 9;
		}
		else if (wcsncmp(argv[1], L"-remove", 7) == 0)
		{
			param.nOption = 10;
		}


		int i = 0;
		for (int nPos = 2; nPos < argc;)
		{
			if (wcsncmp(argv[nPos], L"-inf", 4) == 0)
			{
				nPos++;
				param.itParam.strInfPath = argv[nPos++];
			}
			else if (wcsncmp(argv[nPos], L"-i", 2) == 0)
			{
				nPos++;
				param.stParam.ulIp = inet_addr(CStringA(argv[nPos++]));
			}
			else if(wcsncmp(argv[nPos], L"-p", 2) == 0)
			{
				nPos++;
				param.stParam.ulPort = _wtoi(argv[nPos++]);
			}
			else if (wcsncmp(argv[nPos], L"-d", 2) == 0)
			{
				nPos++;

				ADD_DEL_PARTITION_PARAM aParam;
				aParam.ulDiskNumber = _wtoi(argv[nPos++]);

				memset(aParam.cPartitionId, 0, VOL_ID_LEN);

				UUID uuid = { 0 };
				UuidFromString((RPC_WSTR)OM_VOL_ID[i++], &uuid);
				GuidToStream(&uuid, aParam.cPartitionId);

				aParam.ullPartitionSize = -1;
				param.vecAddParam.push_back(aParam);
			}
			else if (wcsncmp(argv[nPos], L"-rpo", 4) == 0)
			{
				nPos++;
				param.stParam.exp_rpo = _wtoi(argv[nPos++]);
			}
			else if (wcsncmp(argv[nPos], L"-g", 2) == 0)
			{
				nPos++;
				param.stParam.ulDblgLevel = _wtoi(argv[nPos++]);
			}
			else if (wcsncmp(argv[nPos], L"-y", 2) == 0)
			{
				nPos++;
				param.stParam.bDelayAdd = TRUE;
			}
			else if (wcsncmp(argv[nPos], L"-f", 2) == 0)
			{
				nPos++;
				param.itParam.bForce = TRUE;
			}
		}
	}
	else
	{
		wprintf(L"Invalid parameter\n");
	}
}


VOID InstallDRDriver(const CString& strInfPath)
{
	DWORD dwError = 0;
	do
	{
		BOOL bReboot = FALSE;
		dwError = PnpInstall(strInfPath, OM_DEVICE_KEY, &bReboot);
		if (dwError != 0)
		{
			wprintf(L"Failed to do PnpInstall\n");
			break;
		}

		if (bReboot)
		{
			wprintf(L"Need to reboot in order to complete the installation\n");
		}

		dwError = PnpAddFilter(OM_DISK_CLASS_NAME, OM_DEV_FILTER);
		if (dwError != 0)
		{
			wprintf(L"Failed to do PnpAddFilter\n");
			break;
		}
	} while (0);
}

VOID RemoveDriver(const CString& strInfPath)
{
	DWORD dwError = 0;
	do
	{
		dwError = PnpRemoveFilter(OM_DISK_CLASS_NAME, OM_DEV_FILTER);
		if (dwError != 0)
		{
			wprintf(L"Failed to do PnpRemoveFilter\n");
			break;
		}

		BOOL bReboot = FALSE;
		dwError = PnpRemove(strInfPath, OM_DEVICE_KEY, &bReboot);
		if (dwError != 0)
		{
			wprintf(L"Failed to do PnpRemove\n");
			break;
		}

		if (bReboot)
		{
			wprintf(L"Need to reboot in order to complete the remove\n");
		}

		dwError = RemoveFilterService(OM_DEV_FILTER);
		if (dwError != 0)
		{
			wprintf(L"Failed to do RemoveFilterService\n");
			break;
		}
	} while (0);
}

VOID UpdateDriver(const CString& strInfPath)
{
	DWORD dwError = 0;
	do
	{
		dwError = PnpRemoveFilter(OM_DISK_CLASS_NAME, OM_DEV_FILTER);
		if (dwError != 0)
		{
			wprintf(L"Failed to do PnpRemoveFilter\n");
			break;
		}

		BOOL bReboot = FALSE;
		dwError = PnpUpdate(strInfPath, OM_DEVICE_KEY, &bReboot);
		if (dwError != 0)
		{
			wprintf(L"Failed to do PnpUpdate\n");
			break;
		}

		if (bReboot)
		{
			wprintf(L"Need to reboot in order to complete the update\n");
		}

		dwError = PnpAddFilter(OM_DISK_CLASS_NAME, OM_DEV_FILTER);
		if (dwError != 0)
		{
			wprintf(L"Failed to do PnpAddFilter\n");
			break;
		}
	} while (0);
}

BOOL IsIoMirrorRunning()
{
	CDrController drCtl;
	return drCtl.IsIoMirrorRunning();
}

