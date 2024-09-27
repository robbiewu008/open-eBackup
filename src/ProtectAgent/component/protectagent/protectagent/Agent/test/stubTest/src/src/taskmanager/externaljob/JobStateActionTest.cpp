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
#include "taskmanager/externaljob/JobStateActionTest.h"
#include "taskmanager/externaljob/JobStateAction.h"
#include "taskmanager/externaljob/PluginMainJob.h"

#include <memory>

namespace {
mp_void LogTest()
{}

#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub.set(ADDR(CLogger, Log), LogTest);                                                                         \
    } while (0)
}  // namespace

namespace AppProtect {

mp_int32 ActionSuccessTest()
{
    return MP_SUCCESS;
}

mp_int32 ActionFailedTest()
{
    return MP_FAILED;
}

/*
 * 用例名称：主任务初始化成功
 * 前置条件：生成的任务进行初始化
 * check点：1、Mock log 2、主任务初始化成功
 */
TEST_F(JobStateActionTest, NullStateTest)
{
    DoLogTest();
    auto testAction = std::make_shared<JobStateAction<MainJobState>>(
        "test", MainJobState::INITIALIZING, MainJobState::FAILED, MainJobState::COMPLETE);
    MainJobState state = testAction.get()->Transition();
    EXPECT_EQ(state, MainJobState::FAILED);
}

/*
 * 用例名称：JobStateAction设置Enter、Transition、Exit任意函数，执行成功
 * 前置条件：设置OnEnter执行函数，执行函数返回成功
 * check点：1、Mock log 2、JobStateAction执行成功
 */
TEST_F(JobStateActionTest, SuccessStateTest)
{
    DoLogTest();
    auto testAction = std::make_shared<JobStateAction<MainJobState>>(
        "test", MainJobState::INITIALIZING, MainJobState::FAILED, MainJobState::COMPLETE);
    testAction.get()->OnEnter = std::bind(&ActionSuccessTest);
    MainJobState state = testAction.get()->Transition();
    EXPECT_EQ(state, MainJobState::COMPLETE);
}

/*
 * 用例名称：JobStateAction设置Enter、Transition、Exit任意函数，执行失败
 * 前置条件：设置OnEnter执行函数，执行函数返回失败
 * check点：1、Mock log 2、JobStateAction执行失败
 */
TEST_F(JobStateActionTest, FailStateTest)
{
    DoLogTest();
    auto testAction = std::make_shared<JobStateAction<MainJobState>>(
        "test", MainJobState::INITIALIZING, MainJobState::FAILED, MainJobState::COMPLETE);
    testAction.get()->OnEnter = std::bind(&ActionFailedTest);
    MainJobState state = testAction.get()->Transition();
    EXPECT_EQ(state, MainJobState::FAILED);
}
}  // namespace AppProtect
