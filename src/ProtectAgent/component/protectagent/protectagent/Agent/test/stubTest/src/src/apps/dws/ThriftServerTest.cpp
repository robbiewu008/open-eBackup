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
#include "apps/dws/XBSAServer/ThriftServer.h"
#include "apps/dws/XBSAServer/BSAService.h"
#include "gtest/gtest.h"
#include "stub.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::server;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
class ThriftServerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

};

void ThriftServerTest::SetUp() {}
void ThriftServerTest::TearDown() {}
void ThriftServerTest::SetUpTestCase() {}
void ThriftServerTest::TearDownTestCase() {}

namespace {
mp_void CLogger_Log_Stub(mp_void* pthis)
{
    return;
}

}

mp_int32 CConfigXmlParser_GetValueBool_Stub(void *obj, const mp_string& strSection, const mp_string& strKey, mp_bool& bValue)
{
    ThriftServer *o = (ThriftServer *)obj;
    mp_bool b = 1;
    bValue = b;
    return MP_SUCCESS;
}

mp_int32 CConfigXmlParser_Init_Stub(mp_string strCfgFilePath)
{
    return MP_SUCCESS;
}

mp_int32 CConfigXmlParser_GetValueString_Stub(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

int ThriftServer_Init_Stub(std::string certFilePath, std::string cipherkey, int32_t &sslFlag)
{
    return MP_SUCCESS;
}

void TSSLSocketFactory_ciphers_Stub(const string& enable)
{}

void TSSLSocketFactory_loadCertificate_Stub(const char* path, const char* format = "PEM")
{}

void TSSLSocketFactory_overrideDefaultPasswordCallback_Stub()
{}

void TSSLSocketFactory_loadPrivateKey_Stub(const char* path, const char* format = "PEM")
{}

std::shared_ptr<TSSLSocketFactoryPassword> ThriftServer_createServerSocketFactory_Stub(const std::string &certFilePath,
    const std::string &cipherkey)
{
    std::shared_ptr<TSSLSocketFactoryPassword> pServerSocketFactory;
    pServerSocketFactory.reset(new TSSLSocketFactoryPassword());
    return pServerSocketFactory;
}


void Thread_Start_Stub()
{}

void Thread_Join_Stub()
{}
/*
TEST_F(ThriftServerTest, Init) {
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString),
        CConfigXmlParser_GetValueString_Stub);
    stub.set((int(ThriftServer::*)(std::string, std::string, int32_t &))ADDR(ThriftServer,Init), ThriftServer_Init_Stub);
    EXPECT_EQ(MP_SUCCESS, ThriftServer::GetInstance()->Init());
}

TEST_F(ThriftServerTest, Init2) {
    std::string certFilePath;
    std::string cipherkey;
    int32_t sslFlag = 1;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(ThriftServer, createServerSocketFactory), ThriftServer_createServerSocketFactory_Stub);
    typedef void (*fptr1)(apache::thrift::concurrency::Thread*);
    fptr1 fptr_start = (fptr1)(&apache::thrift::concurrency::Thread::start);
    stub.set(fptr_start, Thread_Start_Stub);
    typedef void (*fptr2)(apache::thrift::concurrency::Thread*);
    fptr2 fptr_join = (fptr2)(&apache::thrift::concurrency::Thread::join);
    stub.set(fptr_join, Thread_Join_Stub);
    EXPECT_EQ(MP_SUCCESS, ThriftServer::GetInstance()->Init(certFilePath, cipherkey, sslFlag));
    ThriftServer::GetInstance()->m_threadPtr->detached_ = true;
}
*/
TEST_F(ThriftServerTest, createServerSocketFactory) {
    std::string certFilePath;
    std::string cipherkey;
    Stub stub;
    typedef void (*fptr1)(TSSLSocketFactory*, const string&);
    fptr1 fptr_ciphers = (fptr1)(&TSSLSocketFactory::ciphers);
    stub.set(fptr_ciphers, TSSLSocketFactory_ciphers_Stub);
    typedef void (*fptr2)(TSSLSocketFactory*, const char*, const char*);
    fptr2 fptr_loadCertificate = (fptr2)(&TSSLSocketFactory::loadCertificate);
    stub.set(fptr_loadCertificate, TSSLSocketFactory_loadCertificate_Stub);
    typedef void (*fptr3)(TSSLSocketFactory*);
    fptr3 fptr_overrideDefaultPasswordCallback = (fptr3)(&TSSLSocketFactory::overrideDefaultPasswordCallback);
    stub.set(fptr_overrideDefaultPasswordCallback, TSSLSocketFactory_overrideDefaultPasswordCallback_Stub);
    typedef void (*fptr4)(TSSLSocketFactory*, const char*, const char*);
    fptr4 fptr_loadPrivateKey = (fptr4)(&TSSLSocketFactory::loadPrivateKey);
    stub.set(fptr_loadPrivateKey, TSSLSocketFactory_loadPrivateKey_Stub);
    ThriftServer::GetInstance()->createServerSocketFactory(certFilePath, cipherkey);
}
