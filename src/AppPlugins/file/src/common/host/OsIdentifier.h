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
#ifndef HOST_OS_FLAG_IDENTIFIER_H
#define HOST_OS_FLAG_IDENTIFIER_H

#include "define/Defines.h"

namespace FilePlugin {

const std::string OS_IDENTIFIER_SYSTEM_NAME_RECORD_FILE_NAME = "system.name";
const std::string OS_IDENTIFIER_SYSTEM_VERSION_RECORD_FILE_NAME = "system.version";

const std::string OS_IDENTIFIER_SYSTEM_NAME_LINUX = "Linux";
const std::string OS_IDENTIFIER_SYSTEM_NAME_SOLARIS = "SunOS";

class OsIdentifier {
public:
    // common utils
    AGENT_API static std::string GetSystemName();

    AGENT_API static std::string GetSystemVersion();

    AGENT_API static bool CreateSystemNameRecordFile(const std::string& dirPath);

    AGENT_API static bool CreateSystemVersionRecordFile(const std::string& dirPath);

    AGENT_API static std::string ReadSystemName(const std::string& dirPath);

    AGENT_API static std::string ReadSystemVersion(const std::string& dirPath);

#ifdef SOLARIS
    // check system compatible issue, return true is current os is solaris and compatible with the version in record
    AGENT_API static bool CheckSolarisVersionCompatible(const std::string& dirPath);
#endif
};

}

#endif