/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file SSLHandle.h
 * @brief  The implemention about SSL
 * @version 1.0.0
 * @date 2014-12-6
 * @author lili 00254913
 */
#ifndef _SSLHANDLE_H_
#define _SSLHANDLE_H_

#ifndef WIN32
#include <string>
#include <time.h>
#include "openssl/ssl.h"
#include "openssl/asn1.h"
#include "openssl/bio.h"
#include "curl/curl.h"
#include "common/Defines.h"

int SSLHandle_verify_callback(int ok, X509_STORE_CTX* ctx);

enum SecuritySS_RC {
    SecuritySS_Success,                   /* 0 */
    SecuritySS_InnerWrong,                /* 1 */
    SecuritySS_WrongCredential,           /* 2 */
    Security_Cert_Invalid,                /* 3 */
    Security_Cert_Expire,                 /* 4 */
    Security_Cert_KeyAlgorithmNotSecurit, /* 5 */
    Security_Not_Meet_PwdComplex,         /* 6 */
    Security_Pwd_TooLong,                 /* 7 */
    SecuritySS_WrongVerify,               /* 8 */
    SecuritySS_ConnectFailed              /* 9 */
};

class SSLHandle {
public:
    static void InitOpenSSL();
    static void CleanupOpenSSL();
    static SecuritySS_RC CheckCert(const mp_string& certText);

    static SecuritySS_RC GetCertExpireTime(const mp_string& certText, time_t& ExpireTime);

    static SecuritySS_RC CheckWhetherCACert(const mp_string& certText, bool& isCACert);
    static SecuritySS_RC GetCertValidTime(const mp_string& certText, time_t& StartTime, time_t& ExpireTime);
    static SecuritySS_RC GetCertIssuer(const mp_string& certText, mp_string& issuer);
    static SecuritySS_RC GetCertSubject(const mp_string& certText, mp_string& subject);
    static SecuritySS_RC VerifyCertContent(
        const mp_string& hostName, const mp_string& certText, mp_string& fingerprint);
    static time_t ASN1_TIME_ConvertToTimeT(const ASN1_TIME& tm);

    static void EnrichSSL(CURL* curlCtx, const mp_string& certText);
    static SecuritySS_RC EnrichSSL(SSL_CTX* sslCtx, const mp_string& certText);
    static SecuritySS_RC ConvToPEMCert(const mp_string& certText, mp_string& pemCert);
    static SecuritySS_RC GetThumbprint(const mp_string& certText, mp_string& thumbprint);
    static SecuritySS_RC GetGeneralThumbprint(const mp_string& certText, mp_string& thumbprint,
        const mp_string& algorithm);
    static SecuritySS_RC ConvToX509(const mp_string& certText, STACK_OF(X509) * &pCerts);
    static SecuritySS_RC AddCertToCtx(SSL_CTX* ctx, const mp_string& certText);

private:
    static SecuritySS_RC CheckCertExpire(X509* pCert);
    static SecuritySS_RC checkCertCipher(X509* pCert);
    static CURLcode SslctxFunction(SSL_CTX* sslctx, const mp_string& certText);

    static time_t ASN1_GENERALIZEDTIME_ConvertToTimeT(const ASN1_GENERALIZEDTIME& tmASN1);
    static int ASN1_GENERALIZEDTIME_ConvertToTimeT_Inner(const ASN1_GENERALIZEDTIME& tm, char v[]);
    static time_t ASN1_UTCTIME_ConvertToTimeT(const ASN1_UTCTIME& tm);
    static int ASN1_UTCTIME_ConvertToTimeT_Inner(const ASN1_UTCTIME& tm, const char v[]);
    static SecuritySS_RC GetCerts(const std::string& certText, STACK_OF(X509) * pCerts, X509* pCert);
    static mp_void ConvertToTimeT_Inner(const ASN1_GENERALIZEDTIME& tm, char v[]);
};

#endif  // ! WIN32
#endif
