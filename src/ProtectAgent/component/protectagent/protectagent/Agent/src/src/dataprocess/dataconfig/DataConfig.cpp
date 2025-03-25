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
#include "sstream"
#include "common/Types.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "dataprocess/dataconfig/DataConfig.h"

DataConfig::DataConfig()
{}

DataConfig::~DataConfig()
{}

mp_int32 DataConfig::GetID(mp_int32 &id)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 temp_id = 0;
    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_DP, CFG_DP_KEY, temp_id);
    if (iRet != MP_SUCCESS) {
        return (iRet);
    }
    id = temp_id;

    return (iRet);
}

mp_int32 DataConfig::GetState(mp_string &state)
{
    mp_int32 iRet = MP_SUCCESS;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_DP, CFG_DP_VALUE, state);
    if (iRet != MP_SUCCESS) {
        return (iRet);
    }

    return (iRet);
}

mp_bool DataConfig::SetID(mp_int32 id)
{
    mp_bool iRet = MP_TRUE;
    mp_string strID;
    std::stringstream ss;
    ss << id;
    strID = ss.str();

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_DP, CFG_DP_KEY, strID);
    if (iRet != MP_SUCCESS) {
        return MP_FALSE;
    }

    return MP_TRUE;
}

mp_bool DataConfig::SetState(const mp_string& state)
{
    mp_bool iRet = MP_TRUE;

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_DP, CFG_DP_VALUE, state);
    if (iRet != MP_SUCCESS) {
        return MP_FALSE;
    }

    return MP_TRUE;
}
