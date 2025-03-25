/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * @file TestHost.h
 * @brief Test the link status of host
 * @version 1.0.0
 * @date 2020-04-12
 * @author wangguitao 00510599
 */
#ifndef _AGENTCLI_TESTHOST_H_
#define _AGENTCLI_TESTHOST_H_

#include "common/Types.h"

class TestHost {
public:
    static mp_int32 Handle(const mp_string& strHostIp, const mp_string& strHostPort,
        const mp_string& strTimeout, const mp_string& strSrcHost);

private:
    static mp_int32 InnerAgentHandle(const mp_string& strHostIp, const mp_string& strHostPort,
        uint32_t timeout);
    static mp_int32 CheckConnect(const mp_string& strHostIp, const mp_string& strHostPort,
        uint32_t timeout, const mp_string& strSrcHost);
};

#endif
