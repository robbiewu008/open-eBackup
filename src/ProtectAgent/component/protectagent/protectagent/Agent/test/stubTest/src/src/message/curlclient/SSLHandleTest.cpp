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
#include "message/curlclient/SSLHandleTest.h"
#include "message/curlclient/SSLHandle.h"
#include "common/Log.h"
#include "crypto/x509.h"
#include <string>

using namespace std;

namespace {
mp_void LogTest()
{}
#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub.set(ADDR(CLogger, Log), LogTest);                                                                         \
    } while (0)

SecuritySS_RC StubConvToX509Invalid(const mp_string &certText, STACK_OF(X509) * &pCerts)
{
    return Security_Cert_Invalid;
}

SecuritySS_RC StubConvToX509(const mp_string &certText, STACK_OF(X509) * &pCerts)
{
    return SecuritySS_Success;
}
mp_int32 Stubsk_X509_num()
{
    return 1;
}
X509* Stubsk_X509_value()
{
    return nullptr;
}
}

SecuritySS_RC StubGetCertsSuccess(const string &certText, STACK_OF(X509) * pCerts, X509 *pCert)
{
    return SecuritySS_Success;
}

SecuritySS_RC StubGetCertsFail(const string &certText, STACK_OF(X509) * pCerts, X509 *pCert)
{
    return Security_Cert_Invalid;
}

SecuritySS_RC StubAddCertToCtxFail(SSL_CTX *ctx, const mp_string &certText)
{
    return Security_Cert_Invalid;
}

TEST_F(SSLHandleTest, CleanupOpenSSLTest)
{
    SSLHandle sslHandle;
    // sslHandle.InitOpenSSL();
}

