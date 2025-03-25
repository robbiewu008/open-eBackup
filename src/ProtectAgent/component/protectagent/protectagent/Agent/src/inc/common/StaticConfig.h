/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file StaticConfig.h
 * @brief Contains some config which will not changge when process running.
 * @version 1.0.0
 * @date 2024-09-22
 * @author h00668904
 */
#ifndef AGENT_STATIC_CONFIG_H
#define AGENT_STATIC_CONFIG_H

#include "common/Types.h"
#include "common/Defines.h"

namespace StaticConfig {

// host install type
const mp_int32 AGENT_INSTALL_TYPE_EXTERNAL = 0;
const mp_int32 AGENT_INSTALL_TYPE_INTERNAL = 1;

// check agent install type is inner or outer
AGENT_API bool IsInnerAgent();

AGENT_API bool NeedClusterEsn();

AGENT_API bool GetAgentIp(mp_string& agentIp);

// check agent install type and deploy type
AGENT_API bool IsInnerAgentMainDeploy();

AGENT_API bool GetInnerAgentNodeIps(std::vector<std::string>& ips);
};

#endif