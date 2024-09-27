/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file ScopeExit.h
 * @author t00302329
 * @brief 提供SCOPE_EXIT机制，在作用域退出时自动执行相关的清理动作
 * @version 0.1
 * @date 2021-01-05
 *
 */
#ifndef __SCOPE_EXIT_H__
#define __SCOPE_EXIT_H__

#include <algorithm>

#define SCOPE_EXIT_CAT2(x, y) x##y
#define SCOPE_EXIT_CAT(x, y) SCOPE_EXIT_CAT2(x, y)
#define SCOPE_EXIT const auto SCOPE_EXIT_CAT(scopeExit_, __LINE__) = ScopeExitCreator() += [ & ]

template<typename F>
class ScopeExit {
public:
    explicit ScopeExit(F &&fn) : m_fn(fn) {}

    ~ScopeExit()
    {
        m_fn();
    }

    ScopeExit(ScopeExit &&other) : m_fn(std::move(other.m_fn)) {}

private:
    ScopeExit(const ScopeExit&);
    ScopeExit& operator=(const ScopeExit&);

private:
    F m_fn;
};

struct ScopeExitCreator {
    template<typename F>
    ScopeExit<F> operator+=(F &&fn)
    {
        return ScopeExit<F>(std::move(fn));
    }
};

#endif