/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file CertHandler.cpp
 * @brief  functions for handle cert
 * @version 1.0.0
 * @date 2023-11-24
 * @author hejainfan 00668904
 */

#include "common/Defines.h"
#include "common/Path.h"
#include "securecom/CryptAlg.h"
#include "common/ConfigXmlParse.h"

#include "securecom/CertHandler.h"

namespace {
const mp_string CAINFO = "agentca.pem";
const mp_string SSLCERT = "server.pem";
const mp_string SSLKEY = "server.key";
}

mp_int32 CertHandler::SaveCertKeyPassword(const std::string& inStr)
{
    mp_string ciPherStr = "";
    EncryptStr(inStr, ciPherStr);
    if (ciPherStr.empty()) {
        COMMLOG(OS_LOG_ERROR, "Encrypt str result is empty.");
        return MP_FAILED;
    }

    if (CConfigXmlParser::GetInstance().SetValue(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, ciPherStr) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Save ssl_key_password failed.");
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_INFO, "Save ssl_key_password success.");
    return MP_SUCCESS;
}

mp_int32 CertHandler::ReadCertKeyPassword(std::string& outStr)
{
    mp_string ciPherStr;
    if (CConfigXmlParser::GetInstance().GetValueString(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, ciPherStr) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "get GetValueString of ssl_key_password failed.");
        return MP_FAILED;
    }

    DecryptStr(ciPherStr, outStr);
    if (outStr.empty()) {
        COMMLOG(OS_LOG_ERROR, "Decrypt str result is empty.");
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, "Read Cert Key Password success.");
    return MP_SUCCESS;
}

mp_int32 CertHandler::VerifyCertKeyPassword()
{
    if (ReadCertKeyPassword(m_passwd) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Read Cert Key Password failed.");
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, "Read Cert Key Password successfully");
    // 初始化openssl和载入所有ssl错误信息
    OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, NULL);

    // 载入所有SSL算法
    OpenSSL_add_all_algorithms();

    // 设置安全会话环境
    m_pSslCtx = SSL_CTX_new(TLSv1_2_client_method());
    if (!m_pSslCtx) {
        COMMLOG(OS_LOG_ERROR, "Init client ssl context failed.");
        return MP_FAILED;
    }

    /* 设置证书密码 */
    SSL_CTX_set_default_passwd_cb_userdata(m_pSslCtx, (void*)m_passwd.c_str());

    /* 设置信任根证书 */
    mp_string caInfoPath = CPath::GetInstance().GetNginxConfFilePath(CAINFO);
    mp_int32 ret = SSL_CTX_load_verify_locations(m_pSslCtx, caInfoPath.c_str(), NULL);
    if (ret <= 0) {
        COMMLOG(OS_LOG_ERROR, "Failed to set the trust root certificate.");
        return MP_FAILED;
    }

    /* 载入用户的数字证书 */
    mp_string sslCertPath = CPath::GetInstance().GetNginxConfFilePath(SSLCERT);
    ret = SSL_CTX_use_certificate_file(m_pSslCtx, sslCertPath.c_str(), SSL_FILETYPE_PEM);
    if (ret <= 0) {
        COMMLOG(OS_LOG_ERROR, "Load the user's digital certificate failed.");
        return MP_FAILED;
    }

    /* 载入用户的私钥文件 */
    mp_string sslKeyPath = CPath::GetInstance().GetNginxConfFilePath(SSLKEY);
    ret = SSL_CTX_use_PrivateKey_file(m_pSslCtx, sslKeyPath.c_str(), SSL_FILETYPE_PEM);
    if (ret <= 0) {
        COMMLOG(OS_LOG_ERROR, "Load the user private key failed.");
        return MP_FAILED;
    }

    /* 检查用户私钥是否正确 */
    if (SSL_CTX_check_private_key(m_pSslCtx) == 0) {
        COMMLOG(OS_LOG_ERROR, "The user private key is incorrect.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
