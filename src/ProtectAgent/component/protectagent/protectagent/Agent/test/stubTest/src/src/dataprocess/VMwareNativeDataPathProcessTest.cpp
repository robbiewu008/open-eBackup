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
#include "message/tcp/CDppMessage.h"
#include "dataprocess/VMwareNativeDataPathProcessTest.h"

static mp_void StubCLoggerLogPath(mp_void){
    return;
}
namespace {
    const std::string vddkVersion = "6.7";
    // VMwareNativeVddkInit
    Json::Value vddkReqBody =
        "{\"ProductManager\":{\"Certs\":[\"\"],\"IP\":\"10.10.10.10\",\"Password\":\"******\",\"Port\":443,\"Protocol\":0, \
        \"UserName\":\"admin@vsphere.local\",\"Version\":\"6.7.0\"}, \
        \"parentTaskId\":\"9d139626-9048-4d1f-9939-160a91cc1ace\",\"taskId\":\"9d139626-9048-4d1f-9939-160a91cc1ace\",\
        \"vmInfo\":{\"vmID\":\"501f33cd-4caf-a27a-8df8-b2bed9c29744\",\"vmName\":\"test_vm_0001\",\
        \"vmRef\":\"vm-71024\"}},\"cmd\":1108}";

    CDppMessage msg;
    std::shared_ptr<CDppMessage> vddkMsg(std::make_shared<CDppMessage>());
    // vddkMsg.get()->SetMsgBody(vddkReqBody);
    std::string taskId = "9d139626-9048-4d1f-9939-160a91cc1ace";
    std::string parentTaskId = "9d139626-9048-4d1f-9939-160a91cc1ace";

    TEST_F(VMwareNativeDataPathProcessTest, HandleVddkLibCleanup_success)
    {   
        Stub stub;
        stub.set(&CLogger::Log, StubCLoggerLogPath);
        VMwareNativeDataPathProcess dp(vddkVersion);
        // vm level vddk init
        EXPECT_EQ(MP_FAILED, dp.HandleVddkLibCleanup(vddkMsg, taskId, parentTaskId));
    }

    // HandleDataBlockBackupFinish
    Json::Value finishDiskReqBody = "{\"FinishDisk\":true,\"diskID\":\"6000c291-be2d-a8da-8b66-292e6eeb6e6a\",\
        \"parentTaskId\":\"9d139626-9048-4d1f-9939-160a91cc1ace\",\"taskId\":\"3ca67c73-f0f5-4c54-a3a3-68d92b1669e6\"},\
        \"cmd\":1084}";
    std::shared_ptr<CDppMessage> finishBKMsg(std::make_shared<CDppMessage>());
    // finishBKMsg->SetMsgBody(finishDiskReqBody);
    // TEST_F(VMwareNativeDataPathProcessTest, HandleDataBlockBackupFinish_success)
    // {
    //     Stub stub;
    //     stub.set(&CLogger::Log, StubCLoggerLogPath);
    //     VMwareNativeDataPathProcess dp(vddkVersion);
    //     // vm level vddk init
    //     EXPECT_EQ(MP_FAILED, dp.HandleDataBlockBackupFinish(finishBKMsg, taskId, parentTaskId));
    // }

    // TEST_F(VMwareNativeDataPathProcessTest, HandleDataBlockBackupFinish_failure)
    // {
    //     Stub stub;
    //     stub.set(&CLogger::Log, StubCLoggerLogPath);
    //     VMwareNativeDataPathProcess dp(vddkVersion);
    //     // vm level vddk init
    //     Json::Value nullBody;
    //     EXPECT_NE(0, dp.HandleDataBlockBackupFinish(finishBKMsg, "", parentTaskId));
    // }

    // // HandleDataBlockRestoreFinish
    // Json::Value restoreFinishDiskReqBody = "{\"FinishDisk\":true,\"diskID\":\"6000c291-be2d-a8da-8b66-292e6eeb6e6a\",\
    //     \"parentTaskId\":\"9d139626-9048-4d1f-9939-160a91cc1ace\",\"taskId\":\"3ca67c73-f0f5-4c54-a3a3-68d92b1669e6\"},\
    //     \"cmd\":1096}";
    // std::shared_ptr<CDppMessage> finishRecoveryMsg(std::make_shared<CDppMessage>());
    // // finishRecoveryMsg->SetMsgBody(finishDiskReqBody);
    // TEST_F(VMwareNativeDataPathProcessTest, HandleDataBlockRestoreFinish_success)
    // {
    //     Stub stub;
    //     stub.set(&CLogger::Log, StubCLoggerLogPath);
    //     VMwareNativeDataPathProcess dp(vddkVersion);
    //     // vm level vddk init
    //     EXPECT_EQ(0, dp.HandleDataBlockRestoreFinish(finishRecoveryMsg, taskId, parentTaskId));
    // }

    // TEST_F(VMwareNativeDataPathProcessTest, HandleDataBlockRestoreFinish_failure)
    // {
    //     Stub stub;
    //     stub.set(&CLogger::Log, StubCLoggerLogPath);
    //     VMwareNativeDataPathProcess dp(vddkVersion);
    //     // vm level vddk init
    //     EXPECT_NE(0, dp.HandleDataBlockRestoreFinish(finishRecoveryMsg, "", parentTaskId));
    // }
}
