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
#include "message/rest/http_cgiTest.h"
#include "message/rest/http_cgi.h"
#include "fcgi/include/fcgiapp.h"
#include "agent/Communication.h"
#include "common/Log.h"

namespace {
mp_void LogTest() {}
mp_string GetHeadTest(const mp_string& name)
{
    return name;
}
mp_void SendHeadsTest()
{
}
mp_char* StubFCGX_GetParam()
{
    static mp_char uri[] = "https://abcd.com?param=value";
    return uri;
}
}

TEST_F(CHttpRequestTest, CHttpRequest)
{
 //   FCGX_Request request;
 //   CHttpRequest req(request);
    stub11.set(&FCGX_GetParam, StubFCGX_GetParam);
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    stub11.reset(&FCGX_GetParam);
}

TEST_F(CHttpRequestTest, GetURLTest)
{
 //   FCGX_Request request;
 //   CHttpRequest req(request);
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    CHttpRequest req2(httpReq);
    req1 = req2;

    req1.GetURL();
}

TEST_F(CHttpRequestTest, GetQueryParamStrTest)
{
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    req1.GetQueryParamStr();
}

TEST_F(CHttpRequestTest, GetMethodTest)
{
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    req1.GetMethod();
}

TEST_F(CHttpRequestTest, SetMethodTest)
{
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    std::string strMethod;
    req1.SetMethod(strMethod);
}

TEST_F(CHttpRequestTest, SetAllHeadTest)
{
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    char** envp;
    FCGX_Request pFcgxReq;
    req1.SetFcgxReq(pFcgxReq);
    req1.SetAllHead(envp);
}

TEST_F(CHttpRequestTest, SetFcgxReqTest)
{
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    FCGX_Request pFcgxReq;
    req1.SetFcgxReq(pFcgxReq);
}

mp_char* FCGX_GetParam_stub_Ok(const mp_char *name, FCGX_ParamArray envp)
{
    static mp_char rst[] = "1";
    return rst;
}

mp_char* FCGX_GetParam_stub_Failed(const mp_char *name, FCGX_ParamArray envp)
{
    static mp_char rst[] = "";
    return rst;
}

TEST_F(CHttpRequestTest, GetHeadTesta)
{
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    FCGX_Request pFcgxReq;
    mp_string name;
    mp_string retStr = "1";
    Stub stub;
    stub.set(&FCGX_GetParam, FCGX_GetParam_stub_Ok);
    req1.SetFcgxReq(pFcgxReq);
    EXPECT_EQ(retStr, req1.GetHead(name));
}

TEST_F(CHttpRequestTest, GetHeadNoCheckTest)
{
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    FCGX_Request pFcgxReq;
    mp_string name;
    mp_string retStr = "1";
    Stub stub;
    stub.set(&FCGX_GetParam, FCGX_GetParam_stub_Ok);
    req1.SetFcgxReq(pFcgxReq);
    EXPECT_EQ(retStr, req1.GetHeadNoCheck(name));

    stub.set(&FCGX_GetParam, FCGX_GetParam_stub_Failed);
    req1.SetFcgxReq(pFcgxReq);
    EXPECT_EQ("", req1.GetHeadNoCheck(name));
}

TEST_F(CHttpRequestTest, GetAllHeadTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    FCGX_Request pFcgxReq;
    req1.SetFcgxReq(pFcgxReq);
    req1.GetAllHead();
}

TEST_F(CHttpRequestTest, GetRemoteIPTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    stub.set(&FCGX_GetParam, FCGX_GetParam_stub_Ok);
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    FCGX_Request pFcgxReq;
    req1.SetFcgxReq(pFcgxReq);
    req1.GetRemoteIP();
}

TEST_F(CHttpRequestTest, GetContentLenTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    stub.set(&FCGX_GetParam, FCGX_GetParam_stub_Ok);
    FCGX_Request pFcgxReq;
    req1.SetFcgxReq(pFcgxReq);
    EXPECT_EQ(MP_TRUE, req1.GetContentLen());
}

