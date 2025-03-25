/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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