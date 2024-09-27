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
#ifndef FS_SCANNER_WIN32_PATH_UTILS
#define FS_SCANNER_WIN32_PATH_UTILS

#include <iostream>
#include <string>
#include "define/Defines.h"

namespace Module {

namespace Win32PathUtil {

    AGENT_API std::string LowerCase(const std::string& path);
    AGENT_API std::string PosixToWin32(const std::string &path, bool forceLowerCase = false);
    AGENT_API std::string Win32ToPosix(const std::string &path, bool forceLowerCase = false);
    AGENT_API std::string Win32PathRecoverPrefix(const std::string& path, const std::string& prefix, const char driver);
    AGENT_API std::string Win32PathRecoverPrefix(const std::string& path, const std::string& prefix);
    AGENT_API bool Win32PathEquals(const std::string& path1, const std::string& path2);
    AGENT_API std::string ConcatWin32Path(const std::string& path1, const std::string& path2);
    AGENT_API std::string GetParentDir(const std::string& path);
    AGENT_API std::string GetFileName(const std::string& path);

}

}

#endif