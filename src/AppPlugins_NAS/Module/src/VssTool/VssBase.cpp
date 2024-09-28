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
#include "VssBase.h"

#include <fstream>


namespace {
const DWORD MAX_WAIT_MILLISECONDS = 60000;
}

VssBase::VssBase()
{}

VssBase::~VssBase()
{}

DWORD VssBase::LoadFromFile(const std::string& strFile, std::wstring& strContent)
{
    DWORD dwRet = 0;

    HANDLE hFile = INVALID_HANDLE_VALUE;
    do {
        hFile = CreateFile(strFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, \
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            dwRet = GetLastError();
            break;
        }

        DWORD dwSize = GetFileSize(hFile, NULL);
        if (dwSize <= 0) {
            break;
        }

        if ((dwSize % sizeof(WCHAR)) != 0) {
            break;
        }
        strContent.resize(dwSize / sizeof(WCHAR), L'\0');
        DWORD dwRead = 0;
        if (!ReadFile(hFile, (LPVOID)strContent.c_str(), dwSize, &dwRead, NULL)) {
            dwRet = GetLastError();
            break;
        }
    } while (0);

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
    return dwRet;
}

bool VssBase::WaitAndCheckForAsyncOperation(IVssAsync *pAsync)
{
    if (pAsync == nullptr) {
        return false;
    }

    HRESULT hr = pAsync->Wait(MAX_WAIT_MILLISECONDS);
    CHECK_HR_RETURN(hr, "WaitAndCheckForAsyncOperation pAsync->Wait", false);

    /* Check the result of the asynchronous operation */
    HRESULT hrReturned = S_OK;
    hr = pAsync->QueryStatus(&hrReturned, nullptr);
    CHECK_HR_RETURN(hr, "WaitAndCheckForAsyncOperation pAsync->QueryStatus", false);

    /* Check if the async operation succeeded... */
    if (hrReturned != VSS_S_ASYNC_FINISHED) {
        return false;
    }
    return true;
}

std::wstring VssBase::Utf8ToUtf16(const std::string &str)
{
    using ConvertTypeX = std::codecvt_utf8_utf16<wchar_t>;
    std::wstring_convert<ConvertTypeX> converterX;
    std::wstring wstr = converterX.from_bytes(str);
    return wstr;
}

std::string VssBase::Utf16ToUtf8(const std::wstring &wstr)
{
    using ConvertTypeX = std::codecvt_utf8_utf16<wchar_t>;
    std::wstring_convert<ConvertTypeX> converterX;
    return converterX.to_bytes(wstr);
}

std::wstring VssBase::VssID2WStr(const VSS_ID &vssID)
{
    LPOLESTR wVssIDBuf = nullptr;
    CAutoComPointer ptrAutoCleanUp(wVssIDBuf);
    HRESULT hr = ::StringFromIID(vssID, &wVssIDBuf);
    if (FAILED(hr)) {
        return L"";
    }
    std::wstring wVssIDStr(wVssIDBuf);
    return wVssIDStr;
}

VSS_ID VssBase::VssIDfromWStr(const std::wstring &vssIDWstr)
{
    VSS_ID vssID;
    HRESULT hr = ::IIDFromString(vssIDWstr.c_str(), &vssID);
    if (FAILED(hr)) {
        return vssID;
    }
    return vssID;
}
