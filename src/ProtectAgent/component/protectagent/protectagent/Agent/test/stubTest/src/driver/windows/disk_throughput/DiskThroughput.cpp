// DiskThroughput.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <time.h>

#include <atlstr.h>

#include "log.h"
#include "util.h"


ULONG BLOCK_SIZE_CAD[]{ 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048 };
#define WRITE_BUFFER_SIZE			(2 * 1024 * 1024)
#define BUFFER_FILL_BYTE			128

VOID WriteDiskUnderThroughput(HANDLE hDisk, ULONGLONG ullDiskSize, const CString& strDisk, ULONG ulThroughput, BYTE* pBuffer, ULONG ulBlockSize);

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


	CString strDisk;
	strDisk.Format(L"\\\\.\\PhysicalDrive%s", argv[1]);

	ULONG ulThroughput = _wtoi(argv[2]);
	ulThroughput = ulThroughput << 20;

	ULONGLONG ullDiskSize = _wtoi(argv[3]);
	ullDiskSize = ullDiskSize << 30;

	ULONG ulBlockSize = 0;
	if (argc == 5)
	{
		ulBlockSize = _wtoi(argv[4]);
	}

	HANDLE hDisk = CreateFile(strDisk.GetString(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDisk == INVALID_HANDLE_VALUE)
	{
		IOMIRROR_ERROR(GetLastError(), L"Failed to open disk %s", strDisk.GetString());
		return 0;
	}

	BYTE* pBuffer = new BYTE[WRITE_BUFFER_SIZE];
	memset(pBuffer, BUFFER_FILL_BYTE, WRITE_BUFFER_SIZE);

	WriteDiskUnderThroughput(hDisk, ullDiskSize, strDisk, ulThroughput, pBuffer, ulBlockSize);

    return 0;
}

#define MAX_RANDOM			32767
double RandomPercent()
{
	srand(time(NULL));
	double ret = rand();
	ret /= MAX_RANDOM;

	return ret;
}

ULONGLONG RangeRandom64(ULONGLONG low, ULONGLONG high)
{
	double percent = RandomPercent();
	ULONGLONG ret = (high - low) * percent;

	return ret;
}

ULONG RangeRandom32(ULONG low, ULONG high)
{
	srand(time(NULL));
	ULONG ret = (rand() % (high + 1 - low)) + low;

	return ret;
}

#define SECTOR_SIZE			(512)
VOID WriteDiskUnderThroughput(HANDLE hDisk, ULONGLONG ullDiskSize, const CString& strDisk, ULONG ulThroughput, BYTE* pBuffer, ULONG ulBlockSize)
{
	ULONGLONG ullOffset = 0;
	ULONG ulSize = 0;
	DWORD dwError = 0;
	DWORD dwWritten = 0;
	DWORD dwTimingPoint = 0;
	DWORD dwCurTick = 0;
	ULONG ulWrittenSize = 0;
	while (1)
	{
		dwCurTick = GetTickCount();
		if (dwCurTick - dwTimingPoint >= 1000)
		{
			dwTimingPoint = dwCurTick;
			ulWrittenSize = 0;
		}

		if (ulWrittenSize >= ulThroughput)
		{
			continue;
		}

		if (ullOffset >= ullDiskSize)
		{
			ullOffset = 0;
		}

		LARGE_INTEGER liOffset;
		liOffset.QuadPart = ullOffset;
		if(!SetFilePointerEx(hDisk, liOffset, NULL, FILE_BEGIN))
		{
			dwError = GetLastError();
			IOMIRROR_ERROR(dwError, L"Failed to set disk pos to %lld", ullOffset);
			break;
		}

		if (ulBlockSize == 0)
		{
			ulSize = RangeRandom32(0, 9);
			ulSize = BLOCK_SIZE_CAD[ulSize];
		}
		else
		{
			ulSize = ulBlockSize;
		}

		ulSize = ulSize << 10;

		if (!WriteFile(hDisk, pBuffer, ulSize, &dwWritten, NULL))
		{
			dwError = GetLastError();
			IOMIRROR_ERROR(dwError, L"Failed to write disk %s with size %d", strDisk.GetString(), ulSize);
			break;
		}

		ulWrittenSize += dwWritten;
		ullOffset += ulSize;
	}
}

