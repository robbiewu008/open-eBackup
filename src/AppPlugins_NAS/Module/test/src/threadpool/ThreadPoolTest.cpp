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
#include <stdio.h>
#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "llt_stub/stub.h"
#include "llt_stub/addr_pri.h"

#include "threadpool/ThreadPoolFactory.h"

using namespace std;
using namespace Module;

/*
* 用例名称：初始化线程池
* 前置条件：无
* check点：线程池创建成功
*/
TEST(ThreadPoolTest, GetThreadPoolInstance_1)
{
    std::string key = "test";
    int size = 10;
    ThreadPool* ptr = ThreadPoolFactory::GetThreadPoolInstance(key, size);

    EXPECT_NE(ptr, nullptr);
}
