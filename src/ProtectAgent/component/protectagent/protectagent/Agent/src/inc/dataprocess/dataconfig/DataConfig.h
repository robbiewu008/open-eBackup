/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file DataConfig.h
 * @brief  Contains function declarations for TaskStepOracleNativeMedia
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
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
