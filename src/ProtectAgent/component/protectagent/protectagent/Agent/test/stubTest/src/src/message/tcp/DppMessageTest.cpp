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
#include "message/tcp/DppMessageTest.h"
#include "common/JsonUtils.h"

namespace{
mp_int32 StubFailed(mp_void* pobj)
{
    return MP_FAILED;
}

mp_int32 StubSuccess(mp_void* pobj)
{
    return MP_SUCCESS;
}
}

#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubCDppMessageGetValueInt32Return);                                                                      \
    } while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(CDppMessageTest, InitMsgBody)
{
    CDppMessage om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.InitMsgBody();
}

TEST_F(CDppMessageTest, DestroyMsgBody)
{
    CDppMessage om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    //om.dppMessage.body = "test";
    om.DestroyMsgBody();
}

TEST_F(CDppMessageTest, CDppMessage)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CDppMessage msg;
    msg.mErrNo = 1234;
    CDppMessage om(msg);
}

TEST_F(CDppMessageTest, InitMsgHead)
{
    mp_uint16 cmdNo = 123;
    mp_uint16 flag = 1;
    mp_uint64 seqNo = 456;
    CDppMessage om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.InitMsgHead(cmdNo, flag, seqNo);
}

TEST_F(CDppMessageTest, ReinitMsgBody)
{
    CDppMessage om;
    Json::Value manageBody = "body";
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.ReinitMsgBody();
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(CDppMessage, InitMsgBody), StubFailed);
    iRet = om.ReinitMsgBody();
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(CDppMessageTest, CloneMsg)
{
    CDppMessage om;
    Json::Value manageBody = "body";
    CDppMessage msg;
    msg.mCmd = 1001;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.CloneMsg(msg);
}

TEST_F(CDppMessageTest, GetIpAddr)
{
    CDppMessage om;
    om.dppMessage.uiIpAddr = "192.192";
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string iRet = om.GetIpAddr();
    EXPECT_EQ("192.192", iRet);

    mp_string strRet = om.GetIpAddrStr();
    EXPECT_EQ("192.192", strRet);
}

TEST_F(CDppMessageTest, GetPort)
{
    CDppMessage om;
    om.dppMessage.uiPort = 1001;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_uint16 iRet = om.GetPort();
    EXPECT_EQ(1001, iRet);
}

TEST_F(CDppMessageTest, IsValidPrefix)
{
    CDppMessage om;
    om.dppMessage.uiPrefix = MSG_PREFIX;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_bool bRet = om.IsValidPrefix();
    EXPECT_EQ(true, bRet);

    mp_uint32 iRet = om.GetPrefix();
    EXPECT_EQ(0x72634552, iRet);
}

TEST_F(CDppMessageTest, GetCmd)
{
    CDppMessage om;
    om.dppMessage.uiCmd = 1001;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_uint16 iRet = om.GetCmd();
    EXPECT_EQ(1001, iRet);
}

TEST_F(CDppMessageTest, GetFlag)
{
    CDppMessage om;
    om.dppMessage.uiFlag = 1001;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_uint16 iRet = om.GetFlag();
    EXPECT_EQ(1001, iRet);
}

TEST_F(CDppMessageTest, GetOrgSeqNo)
{
    CDppMessage om;
    om.dppMessage.uiOrgSeqNo = 1001;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_uint64 iRet = om.GetOrgSeqNo();
    EXPECT_EQ(1001, iRet);
}

TEST_F(CDppMessageTest, GetSize1)
{
    CDppMessage om;
    om.dppMessage.uiSize = 1001;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_uint32 iRet = om.GetSize1();
    EXPECT_EQ(20, iRet);

    iRet = om.GetSize2();
    EXPECT_EQ(1001, iRet);

    iRet = om.GetSize();
    EXPECT_EQ(1021, iRet);
}

TEST_F(CDppMessageTest, GetBuffer)
{
    CDppMessage om;
    om.dppMessage.body = new char[5];
    memcpy (om.dppMessage.body, "test", 5);
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_char* iRet = om.GetBuffer();
    EXPECT_EQ(om.dppMessage.body, iRet);
}

TEST_F(CDppMessageTest, GetStart)
{
    CDppMessage om;
    om.dppMessage.uiPrefix = 1234;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_char* iRet = om.GetStart();
    EXPECT_EQ((mp_char*)&om.dppMessage.uiPrefix, iRet);
}

TEST_F(CDppMessageTest, SetMsgSrc)
{
    CDppMessage om;
    MESSAGE_ROLE role = ROLE_HOST_AGENT;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.SetMsgSrc(role);
}

TEST_F(CDppMessageTest, SetMsgTgt)
{
    CDppMessage om;
    MESSAGE_ROLE role = ROLE_HOST_AGENT;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.SetMsgTgt(role);
}

