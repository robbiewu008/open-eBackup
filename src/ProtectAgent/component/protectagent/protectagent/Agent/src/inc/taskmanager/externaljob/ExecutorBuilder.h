/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file builder.h
 * @brief Implement for builder
 * @version 1.1.0
 * @date 2022-1-13
 * @author caomin 511255
 */
#ifndef EXECUTOR_BUILDER_H_
#define EXECUTOR_BUILDER_H_
#include <functional>
#include <vector>

namespace AppProtect {
using Executor = std::function<int32_t(int32_t)>;
class ExecutorBuilder {
public:
    ExecutorBuilder& Next(const Executor& Executor);
    ExecutorBuilder& ConditonNext(const Executor& executor,
        const Executor& failedExecutor, const Executor& successExecutor);
    ExecutorBuilder& ConditonNext(const Executor& executor,
        const Executor& failedExecutor, const Executor& successExecutor, const Executor& redoExecutor);
    Executor Build();
private:
    std::vector<Executor> m_executors;
    std::vector<Executor> m_makeExecutors;
};
}

#endif