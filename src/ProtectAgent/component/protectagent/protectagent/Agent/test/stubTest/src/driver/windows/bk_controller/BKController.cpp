// BKController.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "log.h"

#include "util.h"

#include "controller.h"
#include "backup.h"



typedef struct _PRAMETER
{
	int nOption;
	START_MIRROR_PARAM stParam;
	vector<ADD_DEL_PARTITION_PARAM> vecAddParam;
	DWORD dwFirstTarget;
} PRAMETER, *PPRAMETER;


VOID ReadParameter(int argc, _TCHAR* argv[], PRAMETER& param);

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

	CBkController bkCtl;
	DWORD dwError = 0;
	if (param.nOption == 1)
	{
		dwError = bkCtl.StartIoMirror(param.stParam);
		if (dwError != 0)
		{
			IOMIRROR_ERROR(dwError, L"Failed to start Io mirror");
			return 0;
		}

		for (size_t i = 0; i < param.vecAddParam.size(); i++)
		{
			dwError = bkCtl.AddDelPartition(param.vecAddParam[i], TRUE);
			if (dwError != 0)
			{
				IOMIRROR_ERROR(dwError, L"Failed to add del partition");
				return 0;
			}
		}
	}
	else if (param.nOption == 2)
	{
		dwError = bkCtl.StopIoMirror();
		if (dwError != 0)
		{
			IOMIRROR_ERROR(dwError, L"Failed to stop Io mirror");
			return 0;
		}
	}
	else if (param.nOption == 3 || param.nOption == 5)
	{
		CBackup bk(&bkCtl);
		bk.DoBackup((param.nOption == 3), param.vecAddParam, param.dwFirstTarget);
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
		else if(wcsncmp(argv[1], L"-bkfull", 7) == 0)
		{
			param.nOption = 3;
		}
		else if (wcsncmp(argv[1], L"-bkinc", 6) == 0)
		{
			param.nOption = 5;
		}

		for (int nPos = 2; nPos < argc;)
		{
			if (wcsncmp(argv[nPos], L"-v", 2) == 0)
			{
				nPos++;

				ADD_DEL_PARTITION_PARAM aParam = {0};

				CString strVolume = argv[nPos++];
				if (strVolume.GetAt(strVolume.GetLength() - 1) == L'\\')
				{
					strVolume = strVolume.Left(strVolume.GetLength() - 1);
				}

				WCHAR wszTarget[MAX_PATH] = { 0 };
				DWORD dwError = QueryDosDevice(strVolume.GetString(), wszTarget, MAX_PATH);
				if (dwError == 0)
				{
					wprintf(L"Failed to query Dos name of volume %s, error 0x%08x", strVolume.GetString(), dwError);
					break;
				}

				strcpy_s(aParam.disk_path, CStringA(wszTarget).GetString());
				param.vecAddParam.push_back(aParam);
			}
			else if (wcsncmp(argv[nPos], L"-t", 2) == 0)
			{
				nPos++;
				param.dwFirstTarget = _wtoi(argv[nPos++]);
			}
		}
	}
	else
	{
		wprintf(L"Invalid parameter\n");
	}

}

