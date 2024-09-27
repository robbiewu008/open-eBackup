/**
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*
* @file RemoteHostFilterFactory.cpp
* @brief The factory class of RemoteHostFilter
* @version 0.1
* @date 2023-07-06
* @author wangyunlong w30045225
*/

#include "taskmanager/filter/RemoteHostFilterFactory.h"
#include "taskmanager/filter/DefaultRemoteHostFilter.h"

RemoteHostFilterFactory::RemoteHostFilterFactory()
{}

RemoteHostFilterFactory::~RemoteHostFilterFactory()
{}

std::shared_ptr<RemoteHostFilter> RemoteHostFilterFactory::CreateFilter(FilterType type)
{
    if (type == PORT_TYPE_FILTER) {
        return std::make_shared<DefaultRemoteHostFilter>();
    }
    return nullptr;
}

FilterAction RemoteHostFilterFactory::CreateFilterAction(FilterType type)
{
    FilterAction filterAction = [type](PluginJobData& jobData, mp_bool isInner) -> mp_int32 {
        return CreateFilter(type)->DoFilter(jobData, isInner);
    };
    return filterAction;
}