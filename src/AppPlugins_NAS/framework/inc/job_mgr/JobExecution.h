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
#ifndef NEWFRAMEWORKTEST_JOBEXECUTION_H
#define NEWFRAMEWORKTEST_JOBEXECUTION_H

#include <map>
#include <functional>
#include "PluginTypes.h"
#include "BasicJob.h"

namespace common {
namespace jobmanager {
class JobExecution {
public:
    JobExecution();
    ~JobExecution();
    int ExecuteJob(AppProtect::ActionResult& result, const std::shared_ptr<BasicJob>& job, const std::string& jobId,
        OperType type);

private:
    int ExecPreJob(std::shared_ptr<BasicJob> job, const std::string& jobId);
    int ExecGenSubJob(std::shared_ptr<BasicJob> job, const std::string& jobId);
    int ExecSubJob(std::shared_ptr<BasicJob> job, const std::string& jobId);
    int ExecPostJob(std::shared_ptr<BasicJob> job, const std::string& jobId);
    int InitAsyncThread(std::shared_ptr<BasicJob> job, std::shared_ptr<std::thread> thread, const std::string& jobId);
    using CallFunc = std::function<int(std::shared_ptr<BasicJob>, const std::string& jobId)>;
    std::map<OperType, CallFunc> m_funcMap {};
};
}
}

#endif // NEWFRAMEWORKTEST_JOBEXECUTION_H
