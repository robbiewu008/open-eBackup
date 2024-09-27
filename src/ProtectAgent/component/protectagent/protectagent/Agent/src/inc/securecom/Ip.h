/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Ip.h
 * @brief  The implemention about secure Ip
 * @version 1.0.0.0
 * @date 2021-05-15
 * @author wangguitao 00510599
 */
#ifndef __AGENT_SECURE_IP_H__
#define __AGENT_SECURE_IP_H__

#include "common/Defines.h"
#include <vector>

namespace SecureCom {
class AGENT_API CIP
{
public:
    static mp_int32 CheckHostLinkStatus(const std::vector<mp_string>& hostIpv4List,
    const std::vector<mp_string>& hostIpv6List, std::vector<mp_string>& srcIpv4List,
    std::vector<mp_string>& srcIpv6List);

private:
    static mp_bool CheckHostLinkStatus(const mp_string& strSrcIP, const std::vector<mp_string>& hostIpList);
};
}

#endif
