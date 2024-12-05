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
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <comdef.h>
#include <wtypes.h>
#include <atlbase.h>
#include <codecvt>
#include "common/JsonHelper.h"

#define CHECK_HR_RETURN(HR, FUNC, RET) \
    do { \
        _com_error err(HR); \
        if ((HR) != S_OK) { \
            ::printf("HRESULT Return FAILED, Function: " ## FUNC ## ", Error: %s", err.ErrorMessage()); \
            ERRLOG("HRESULT Return FAILED, Function: " ## FUNC ## ", Error: %s", err.ErrorMessage()); \
            return RET; \
        } \
    } \
    while (0)

#define CHECK_BOOL_RETURN(BOOLVALUE, FUNC, RET) \
    do { \
        if ((!(BOOLVALUE))) { \
            ::printf("Boolean Return False, Function: " ## FUNC ## ""); \
            ERRLOG("Boolean Return False, Function: " ## FUNC ## ""); \
            return RET; \
        } \
    } \
    while (0)

class VssBase {
public:
    VssBase();
    ~VssBase();
    bool WaitAndCheckForAsyncOperation(IVssAsync *pAsync);
    std::wstring Utf8ToUtf16(const std::string &str);
    std::string Utf16ToUtf8(const std::wstring &wstr);
    DWORD LoadFromFile(const std::string& strFile, std::wstring& strContent);
    std::wstring VssID2WStr(const VSS_ID &vssID);
    VSS_ID VssIDfromWStr(const std::wstring &vssIDWstr);
};

class CAutoComPointer {
public:
    explicit CAutoComPointer(LPVOID ptr) : m_ptr(ptr){};
    ~CAutoComPointer()
    {
        ::CoTaskMemFree(m_ptr);
    }

private:
    LPVOID m_ptr;
};