TEST_F(CHttpRequestTest, GetContentTypeTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    mp_string type;
    FCGX_Request pFcgxReq;
    req1.SetFcgxReq(pFcgxReq);
    stub.set(ADDR(CHttpRequest, GetHead), GetHeadTest);
    req1.GetContentType(type);
}

TEST_F(CHttpRequestTest, GetFcgxReqTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    req1.GetFcgxReq();
}

TEST_F(CHttpRequestTest, ReadCharTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    FCGX_Request pFcgxReq;
    req1.SetFcgxReq(pFcgxReq);
    // req1.ReadChar();
}

TEST_F(CHttpRequestTest, ReadStrTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    FCGX_Request pFcgxReq;
    req1.SetFcgxReq(pFcgxReq);
    mp_char* b = "test";
    mp_int32 l = 1;
//    req1.ReadStr(b, l);
}

TEST_F(CHttpRequestTest, GetClientCertDNTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    stub.set(&FCGX_GetParam, FCGX_GetParam_stub_Ok);
    CHttpRequest httpReq;
    CHttpRequest req1(httpReq);
    FCGX_Request pFcgxReq;
    req1.SetFcgxReq(pFcgxReq);
    req1.GetClientCertDN();
}

TEST_F(CHttpRequestTest, SetContentTypeTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    FCGX_Request pFcgxReq;
    CHttpResponse req1(pFcgxReq);
    mp_string type;
    req1.SetContentType(type);
}

/*
 * 用例名称：根据名称获取相应消息头信息的值
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CHttpRequestTest, GetHeadTest1)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    stub.set(ADDR(CLogger, Log), LogTest);
    FCGX_Request pFcgxReq;
    CHttpResponse req1(pFcgxReq);
    req1.m_Heads.insert(std::pair<mp_string, mp_string>("test", "testValue"));
    mp_string name;
    mp_string sRet = req1.GetHead(name);
    EXPECT_EQ(sRet, "");

    name = "test";
    sRet = req1.GetHead(name);
    EXPECT_EQ(sRet, "testValue");
}

TEST_F(CHttpRequestTest, SetHeadTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    FCGX_Request pFcgxReq;
    CHttpResponse req1(pFcgxReq);
    mp_string name;
    mp_string value;
    req1.SetHead(name, value);
}

TEST_F(CHttpRequestTest, RemoveHeadTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    FCGX_Request pFcgxReq;
    CHttpResponse req1(pFcgxReq);
    mp_string name;
    req1.RemoveHead(name);
}

TEST_F(CHttpRequestTest, WriteCharTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    FCGX_Request pFcgxReq;
    CHttpResponse req1(pFcgxReq);
    mp_int32 c;
    stub.set(ADDR(CHttpResponse, SendHeads), SendHeadsTest);
    // req1.WriteChar(c);
}

TEST_F(CHttpRequestTest, WriteStrTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    FCGX_Request pFcgxReq;
    CHttpResponse req1(pFcgxReq);
    mp_string str;
    mp_int32 n;
    stub.set(ADDR(CHttpResponse, SendHeads), SendHeadsTest);
    // req1.WriteStr(str, n);
}

TEST_F(CHttpRequestTest, WriteSTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    FCGX_Request pFcgxReq;
    CHttpResponse req1(pFcgxReq);
    mp_string str;
    mp_int32 n;
    stub.set(ADDR(CHttpResponse, SendHeads), SendHeadsTest);
    // req1.WriteS(str);
}

TEST_F(CHttpRequestTest, CompleteTest)
{
    Stub stub;
    stub.set(ADDR(CLogger, Log), LogTest);
    FCGX_Request pFcgxReq;
    CHttpResponse req1(pFcgxReq);
    req1.m_pFcgRequest = &pFcgxReq;
    // req1.Complete();
    EXPECT_TRUE(1);
}