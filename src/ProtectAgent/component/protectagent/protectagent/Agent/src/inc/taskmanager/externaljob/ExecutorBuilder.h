#ifndef EXECUTOR_BUILDER_H_
#define EXECUTOR_BUILDER_H_
#include <functional>
#include <vector>
#include <stdint.h>

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