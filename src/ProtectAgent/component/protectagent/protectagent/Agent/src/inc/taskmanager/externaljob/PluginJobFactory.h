/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  Factory for Plugin job
 * @version 1.1.0
 * @date 2021-11-19
 * @author kWX884906
 */

#ifndef _PLUGIN_JOB_FACTORY_H_
#define _PLUGIN_JOB_FACTORY_H_

#include "taskmanager/externaljob/Job.h"

namespace AppProtect {
class PluginJobFactory {
public:
    static PluginJobFactory* GetInstance()
    {
        static PluginJobFactory g_instance;
        return &g_instance;
    }

    std::shared_ptr<Job> CreatePluginJob(const PluginJobData& data);
private:
    std::shared_ptr<Job> BuildMainJob(const PluginJobData& data);
    std::shared_ptr<Job> BuildGeneSubJob(const PluginJobData& data);
    std::shared_ptr<Job> BuildPreSubJob(const PluginJobData& data);
    std::shared_ptr<Job> BuildSubBusiJob(const PluginJobData& data);
    std::shared_ptr<Job> BuildSubPostJob(const PluginJobData& data);
private:
    PluginJobFactory()
    {}
    ~PluginJobFactory() = default;
    PluginJobFactory(const PluginJobFactory&) = delete;
    PluginJobFactory& operator=(const PluginJobFactory&) = delete;
    PluginJobFactory(PluginJobFactory&&) = delete;
    PluginJobFactory& operator=(PluginJobFactory&&) = delete;
};
}  // namespace AppProtect

#endif