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
#include "taskmanager/externaljob/ExecutorBuilder.h"
#include "common/Types.h"

namespace AppProtect {
ExecutorBuilder& ExecutorBuilder::Next(const Executor& executor)
{
    m_executors.push_back(executor);
    return *this;
}

ExecutorBuilder& ExecutorBuilder::ConditonNext(const Executor& executor, const Executor& failedExecutor,
    const Executor& successExecutor)
{
    m_executors.push_back([executor, failedExecutor, successExecutor](int32_t result)->int32_t {
        auto ret = MP_SUCCESS;
        ret = executor(ret);
        if (ret == MP_SUCCESS) {
            return successExecutor(ret);
        } else {
            return failedExecutor(ret);
        }
    });
    return *this;
}

ExecutorBuilder& ExecutorBuilder::ConditonNext(const Executor& executor, const Executor& failedExecutor,
    const Executor& successExecutor, const Executor& redoExecutor)
{
    m_executors.push_back([executor, failedExecutor, successExecutor, redoExecutor](int32_t result)->int32_t {
        auto ret = MP_SUCCESS;
        ret = executor(ret);
        if (ret == MP_SUCCESS) {
            return successExecutor(ret);
        } else if (ret == MP_REDO) {
            return redoExecutor(ret);
        } else {
            return failedExecutor(ret);
        }
    });
    return *this;
}

Executor ExecutorBuilder::Build()
{
    std::vector<Executor> executors;
    executors.swap(m_executors);
    return [executors](int32_t result) {
        for (const auto& executor : executors) {
            result = executor(result);
        }
        return result;
    };
}
}