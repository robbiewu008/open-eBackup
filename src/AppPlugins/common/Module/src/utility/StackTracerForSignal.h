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
#ifndef STACK_TRACER_H_
#define STACK_TRACER_H_

#include <string>
#include <csignal>

namespace Module {
class StackTracerForSignal {
public:
    StackTracerForSignal();
    ~StackTracerForSignal();

private:
    void InstallSignalHandler();
    void UninstallSignalHandler();

#ifndef __WINDOWS__
    static void SignalHandler(int signum, siginfo_t *siginfo, void *ucontext);
    static void SafeAbort();
    static void* btarray[sizeof(int32_t) * sizeof(int64_t)];
#endif
};

}

#endif  // STACK_TRACER_H_
