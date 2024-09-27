/**
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*
* @file RemoteHostFilter.h
* @brief Filter logic port in job param
* @version 0.1
* @date 2023-07-05
* @author wangyunlong w30045225
*/
#ifndef FILTER_REMOTE_HOST_FILTER_H
#define FILTER_REMOTE_HOST_FILTER_H

#include "common/Types.h"
#include "taskmanager/externaljob/Job.h"

using namespace AppProtect;

namespace JobParamKey {
    const mp_string APP_INFO = "appInfo";
    const mp_string EXTEND_INFO = "extendInfo";
    const mp_string LAN_TYPE = "lanType";
    const mp_string PORT_TYPE = "portType";
    const mp_string COPIES = "copies";
    const mp_string REPOSITORIES = "repositories";
    const mp_string REMOTE_HOST = "remoteHost";
}

class RemoteHostFilter {
public:
    virtual ~RemoteHostFilter() {}
    virtual mp_int32 DoFilter(PluginJobData& jobData, mp_bool isInner) = 0;
};
#endif
