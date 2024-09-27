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
#include "mockcpp/mokc.h"

#include "../../../../virtualization/src/job_controller/manager/JobManager.h"

namespace HDT_TEST {
class JobManagerTest : public testing::Test {
protected:
    void SetUp()
    {
    }

    void TearDown()
    {
        GlobalMockObject::verify();
        std::cout << "TearDown func call" << std::endl;
    }
};

/*
 * 用例名称：中止任务
 * 前置条件：无
 * check点：HDT使用验证
 */
TEST_F(JobManagerTest, AbortJob)
{
    JobManager mgr;
    mgr.AbortJob("1123");
    std::cout << "AbortJob test func call" << std::endl;
}
}