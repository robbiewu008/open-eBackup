#include "stdafx.h"
#include "util.h"



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