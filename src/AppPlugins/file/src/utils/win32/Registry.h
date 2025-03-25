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
#ifndef WIN32_REGISTRY_H
#define WIN32_REGISTRY_H

#include <WinSock2.h>
#include <windows.h>
#include <winreg.h>
#include <string>
#include <vector>
#include <stdexcept>
#include "define/Defines.h"

namespace PluginUtils {
namespace Win32 {

class AGENT_API RegistryError : public std::runtime_error {
public:
    RegistryError(const char *message, LONG errorCode) : std::runtime_error{message}, m_errorCode(errorCode)
    {}

    LONG ErrorCode() const noexcept
    {
        return m_errorCode;
    }

private:
    LONG m_errorCode;
};

AGENT_API DWORD RegGetDword(HKEY hKey, const std::wstring &subKey, const std::wstring &value);

AGENT_API ULONGLONG RegGetQword(HKEY hKey, const std::wstring &subKey, const std::wstring &value);

AGENT_API std::wstring RegGetString(HKEY hKey, const std::wstring &subKey, const std::wstring &value);

AGENT_API std::vector<std::wstring> RegGetMultiString(HKEY hKey, const std::wstring &subKey, const std::wstring &value);

}  // namespace Win32
}  // namespace PluginUtils

#endif  // WIN32_REGISTRY_H