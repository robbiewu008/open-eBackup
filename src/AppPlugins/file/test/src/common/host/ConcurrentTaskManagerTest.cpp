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
#include <chrono>
#include <thread>

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"
#include "host/HostCommonService.h"
#include "ConcurrentTaskManager.h"

using namespace std;
using namespace FilePlugin;

int ConfigReader_getInt_stub(void*)
{
    return 1;
}

/*
 * 用例名称:ConcurrentTaskManagerTest
 * 前置条件：
 * check点：
 */
TEST(ConcurrentTaskManagerTest, TestSingle)
{
    Stub stub;
    stub.set(ADDR(Module::ConfigReader, getInt), ConfigReader_getInt_stub);
    std::string taskID = "pujipa";
    std::shared_ptr<void> defer(nullptr, [&](...) {
        ConcurrentTaskManager::GetConcurrentTaskManager().ReleaseLock(taskID);
    });
    while (!ConcurrentTaskManager::GetConcurrentTaskManager().AccquireLock(taskID)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    stub.reset(ADDR(Module::ConfigReader, getInt));
}

/*
 * 用例名称:ConcurrentTaskManagerTest
 * 前置条件：
 * check点：
 */
TEST(ConcurrentTaskManagerTest, TestConcurrent)
{
    Stub stub;
    stub.set(ADDR(Module::ConfigReader, getInt), ConfigReader_getInt_stub);
    std::vector<std::thread> workers;
    for (const std::string& taskID : std::vector<std::string> { "114", "514", "1919", "810"}) {
        workers.emplace_back(std::thread([taskID](){
            std::shared_ptr<void> defer(nullptr, [&](...) {
                ConcurrentTaskManager::GetConcurrentTaskManager().ReleaseLock(taskID);
            });
            while (!ConcurrentTaskManager::GetConcurrentTaskManager().AccquireLock(taskID)) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }));
    }
    for (auto& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }
    stub.reset(ADDR(Module::ConfigReader, getInt));
}
