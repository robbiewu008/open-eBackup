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
#ifndef __CONST_H__
#define __CONST_H__
#include "common/Types.h"

const mp_string FUSION_STORAGE_TYPE = "Type";
const mp_string FUSION_STORAGE_IP = "FusionStorageIP";
const mp_string FUSION_STORAGE_PORT = "FusionStoragePort";
const mp_string FUSION_STORAGE_POOL = "PoolId";
const mp_string FUSION_STORAGE_USERNAME = "FusionStorageUsername";
const mp_string FUSION_STORAGE_PASSWORD = "FusionStoragePassword";

enum CLONE_SOURCE { VOLUME = 0, SNAPSHOT = 1};

constexpr mp_int32 FUSION_STORAGE_SNAPSHOP_STATUS_NORMAL = 0;
constexpr mp_int32 FUSION_STORAGE_SNAPSHOP_STATUS_REVERTING = 16;

#endif