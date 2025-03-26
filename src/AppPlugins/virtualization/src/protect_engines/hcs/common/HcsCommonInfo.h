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
#ifndef HCS_COMMON_INFO_H
#define HCS_COMMON_INFO_H

#include <string>
#include <set>
#include <map>
#include <vector>
#include "HcsMacros.h"
#include "common/Macros.h"
#include "common/Constants.h"
#include "common/Structs.h"
#include <common/JsonHelper.h>
#include <common/CleanMemPwd.h>

USING_NAMESPACE_VIRT_PLUGIN;

HCS_PLUGIN_NAMESPACE_BEGIN

enum class ErrorSort {
    NONE = 0,
    ERROR_PROJECT,   // 获取project 错误
    ERROR_VDC_MISS,  // VDC管理员不存在错误
};

/**
 *  卷扩展信息
 */
struct HcsVolExtendInfo {
public:
    std::string m_architecture;
    std::string m_attr;
    std::string m_lunWWN;
    std::string m_mode;
    std::string m_size;
    std::string m_encrypted;
    std::string m_cmkId;
    std::string m_cipher;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_architecture, architecture)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attr, attr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lunWWN, lunWWN)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_mode, mode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_encrypted, systemEncrypted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cmkId, systemCmkId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cipher, cipher)
    END_SERIAL_MEMEBER
};

struct HcsProtectExtendInfo {
    std::string m_projectId;
    std::string m_isWorkSpace;
    std::string m_status;
    std::string m_host;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, cloud_server_project_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isWorkSpace, isWorkspace)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_host, host)
    END_SERIAL_MEMEBER
};

struct HcsGetToken {
    std::string m_token;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_token, tokenStr)
    END_SERIAL_MEMEBER
};

HCS_PLUGIN_NAMESPACE_END

#endif