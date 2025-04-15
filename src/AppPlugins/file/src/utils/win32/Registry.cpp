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
#include "Registry.h"
#include <cwchar>

namespace PluginUtils {
namespace Win32 {

DWORD RegGetDword(HKEY hKey, const std::wstring &subKey, const std::wstring &value)
{
    DWORD data{};
    DWORD dataSize = sizeof(data);
    LONG retCode = ::RegGetValue(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_DWORD, nullptr, &data, &dataSize);
    if (retCode != ERROR_SUCCESS) {
        throw RegistryError{"Cannot read DWORD from registry.", retCode};
    }
    return data;
}

ULONGLONG RegGetQword(HKEY hKey, const std::wstring &subKey, const std::wstring &value)
{
    ULONGLONG data{};
    DWORD dataSize = sizeof(data);
    LONG retCode = ::RegGetValue(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_QWORD, nullptr, &data, &dataSize);
    if (retCode != ERROR_SUCCESS) {
        throw RegistryError{"Cannot read QWORD from registry.", retCode};
    }
    return data;
}

std::wstring RegGetString(HKEY hKey, const std::wstring &subKey, const std::wstring &value)
{
    DWORD dataSize{};
    LONG retCode = ::RegGetValue(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &dataSize);
    if (retCode != ERROR_SUCCESS) {
        throw RegistryError{"Cannot read string from registry.", retCode};
    }

    std::wstring data;
    data.resize(dataSize / sizeof(wchar_t));

    retCode = ::RegGetValue(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, &data[0], &dataSize);
    if (retCode != ERROR_SUCCESS) {
        throw RegistryError{"Cannot read string from registry.", retCode};
    }

    DWORD wstringDataSize = dataSize / sizeof(wchar_t);
    wstringDataSize--;
    data.resize(wstringDataSize);

    return data;
}

std::vector<std::wstring> RegGetMultiString(HKEY hKey, const std::wstring &subKey, const std::wstring &value)
{
    DWORD dataSize{};
    LONG retCode = ::RegGetValue(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_MULTI_SZ, nullptr, nullptr, &dataSize);
    if (retCode != ERROR_SUCCESS) {
        throw RegistryError{"Cannot read string from registry.", retCode};
    }
    if (dataSize == 0) {
        return {};
    }

    std::vector<wchar_t> data;
    data.resize(dataSize / sizeof(wchar_t));

    retCode = ::RegGetValue(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_MULTI_SZ, nullptr, &data[0], &dataSize);
    if (retCode != ERROR_SUCCESS) {
        throw RegistryError{"Cannot read string from registry.", retCode};
    }

    DWORD wstringDataSize = dataSize / sizeof(wchar_t);
    data.resize(wstringDataSize);

    std::vector<std::wstring> result;
    wchar_t *currStringPtr = data.data();
    while (*currStringPtr != L'\0') {
        const size_t currStringLength = ::wcslen(currStringPtr);
        result.push_back(std::wstring{currStringPtr, currStringPtr + currStringLength});
        currStringPtr += currStringLength + 1;
    }

    return result;
}

}  // namespace Win32
}  // namespace PluginUtils
