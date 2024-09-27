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
#include <iostream>
#include "gtest/gtest.h"
#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/value.h"
#include "common/ErrorCode.h"
#include "common/Log.h"

#define private public  // hack complier
#define protected public
#include "message/tcp/CDppMessage.h"
#include "plugins/oraclenative/OracleNativeBackupPlugin.h"
#undef private
#undef protected

#undef LOGGUARD
#undef COMMLOG
#define LOGGUARD(pszFormat, ...) {}
#define COMMLOG(iLevel, pszFormat, ...) {}

using namespace std;

class TestOracleNativeBackupPlugin : public testing::Test {
public:
    static void SetUpTestCase()
    {
        cout << "SetUpTestCase" << endl;
    }
    static void TearDownTestCase()
    {
        cout << "TearDownTestCase" << endl;
    }

private:
};

/********
测试用例描述:
    预置条件:  无
    操作步骤:  测试查询数据库类型
    预期结果:  
********/
TEST(TestGetDBType, TestNullMsg)
{
    // Mock Log单例类
    

    // 输入为空
    OracleNativeBackupPlugin oracleNativeBackupPlugin;
    CDppMessage reqMsg;
    CDppMessage rspMsg;
    EXPECT_EQ(oracleNativeBackupPlugin.GetDBStorType(reqMsg, rspMsg), MP_FAILED);

    // 部分字段为空
    Json::Value reqMsgVal;
    Json::Value reqBody;
    reqBody["dbName"] = "db1";
    reqBody["instName"] = "inst1";
    reqBody["dbUser"] = "dbUser";
    reqMsgVal["body"] = reqBody;
    reqMsg.SetMsgBody(reqMsgVal);
    reqMsg.InitMsgHead(MSG_DATA_TYPE_MANAGE, 0, 0);
    EXPECT_EQ(oracleNativeBackupPlugin.GetDBStorType(reqMsg, rspMsg), MP_FAILED);

    reqBody["dbPwd"] = "dbPwd";
    EXPECT_EQ(MP_SUCCESS, MP_FAILED);
    EXPECT_EQ(oracleNativeBackupPlugin.GetDBStorType(reqMsg, rspMsg), MP_FAILED);

}

