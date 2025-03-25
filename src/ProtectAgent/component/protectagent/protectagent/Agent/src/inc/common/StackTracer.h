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
#ifndef _AGENT_STACK_TRACER_H_
#define _AGENT_STACK_TRACER_H_

#include <string>
#include <vector>
#include "common/Defines.h"

class StackTracer {
public:
    StackTracer();
    ~StackTracer();

private:
    void InstallSignalHandler();
    void UninstallSignalHandler();
    
    static void OutputMaps(std::vector<mp_string>& stackStream);
    static int SignalHandler(int signum, void *siginfo, void *ucontext);
    static void* btarray[sizeof(int32_t) * sizeof(int64_t)];
    static void WriteStackContent(const mp_string& stackFile, const std::vector<mp_string>& vecInput);
};

#endif  // STACK_TRACER_H_
