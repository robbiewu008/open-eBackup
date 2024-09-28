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
#include "common/AlarmInfoXmlParserTest.h"
#include "common/File.h"
#include "common/ErrorCode.h"

using namespace std;

namespace {
mp_void StubCLoggerLog(mp_void){
    return;
}

mp_bool StubTrue(mp_void *pthis)
{
    return MP_TRUE;
}

mp_bool StubFalse(mp_void *pthis)
{
    return MP_FALSE;
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_void StubVoid(mp_void *pthis)
{
    return;
}
}

/*
*用例名称：初始化函数
*前置条件：无
*check点：导入xml配置文件成功，初始化成功
*/
TEST_F(AlarmInfoXmlParserTest, Init)
{
    mp_string strCfgFilePath;
    mp_int32 iRet;

    stub.set(&AlarmInfoXmlParser::Load, StubSuccess);
    iRet = AlarmInfoXmlParser::GetInstance().Init(strCfgFilePath);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
*用例名称：导入xml配置文件
*前置条件：无
*check点：导入xml配置文件失败
*/
TEST_F(AlarmInfoXmlParserTest, Load)
{
    mp_int32 iRet;

    {
        stub.set(&CMpFile::FileExist, StubFalse);
        iRet = AlarmInfoXmlParser::GetInstance().Load();
        EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);
    }

    {
        stub.set(&CMpFile::FileExist, StubTrue);
        iRet = AlarmInfoXmlParser::GetInstance().Load();
        EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);
    }
}

/*
*用例名称：判断配置文件导入后是否被修改过
*前置条件：配置文件存在
*check点：1、文件路径存在 2、判断是否修改
*/
TEST_F(AlarmInfoXmlParserTest, IsModified)
{
    mp_bool bRet;
    AlarmInfoXmlParser::GetInstance().m_strCfgFilePath = "";

    {
        // m_strCfgFilePath为空
        bRet = AlarmInfoXmlParser::GetInstance().IsModified();
        EXPECT_EQ(bRet, MP_FALSE);
    }

    AlarmInfoXmlParser::GetInstance().m_strCfgFilePath = "testPath";
    AlarmInfoXmlParser::GetInstance().m_lastTime = 1;
    {
        stub.set(&CMpFile::GetlLastModifyTime, StubFailed);
        bRet = AlarmInfoXmlParser::GetInstance().IsModified();
        EXPECT_EQ(bRet, MP_FALSE);
    }

    {
        stub.set(&CMpFile::GetlLastModifyTime, StubSuccess);
        bRet = AlarmInfoXmlParser::GetInstance().IsModified();
        EXPECT_EQ(bRet, MP_TRUE);
    }
}

TEST_F(AlarmInfoXmlParserTest, GetAlarmElementT)
{
    mp_string stralarmid = "Alarm";
    stub.set(&AlarmInfoXmlParser::IsModified, StubTrue);
    AlarmInfoXmlParser::GetInstance().GetAlarmElement(stralarmid);
    stub.set(&AlarmInfoXmlParser::Load, StubSuccess);
    AlarmInfoXmlParser::GetInstance().GetAlarmElement(stralarmid);
}

tinyxml2::XMLElement* stub_GetAlarmElement(){
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* element = doc.NewElement("Alarm");
    return element;
}

TEST_F(AlarmInfoXmlParserTest, GetRectificationT)
{
    mp_string stralarmid = "Alarm";
    mp_string ret = AlarmInfoXmlParser::GetInstance().GetRectification(stralarmid);
    EXPECT_EQ(ret, "");
    {
        stub.set(&AlarmInfoXmlParser::GetAlarmElement, stub_GetAlarmElement);
        ret = AlarmInfoXmlParser::GetInstance().GetRectification(stralarmid);
        EXPECT_EQ(ret, "");
    }

    ret = AlarmInfoXmlParser::GetInstance().GetFaultTitle(stralarmid);
    EXPECT_EQ(ret, "");
    {
        stub.set(&AlarmInfoXmlParser::GetAlarmElement, stub_GetAlarmElement);
        ret = AlarmInfoXmlParser::GetInstance().GetFaultTitle(stralarmid);
        EXPECT_EQ(ret, "");
    }

    ret = AlarmInfoXmlParser::GetInstance().GetAdditionInfo(stralarmid);
    EXPECT_EQ(ret, "");
    {
        stub.set(&AlarmInfoXmlParser::GetAlarmElement, stub_GetAlarmElement);
        ret = AlarmInfoXmlParser::GetInstance().GetAdditionInfo(stralarmid);
        EXPECT_EQ(ret, "");
    }

    mp_int32 bret = AlarmInfoXmlParser::GetInstance().GetFaultType(stralarmid);
    EXPECT_EQ(bret, 0);
    {
        stub.set(&AlarmInfoXmlParser::GetAlarmElement, stub_GetAlarmElement);
        bret = AlarmInfoXmlParser::GetInstance().GetFaultType(stralarmid);
        EXPECT_EQ(bret, 0);
    }

    bret = AlarmInfoXmlParser::GetInstance().GetFaultLevel(stralarmid);
    EXPECT_EQ(bret, 0);
    {
        stub.set(&AlarmInfoXmlParser::GetAlarmElement, stub_GetAlarmElement);
        bret = AlarmInfoXmlParser::GetInstance().GetFaultLevel(stralarmid);
        EXPECT_EQ(bret, 0);
    }

    bret = AlarmInfoXmlParser::GetInstance().GetFaultCategory(stralarmid);
    EXPECT_EQ(bret, 0);
    {
        stub.set(&AlarmInfoXmlParser::GetAlarmElement, stub_GetAlarmElement);
        bret = AlarmInfoXmlParser::GetInstance().GetFaultCategory(stralarmid);
        EXPECT_EQ(bret, 0);
    }
}
