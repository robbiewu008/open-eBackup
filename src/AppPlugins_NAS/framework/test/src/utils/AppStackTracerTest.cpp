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
#include "gtest/gtest.h"
#include "AppStackTracer.h"
#include "stub.h"

class AppStackTracerTest : public testing::Test {
public:
    Stub stub;
};

void AbortStub()
{
}

TEST_F(AppStackTracerTest, SignalHandlerTest)
{
    AppStackTracer work("/tmp");
    work.Init();
    int signum = SIGINT;
    char siginfo[20] = "123456789";
    char ucontext[20] = "123456789";
    work.SignalHandler(signum, (siginfo_t*)siginfo, ucontext);
}

TEST_F(AppStackTracerTest, SignalHandlerTest2)
{
    AppStackTracer work("/tmp");
    work.Init();
    int signum = SIGSEGV;
    char siginfo[20] = "123456789";
    char ucontext[20] = "123456789";
    stub.set(std::abort, AbortStub);
    work.SignalHandler(signum, (siginfo_t*)siginfo, ucontext);
}
