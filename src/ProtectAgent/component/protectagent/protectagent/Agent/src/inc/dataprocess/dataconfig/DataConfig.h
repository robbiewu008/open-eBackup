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
#ifndef __DATA_CONFIG_H__
#define __DATA_CONFIG_H__

#include "tinyxml2.h"
#include "common/Types.h"
#include "common/TimeOut.h"
#include "common/ConfigXmlParse.h"

static const mp_string CFG_DP = "dataprocess";
static const mp_string CFG_DP_KEY = "id";
static const mp_string CFG_DP_VALUE = "state";

class DataConfig {
public:
    DataConfig();
    ~DataConfig();
    mp_int32 GetID(mp_int32 &id);
    mp_int32 GetState(mp_string &state);

    mp_bool SetID(mp_int32 id);
    mp_bool SetState(const mp_string& state);
};

#endif
