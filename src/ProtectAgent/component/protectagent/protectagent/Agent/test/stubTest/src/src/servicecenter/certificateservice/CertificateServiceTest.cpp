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
#include "certificateservice/CertificateServiceTest.h"
#include "servicefactory/include/ServiceFactory.h"
#include "certificateservice/include/ICertificateService.h"
#include "certificateservice/detail/CertificateService.h"
#include "certificateservice/include/ICertificateComm.h"
#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"

using namespace servicecenter;
using namespace certificateservice;
using namespace certificateservice::detail;

namespace {
mp_void LogTest()
{}
#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub11.set(ADDR(CLogger, Log), LogTest);                                                                       \
    } while (0)

static mp_int32 StubGetValueStringSec(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

static mp_int32 StubGetValueStringParentSec(
    mp_void* pthis, const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey, mp_string& strValue)
{
    strValue = "value";
    return MP_SUCCESS;
}
}  // namespace

class CertificatePathProxyStub : public ICertificatePathProxy {
public:
    CertificatePathProxyStub() = default;
    virtual ~CertificatePathProxyStub() = default;
    virtual std::string GetCertificateRootPath()
    {
        return "/test";
    }
    virtual std::string GetCertificateFileName(CertificateType type)
    {
        return "test.pem";
    }
    virtual bool GetCertificateConfig(CertificateConfig config, std::string& value)
    {
        if (config == CertificateConfig::PASSWORD) {
            value = "123456";
        } else {
            value = "test";
        }

        return true;
    }
};

void DecryptStrStub(const std::string& instr, std::string& outstr)
{
    outstr = instr;
}

static bool InitPathAndLog()
{
    std::string strRootPath = "../../..";
    CPath::GetInstance().SetRootPath(strRootPath);
    auto path = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    auto ret = CConfigXmlParser::GetInstance().Init(path);
    if (ret != MP_SUCCESS) {
        return false;
    }
    return true;
}

/*
 * 用例名称：获取默认证书服务句柄
 * 前置条件：1、证书服务已注册到服务中心
 * check点：1、默认证书服务句柄不为空
 */
TEST_F(CertificateServiceTest, certificate_service_get_handler_test_not_true)
{
    DoLogTest();
    auto ret = ServiceFactory::GetInstance()->Register<CertificateService>("ICertificateService");
    std::shared_ptr<ICertificateService> service = ServiceFactory::GetInstance()->GetService<ICertificateService>(
        "ICertificateService");
    auto handler = service->GetCertificateHandler();
    EXPECT_NE(handler, nullptr);
}

/*
 * 用例名称：默认证书服务句柄使用nignx证书
 * 前置条件：1、获取默认证书服务句柄
 * check点：1、获取nignx证书路径不为空
 */
TEST_F(CertificateServiceTest, certificate_handler_get_certificate_path_test_not_empty)
{
    DoLogTest();
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    std::shared_ptr<ICertificateService> service = ServiceFactory::GetInstance()->GetService<ICertificateService>(
        "ICertificateService");
    auto handler = service->GetCertificateHandler();
    auto path = handler->GetCertificateFile(CertificateType::KEY_FILE);
    EXPECT_EQ(path, "../../../nginx/conf//server.key");
}

/*
 * 用例名称：默认证书服务句柄获取密码
 * 前置条件：1、获取默认证书服务句柄
 * check点：1、获取nignx证书路径不为空
 */
TEST_F(CertificateServiceTest, certificate_handler_get_certificate_password_test_not_empty)
{
    DoLogTest();
    stub11.set(&DecryptStr, DecryptStrStub);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    std::shared_ptr<ICertificateService> service = ServiceFactory::GetInstance()->GetService<ICertificateService>(
        "ICertificateService");
    auto handler = service->GetCertificateHandler();
    std::string ps;
    EXPECT_EQ(true, handler->GetCertificateConfig(CertificateConfig::PASSWORD, ps));
}

/*
 * 用例名称：替换证书服务句柄获取算法配置
 * 前置条件：1、替换默认证书服务句柄
 * check点：1、获取nignx证书路径不为空
 */
TEST_F(CertificateServiceTest, certificate_user_handler_get_certificate_suite_test_true)
{
    DoLogTest();
    stub11.set(&DecryptStr, DecryptStrStub);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    std::shared_ptr<ICertificateService> service = ServiceFactory::GetInstance()->GetService<ICertificateService>(
        "ICertificateService");
    auto handler = service->GetCertificateHandler();
    std::string suite;
    auto config = handler->GetCertificateConfig(CertificateConfig::ALGORITEHM_SUITE, suite);
    EXPECT_EQ(config, true);
}

/*
 * 用例名称：替换证书服务句柄获取密码
 * 前置条件：1、替换默认证书服务句柄
 * check点：1、获取获取密码不为空
 */
TEST_F(CertificateServiceTest, certificate_user_handler_get_certificate_password_test_not_empty)
{
    DoLogTest();
    stub11.set(&DecryptStr, DecryptStrStub);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    std::shared_ptr<ICertificateService> service = ServiceFactory::GetInstance()->GetService<ICertificateService>(
        "ICertificateService");
    auto proxy = std::make_shared<CertificatePathProxyStub>();
    auto handler = service->GetCertificateHandler(proxy);
    std::string ps;
    auto path = handler->GetCertificateConfig(CertificateConfig::PASSWORD, ps);
    EXPECT_EQ(ps, "123456");
}

/*
 * 用例名称：替换证书服务句柄获取密码
 * 前置条件：1、替换默认证书服务句柄
 * check点：1、获取获取key文件不为空
 */
TEST_F(CertificateServiceTest, certificate_user_handler_get_certificate_file_test_not_empty)
{
    DoLogTest();
    stub11.set(&DecryptStr, DecryptStrStub);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    std::shared_ptr<ICertificateService> service = ServiceFactory::GetInstance()->GetService<ICertificateService>(
        "ICertificateService");
    auto proxy = std::make_shared<CertificatePathProxyStub>();
    auto handler = service->GetCertificateHandler(proxy);
    auto path = handler->GetCertificateFile(CertificateType::KEY_FILE);
    EXPECT_EQ(path, "/test/test.pem");
}

/*
 * 用例名称：获取证书的域名主机
 * 前置条件：1、获取默认证书服务句柄
 * check点：1、获取获取证书域名是否为空
 */
TEST_F(CertificateServiceTest, certificate_user_handler_get_certificate_hostname_test_not_empty)
{
    DoLogTest();
    stub11.set(&DecryptStr, DecryptStrStub);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    std::shared_ptr<ICertificateService> service = ServiceFactory::GetInstance()->GetService<ICertificateService>(
        "ICertificateService");
    auto handler = service->GetCertificateHandler();
    std::string hn;
    auto path = handler->GetCertificateConfig(CertificateConfig::HOST_NAME, hn);
}
