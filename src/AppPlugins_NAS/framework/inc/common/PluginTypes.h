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
#ifndef PLUGIN_TYPES_H
#define PLUGIN_TYPES_H

#include "define/Types.h"

// thrift return errorcode
static const int INNER_ERROR = 200;
static const int ERROR_COMMON_INVALID_PARAM = 0x4003291A;                    // 参数错误

// according to agent RPC API's name
enum class JobType {
    BACKUP = 1,
    RESTORE,
    ARCHIVE_RESTORE,
    INSTANT_RESTORE,
    INDEX,
    LIVEMOUNT,
    CANCELLIVEMOUNT,
    DELCOPY,
    CHECK_COPY,
    UNDEFINED_JOB_TYPE = 65536
};

// according to different operations from agent RPC API's name
enum class OperType {
    PRE = 1,
    GENERATE,
    EXECUTE,
    POST
};

#define NAS_PLUGIN_LOG(logLevel, ...) printf(__VA_ARGS__)

#endif  // _PLUGIN_NAS_TYPES_H_
