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
#ifndef AGENT_APP_VERSION
#define AGENT_APP_VERSION

#include "common/Types.h"

static const mp_string AGENT_PACKAGE_VERSION = "8.0.0";
static const mp_string AGENT_HDRS_VERSION = "8.0.0";
static const mp_string AGENT_VERSION = "8.0.0";
static const mp_wstring RD_PROVIDER_VERSION = L"1.0.0.0";
static const mp_string COMPILE_TIME = "compile";
static const mp_int64 AGENT_UPDATE_VERSION = 1617181350;

// 变更规则"1.0.0" :
// 1.每次V或者R版本号的变更，第一个数字加1.
// 2.每次C版本号的变更，第二个数字加1.
// 3.每次交互版本时，第三个数字加1.
static const mp_string AGENT_BUILD_NUM = "1.9.0";

inline void AgentVersion()
{
    printf("Copyright 2013-2020 Huawei Technologies Co., Ltd.\n");
    printf("Version     : %s\n", AGENT_VERSION.c_str());
    printf("Build Number: %s\n", AGENT_BUILD_NUM.c_str());
}

#endif  // _AGENT_APP_VERSION_