TEST_F(CDppMessageTest, GetMsgSrc)
{
    CDppMessage om;
    om.dppMessage.uiFlag = 1234;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.GetMsgSrc();
}

TEST_F(CDppMessageTest, SwapSrcTgt)
{
    CDppMessage om;
    om.dppMessage.uiFlag = 1234;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.SwapSrcTgt();
}

TEST_F(CDppMessageTest, SetMsgBody)
{
    CDppMessage om;
    Json::Value msgBody = "test";
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.SetMsgBody(msgBody);
}

TEST_F(CDppMessageTest, SetOrgSeqNo)
{
    CDppMessage om;
    mp_uint64 uiOrgSeqNo = 1001;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.SetOrgSeqNo(uiOrgSeqNo);
}

TEST_F(CDppMessageTest, SetLinkInfo)
{
    CDppMessage om;
    mp_string uiIpAddr = "192.168";
    mp_uint16 uiPort = 1001;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.SetLinkInfo(uiIpAddr, uiPort);
}

TEST_F(CDppMessageTest, GetManageCmd)
{
    CDppMessage om;
    om.mCmd = 1;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.GetManageCmd();

    om.mCmd = 0;
    stub.set(ADDR(CDppMessage, AnalyzeManageMsg), StubAnalyzeManageMsg);
    om.GetManageCmd();
}

TEST_F(CDppMessageTest, GetManageBody)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CDppMessage om;
    Json::Value dppBody;

    stub.set(ADDR(CDppMessage, AnalyzeManageMsg), StubAnalyzeManageMsg);
    om.GetManageBody(dppBody);

    om.manageBody = "test";
    om.GetManageBody(dppBody);

    om.manageBody = "";
    stub.set(ADDR(CDppMessage, AnalyzeManageMsg), StubAnalyzeManageMsgFailed);
    om.GetManageBody(dppBody);
}

TEST_F(CDppMessageTest, GetManageError)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CDppMessage om;
    om.mCmd = -1;
    mp_int64 errNo = 123;
    mp_string errDetail = "test";
   
    stub.set(ADDR(CDppMessage, AnalyzeManageMsg), StubAnalyzeManageMsg);
    om.GetManageError(errNo, errDetail);

    om.mCmd = 1;
    om.GetManageError(errNo, errDetail);

    om.mErrNo = 0;
    om.GetManageError(errNo, errDetail);

    om.mErrNo = MANAGE_ERRNO_INVALID;
    stub.set(ADDR(CDppMessage, AnalyzeManageMsg), StubAnalyzeManageMsgFailed);
    om.GetManageError(errNo, errDetail);
}

TEST_F(CDppMessageTest, UpdateTime)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CDppMessage om;
    om.UpdateTime();
}

TEST_F(CDppMessageTest, GetUpdateTime)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CDppMessage om;
    om.GetUpdateTime();
}

TEST_F(CDppMessageTest, AnalyzeManageMsg)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CJsonUtils, GetJsonUInt32), StubGetJsonUInt32);
    stub.set(ADDR(CJsonUtils, GetJsonInt64), StubGetJsonInt64);
    stub.set(ADDR(CJsonUtils, GetJsonString), StubGetJsonString);
    CDppMessage om;
    om.dppMessage.uiCmd = 1;
    om.AnalyzeManageMsg();

    om.dppMessage.body = "";
    om.dppMessage.body = new char[1];
    memcpy (om.dppMessage.body, "", 1);
    om.dppMessage.uiCmd = MSG_DATA_TYPE_MANAGE;
    om.AnalyzeManageMsg();

    om.dppMessage.body = new char[5];
    memcpy (om.dppMessage.body, "test", 5);
    om.dppMessage.uiCmd = MSG_DATA_TYPE_MANAGE;
    om.AnalyzeManageMsg();

    om.dppMessage.body = new char[1];
    memcpy (om.dppMessage.body, "{", 1);
    mp_int32 iRet = om.AnalyzeManageMsg();
    EXPECT_EQ(iRet, MP_FAILED);

    // om.dppMessage.body = new char[5];
    // memcpy (om.dppMessage.body, "{a:b}", 5);
    // stub.set(&Reader::parse, StubFailed);
    // iRet = om.AnalyzeManageMsg();
    // EXPECT_EQ(iRet, MP_FAILED);

    om.dppMessage.body = new char[5];
    memcpy (om.dppMessage.body, "{a:b}", 5);
    stub.set(ADDR(CJsonUtils, GetJsonUInt32), StubFailed);
    iRet = om.AnalyzeManageMsg();
    EXPECT_EQ(iRet, MP_FAILED);

}

TEST_F(CDppMessageTest, ItemEendianSwap)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CDppMessage om;
    om.ItemEendianSwap();
}

