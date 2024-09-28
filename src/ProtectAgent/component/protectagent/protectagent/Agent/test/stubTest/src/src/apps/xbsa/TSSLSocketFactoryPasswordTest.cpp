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
#include "XBSACom/TSSLSocketFactoryPassword.h"
#include "common/Log.h"
#include "common/Path.h"
#include "securecom/CryptAlg.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class TSSLSocketFactoryPasswordTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

};

void TSSLSocketFactoryPasswordTest::SetUp() {}
void TSSLSocketFactoryPasswordTest::TearDown() {}
void TSSLSocketFactoryPasswordTest::SetUpTestCase() {}
void TSSLSocketFactoryPasswordTest::TearDownTestCase() {}

namespace {
mp_void CLogger_Log_Stub(mp_void* pthis)
{
    return;
}

mp_int32 CConfigXmlParser_Init_Stub(mp_string strCfgFilePath)
{
    return MP_SUCCESS;
}

mp_int32 CConfigXmlParser_Init_Failed_Stub(mp_string strCfgFilePath)
{
    return MP_FAILED;
}

mp_int32 CConfigXmlParser_GetValueString_Stub(const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey,
    mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 CConfigXmlParser_GetValueString_Failed_Stub(const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey,
    mp_string& strValue)
{
    return MP_FAILED;
}

mp_void AGENT_API DecryptStr_Stub(const mp_string& inStr, mp_string& outStr)
{}

}

TEST_F(TSSLSocketFactoryPasswordTest, getPassword) {
    TSSLSocketFactoryPassword om;
    std::string password = "123";
    int size;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&, mp_string&))
        ADDR(CConfigXmlParser,GetValueString), CConfigXmlParser_GetValueString_Stub);
    stub.set(DecryptStr, DecryptStr_Stub);
    om.getPassword(password, size);
}

TEST_F(TSSLSocketFactoryPasswordTest, getPassword1) {
    TSSLSocketFactoryPassword om;
    std::string password = "123";
    int size;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Failed_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&, mp_string&))
        ADDR(CConfigXmlParser,GetValueString), CConfigXmlParser_GetValueString_Stub);
    stub.set(DecryptStr, DecryptStr_Stub);
    om.getPassword(password, size);
}

TEST_F(TSSLSocketFactoryPasswordTest, getPassword2) {
    TSSLSocketFactoryPassword om;
    std::string password = "123";
    int size;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&, mp_string&))
        ADDR(CConfigXmlParser,GetValueString), CConfigXmlParser_GetValueString_Failed_Stub);
    stub.set(DecryptStr, DecryptStr_Stub);
    om.getPassword(password, size);
}

TEST_F(TSSLSocketFactoryPasswordTest, getPassword3) {
    TSSLSocketFactoryPassword om;
    std::string password;
    int size;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&, mp_string&))
        ADDR(CConfigXmlParser,GetValueString), CConfigXmlParser_GetValueString_Stub);
    stub.set(DecryptStr, DecryptStr_Stub);
    om.getPassword(password, size);
}
