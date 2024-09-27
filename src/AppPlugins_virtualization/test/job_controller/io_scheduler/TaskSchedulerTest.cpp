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
#include "stub.h"
#include "common/Macros.h"
#include "job_controller/io_scheduler/TaskScheduler.h"
#include "config_reader/ConfigIniReader.h"

namespace IOTaskSchedulerTest {
USING_NAMESPACE_VIRT_PLUGIN;
class IOTaskTest;
using FuncPtr = int32_t (*)(IOTaskTest*);

class TaskScheduleTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
public:
    Stub stub;
};

class IOTaskTest : public BlockTask {
public:
    IOTaskTest() {}
    ~IOTaskTest() {}

    int32_t Exec() override
    {
        return SUCCESS;
    }
};

int32_t TaskExecFailed()
{
    return FAILED;
}

void StubSleep()
{
    return ;
}

int stub_get_pool_thread()
{
    return 1;
}

void TaskScheduleTest::SetUp()
{
    stub.set(sleep, StubSleep);
    stub.set(ADDR(Module::ConfigReader, getUint), stub_get_pool_thread);
}

void TaskScheduleTest::TearDown()
{
    stub.reset(ADDR(Module::ConfigReader, getUint));
}

void waitTashFinish(TaskScheduler &ts, int taskCount)
{
    std::shared_ptr<BlockTask> task = nullptr;
    while (taskCount > 0) {
        ts.Get(task);
        taskCount -= 1;
    }
}

TEST_F(TaskScheduleTest, Put_Failed)
{
    TaskPool* taskPool = TaskPool::GetInstance();
    TaskScheduler taskScheduler(*taskPool);
    std::shared_ptr<BlockTask> task = nullptr;

    bool retValue = taskScheduler.Put(task);
    EXPECT_EQ(retValue, false);
}

TEST_F(TaskScheduleTest, Put_SuccessHaveTime)
{
    TaskPool* taskPool = TaskPool::GetInstance();
    TaskScheduler taskScheduler(*taskPool);
    std::shared_ptr<BlockTask> task = std::make_shared<IOTaskTest>();

    bool retValue = taskScheduler.Put(task, true, 100);
    waitTashFinish(taskScheduler, 1);
    EXPECT_EQ(retValue, true);
}

TEST_F(TaskScheduleTest, Put_Success)
{
    TaskPool* taskPool = TaskPool::GetInstance();
    TaskScheduler taskScheduler(*taskPool);
    std::shared_ptr<BlockTask> task = std::make_shared<IOTaskTest>();

    bool retValue = taskScheduler.Put(task);
    waitTashFinish(taskScheduler, 1);
    EXPECT_EQ(retValue, true);
}

TEST_F(TaskScheduleTest, Get_Success)
{
    TaskPool* taskPool = TaskPool::GetInstance();
    TaskScheduler taskScheduler(*taskPool);
    std::shared_ptr<BlockTask> task = std::make_shared<IOTaskTest>();
    taskScheduler.Put(task);

    bool retValue = taskScheduler.Get(task);
    EXPECT_EQ(retValue, true);
}

TEST_F(TaskScheduleTest, Get_EmptyFalse)
{
    TaskPool* taskPool = TaskPool::GetInstance();
    TaskScheduler taskScheduler(*taskPool);
    std::shared_ptr<BlockTask> task;

    bool retValue = taskScheduler.Get(task, 5);
    EXPECT_EQ(retValue, false);
}

TEST_F(TaskScheduleTest, TaskWrapperExec_Failed)
{
    TaskPool* taskPool = TaskPool::GetInstance();
    TaskScheduler taskScheduler(*taskPool);
    std::shared_ptr<BlockTask> task = std::make_shared<IOTaskTest>();
    taskScheduler.Put(task);
    FuncPtr fp = (FuncPtr)(&IOTaskTest::Exec);
    stub.set(fp, TaskExecFailed);
    int32_t retValue = task->Exec();
    waitTashFinish(taskScheduler, 1);
    EXPECT_EQ(retValue, FAILED);
    stub.reset(fp);
}

TEST_F(TaskScheduleTest, TaskWrapperExec_Success)
{
    TaskPool* taskPool = TaskPool::GetInstance();
    TaskScheduler taskScheduler(*taskPool);
    std::shared_ptr<BlockTask> task = std::make_shared<IOTaskTest>();
    taskScheduler.Put(task);
    int32_t retValue = task->Exec();
    waitTashFinish(taskScheduler, 1);
    EXPECT_EQ(retValue, SUCCESS);
}



class IOTaskTestFailed : public BlockTask {
public:
    IOTaskTestFailed() {}
    ~IOTaskTestFailed() {}

    int32_t Exec() override
    {
        return FAILED;
    }
};

TEST_F(TaskScheduleTest, IOTaskTestExecFailed)
{
    TaskPool* taskPool = TaskPool::GetInstance();
    TaskScheduler taskScheduler(*taskPool);
    std::shared_ptr<BlockTask> task = std::make_shared<IOTaskTestFailed>();

    bool retValue = taskScheduler.Put(task);
    waitTashFinish(taskScheduler, 1);
    EXPECT_EQ(retValue, true);
}

TEST_F(TaskScheduleTest, TaskNullFailed)
{
    TaskOutq outQue;
    std::shared_ptr<IOTaskTest> task = nullptr;
    TaskScheduler::TaskWrapper taskWrapper(task, outQue);

    int32_t retValue = taskWrapper.Exec();
    EXPECT_EQ(retValue, FAILED);
}

}