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
#ifndef HCS_CONSTANTS_H
#define HCS_CONSTANTS_H

#include "HcsMacros.h"

HCS_PLUGIN_NAMESPACE_BEGIN
const std::string SERVER_STATUS_ACTIVE = "ACTIVE";
const std::string SERVER_STATUS_SHUTOFF = "SHUTOFF";
const std::string VOLUME_STATUS_AVAILABLE = "available";
const std::string VOLUME_STATUS_INUSE = "in-use";
const int PART_MISS = 1; // VDC管理员部分删除情况
HCS_PLUGIN_NAMESPACE_END

#endif