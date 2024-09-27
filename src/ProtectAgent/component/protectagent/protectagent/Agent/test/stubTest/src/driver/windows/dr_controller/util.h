#pragma once
#include "header.h"



DWORD CreateDirectories(const CString& strPath);
CString GetModulePath();
BOOL RegDeleteKeyRecursive(HKEY hKeyRoot, const CString& strKey);