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
#include <memory>
#include <shared_mutex>
#include "config_reader/ConfigIniReader.h"
#include "common/FSBackupUtils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "llt_stub/stub.h"
#include "llt_stub/addr_pri.h"
#include "LibsmbHardlinkWriter.h"
#include "libsmb_ctx/SmbContextWrapper.h"

using namespace std;
namespace  {
}

class LibsmbHardlinkWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void LibsmbHardlinkWriterTest::SetUp()
{}

void LibsmbHardlinkWriterTest::TearDown()
{}

void LibsmbHardlinkWriterTest::SetUpTestCase()
{}

void LibsmbHardlinkWriterTest::TearDownTestCase()
{}

/*
 * 用例名称：验证LibsmbHardlinkWriter的IsComplete接口
 * 前置条件：无
 * check点：在ControlFileReader没有file的情况下，IsComplete返回true
 */
// TEST_F(LibsmbHardlinkWriterTest, IsComplete) {
//     BackupParams backupParams;
//     backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

//     WriterParams hardlinkWriterParams {};
//     hardlinkWriterParams.backupParams = backupParams;
//     hardlinkWriterParams.writeQueuePtr = nullptr;
//     hardlinkWriterParams.readQueuePtr = nullptr;
//     hardlinkWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
//     hardlinkWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

//     LibsmbHardlinkWriter libsmbHardlinkWriter(hardlinkWriterParams);

//     hardlinkWriterParams.controlInfo->m_aggregatePhaseComplete = true;
//     EXPECT_EQ(libsmbHardlinkWriter.IsComplete(), true);
// }
