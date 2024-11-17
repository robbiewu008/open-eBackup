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