/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file CollectLog.h
 * @brief  The implemention about CollectLog.h
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef _AGENTCLI_COLLECTLOG_H_
#define _AGENTCLI_COLLECTLOG_H_

#include "common/Types.h"
#define COLLECTLOG_HINT \
    "This operation allows you to collect debugging logs of Agent running on this host. \
The collected logs can be used to analyze the operating status of Agent."

class CollectLog {
public:
    static mp_int32 Handle();
};

#endif
