/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file builder.h
 * @brief Implement for builder
 * @version 1.1.0
 * @date 2022-1-13
 * @author caomin 511255
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