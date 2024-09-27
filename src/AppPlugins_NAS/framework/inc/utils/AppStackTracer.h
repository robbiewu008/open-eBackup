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
#ifndef APP_STACK_TRACER_H
#define APP_STACK_TRACER_H

#include <string>
#include <csignal>

class AppStackTracer {
public:
    explicit AppStackTracer(std::string path);
    void Init();
    ~AppStackTracer();

private:
    void InstallHandler();
    void UninstallHandler();

#ifndef __WINDOWS__
    static void SignalHandler(int signum, siginfo_t *siginfo, void *ucontext);
    static void SafeAbort();
#endif
};

#endif  // APP_STACK_TRACER_H