/*
 * 用例名称：检查证书
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, CheckCert)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "none";
    auto iRet = sslHandle.CheckCert(certText);
    EXPECT_EQ(iRet, SecuritySS_Success);

    certText = "test";
    iRet = sslHandle.CheckCert(certText);
    EXPECT_EQ(iRet, SecuritySS_Success);

    certText = "test";
    stub.set(&SSLHandle::ConvToX509, StubConvToX509Invalid);
    iRet = sslHandle.CheckCert(certText);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：转换为PEM证书
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, ConvToPEMCert)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "none";
    string pemCert;
    auto iRet = sslHandle.ConvToPEMCert(certText, pemCert);
    EXPECT_EQ(iRet, SecuritySS_Success);

    certText = "test";
    iRet = sslHandle.ConvToPEMCert(certText, pemCert);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：转换为PEM证书
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, EnrichSSL_Test)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "none";
    SSL_CTX *sslCtx;
    auto iRet = sslHandle.EnrichSSL(sslCtx, certText);
    EXPECT_EQ(iRet, SecuritySS_Success);
}

/*
 * 用例名称：检查证书过期
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, CheckCertExpire)
{
    DoLogTest();
    SSLHandle sslHandle;
    X509 *pCert = nullptr;
    auto iRet = sslHandle.CheckCertExpire(pCert);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：获取证书过期时间
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, GetCertExpireTime)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "none";
    time_t ExpireTime;
    auto iRet = sslHandle.GetCertExpireTime(certText, ExpireTime);
    EXPECT_EQ(iRet, SecuritySS_Success);
    
    certText = "test";
    iRet = sslHandle.GetCertExpireTime(certText, ExpireTime);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：校验证书密码
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, checkCertCipher)
{
    DoLogTest();
    SSLHandle sslHandle;
    X509 *pCert = nullptr;
    auto iRet = sslHandle.checkCertCipher(pCert);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：转换到X509
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, ConvToX509)
{
    DoLogTest();
    SSLHandle sslHandle;
    mp_string certText = "test";
    STACK_OF(X509) * pCerts = sk_X509_new_null();
    auto iRet = sslHandle.ConvToX509(certText, pCerts);
    EXPECT_EQ(iRet, SecuritySS_Success);

    pCerts = nullptr;
    iRet = sslHandle.ConvToX509(certText, pCerts);
    EXPECT_EQ(iRet, Security_Cert_Invalid);

    pCerts = sk_X509_new_null();
    certText.clear();
    iRet = sslHandle.ConvToX509(certText, pCerts);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：SslctxFunction
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, SslctxFunction)
{
    DoLogTest();
    SSLHandle sslHandle;
    mp_string certText = "test";
    SSL_CTX *sslctx = SSL_CTX_new(SSLv23_client_method());
    auto iRet = sslHandle.SslctxFunction(sslctx, certText);
    EXPECT_EQ(iRet, SecuritySS_Success);

    certText.clear();
    iRet = sslHandle.SslctxFunction(sslctx, certText);
    EXPECT_EQ(iRet, 77);
}

/*
 * 用例名称：获取指纹
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, GetThumbprint)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "test";
    string thumbprint;
    auto iRet = sslHandle.GetThumbprint(certText, thumbprint);
    EXPECT_EQ(iRet, Security_Cert_Invalid);

    certText.clear();
    iRet = sslHandle.GetThumbprint(certText, thumbprint);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：获取证书
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, GetCerts)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "test";
    STACK_OF(X509) * pCerts = sk_X509_new_null();
    X509 *pCert;
    auto iRet = sslHandle.GetCerts(certText, pCerts, pCert);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：检查是否CA证书
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, CheckWhetherCACert)
{
    DoLogTest();
    stub.set(&SSLHandle::GetCerts, StubGetCertsSuccess);
    SSLHandle sslHandle;
    string certText = "none";
    bool isCACert;
    auto iRet = sslHandle.CheckWhetherCACert(certText, isCACert);
    EXPECT_EQ(iRet, SecuritySS_Success);
    
    stub.set(&SSLHandle::GetCerts, StubGetCertsFail);
    iRet = sslHandle.CheckWhetherCACert(certText, isCACert);
    EXPECT_EQ(iRet, SecuritySS_InnerWrong);
}

/*
 * 用例名称：获取证书有效时间
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, GetCertValidTime)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "none";
    time_t StartTime;
    time_t ExpireTime;
    auto iRet = sslHandle.GetCertValidTime(certText, StartTime, ExpireTime);
    EXPECT_EQ(iRet, SecuritySS_Success);

    certText = "auto_match";
    iRet = sslHandle.GetCertValidTime(certText, StartTime, ExpireTime);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：获取证书颁发者
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, GetCertIssuer)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "none";
    string issuer;
    auto iRet = sslHandle.GetCertIssuer(certText, issuer);
    EXPECT_EQ(iRet, SecuritySS_Success);

    certText = "auto_match";
    iRet = sslHandle.GetCertIssuer(certText, issuer);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：获取证书主题
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, GetCertSubject)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "none";
    string issuer;
    auto iRet = sslHandle.GetCertSubject(certText, issuer);
    EXPECT_EQ(iRet, SecuritySS_Success);

    certText = "auto_match";
    iRet = sslHandle.GetCertSubject(certText, issuer);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：验证证书内容
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, VerifyCertContent)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "none";
    mp_string hostName = "";
    mp_string fingerprint;
    auto iRet = sslHandle.VerifyCertContent(hostName, certText, fingerprint);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
    
    stub.set(&SSLHandle::AddCertToCtx, StubAddCertToCtxFail);
    iRet = sslHandle.VerifyCertContent(hostName, certText, fingerprint);
    EXPECT_EQ(iRet, Security_Cert_Invalid);
}

/*
 * 用例名称：添加证书到Ctx
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, AddCertToCtx)
{
    DoLogTest();
    SSLHandle sslHandle;
    string certText = "none";
    SSL_CTX *ctx;
    auto iRet = sslHandle.AddCertToCtx(ctx, certText);
    EXPECT_EQ(iRet, SecuritySS_Success);
}

/*
 * 用例名称：ASN1_UTCTIME_ConvertToTimeT
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, ASN1_UTCTIME_ConvertToTimeT)
{
    DoLogTest();
    SSLHandle sslHandle;
    ASN1_UTCTIME tmASN1;
    tmASN1.length = 9;
    auto iRet = sslHandle.ASN1_UTCTIME_ConvertToTimeT(tmASN1);
    EXPECT_EQ(iRet, 0);
    
    tmASN1.length = 11;
    tmASN1.data = (unsigned char *)"19991231000000Z";
    iRet = sslHandle.ASN1_UTCTIME_ConvertToTimeT(tmASN1);
    EXPECT_EQ(iRet, 0);

    tmASN1.data = (unsigned char *)"19991231000000Z";
    iRet = sslHandle.ASN1_UTCTIME_ConvertToTimeT(tmASN1);
    EXPECT_EQ(iRet, 0);
}

/*
 * 用例名称：ASN1_UTCTIME_ConvertToTimeT_Inner
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, ASN1_UTCTIME_ConvertToTimeT_Inner)
{
    DoLogTest();
    SSLHandle sslHandle;
    ASN1_UTCTIME tmASN1;
    char v[] = {'1','2','3','1','2','3','1','2','3','0','1','1'};
    tmASN1.length = 12;
    auto iRet = sslHandle.ASN1_UTCTIME_ConvertToTimeT_Inner(tmASN1, v);
    EXPECT_EQ(iRet, 11);
}

/*
 * 用例名称：ASN1_GENERALIZEDTIME_ConvertToTimeT
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, ASN1_GENERALIZEDTIME_ConvertToTimeT)
{
    DoLogTest();
    SSLHandle sslHandle;
    ASN1_UTCTIME tmASN1;
    unsigned char *tempNum = (unsigned char *)"19991231000000Z";
    tmASN1.data = tempNum;
    tmASN1.length = 9;
    auto iRet = sslHandle.ASN1_GENERALIZEDTIME_ConvertToTimeT(tmASN1);
    EXPECT_EQ(iRet, 0);
    
    tmASN1.data = (unsigned char *)"19991331000000Z";
    tmASN1.length = 13;
    iRet = sslHandle.ASN1_GENERALIZEDTIME_ConvertToTimeT(tmASN1);
    EXPECT_EQ(iRet, 0);

    tmASN1.data = (unsigned char *)"19991231000000Z";
    iRet = sslHandle.ASN1_GENERALIZEDTIME_ConvertToTimeT(tmASN1);
    EXPECT_NE(iRet, 0);
}

/*
 * 用例名称：ASN1_TIME_ConvertToTimeT
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, ASN1_TIME_ConvertToTimeT)
{
    DoLogTest();
    SSLHandle sslHandle;
    ASN1_UTCTIME tmASN1;
    tmASN1.length = 9;
    auto iRet = sslHandle.ASN1_TIME_ConvertToTimeT(tmASN1);
    EXPECT_EQ(iRet, 0);

    tmASN1.type = V_ASN1_UTCTIME;
    iRet = sslHandle.ASN1_TIME_ConvertToTimeT(tmASN1);
    EXPECT_EQ(iRet, 0);
    
    tmASN1.type = V_ASN1_GENERALIZEDTIME;
    iRet = sslHandle.ASN1_TIME_ConvertToTimeT(tmASN1);
    EXPECT_EQ(iRet, 0);
}

/*
 * 用例名称：ASN1_TIME_ConvertToTimeT
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(SSLHandleTest, SSLHandleVerifyCallback)
{
    DoLogTest();
    SSLHandle sslHandle;
    ASN1_UTCTIME tmASN1;
    tmASN1.length = 9;
    auto iRet = sslHandle.ASN1_TIME_ConvertToTimeT(tmASN1);
    EXPECT_EQ(iRet, 0);
}
