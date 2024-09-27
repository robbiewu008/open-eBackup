/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file TestHost.h
 * @brief Test the link status of host
 * @version 1.0.0
 * @date 2021-07-13
 * @author liuliyue
 */

#ifndef _AGENTCLI_UNIXTIMESTAMP_H_
#define _AGENTCLI_UNIXTIMESTAMP_H_

#include "common/Types.h"

class UnixTimeStamp {
public:
    static mp_int32 Handle(const mp_string& timeString, const mp_string& transMode);

private:
    static mp_int32 UnixStampTranforDate(const mp_string& timeString);
    static mp_int32 DateTransforUnixStamp(const mp_string& timeString);
};
#endif