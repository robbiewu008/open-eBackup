/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file DataConfig.cpp
 * @brief  Contains function declarations DataConfig Operations
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
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
