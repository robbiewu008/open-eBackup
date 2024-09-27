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
#ifndef VSS_TOOL_H
#define VSS_TOOL_H
#include "TargetEnv.h"
#include "VssBackup.h"
#include "VssRestore.h"

const int LEAST_VSSTOOL_INPUT_COUNT = 2;
const int BACKUP_INPUT_COUNT = 6;
const int DELETE_SNAPSHOT_INPUT_COUNT = 3;

const int VSSTOOL_SUCCESS = 0;
const int VSSTOOL_FAILED = -1;
#endif