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
#ifndef _NAS_PLUGIN_ERROR_CODE_H
#define _NAS_PLUGIN_ERROR_CODE_H

#include "PluginNasTypes.h"

// *********************框架内部错误码************************
static const int NO_AGNET_AVAILABLE = 256; // 对象不存在

static const uint32_t E_RESOURCE_DIR_NOT_EXIST = 0x5E02503F;
static const uint32_t E_RESOURCE_NO_ACCESS_RIGHT = 0x5E025040;
static const uint32_t E_RESOURCE_TYPE_INVALID = 0x5E025042;
static const uint32_t E_RESOURCE_AGENT_LVM_NOT_SUPPORT = 0x5E114514;
static const uint32_t E_BACKUP_FAILED_NOSPACE_ERROR = 1577209929;
static const uint32_t E_BACKUP_BACKUP_SECONDARY_SERVER_NOT_REACHABLE = 1577209931;
static const uint32_t E_BACKUP_FAILED_DETAIL_ERROR = 1577209859;
static const uint32_t E_JOB_SERVICE_SUB_JOB_CNT_MAX = 1593987332;


#endif