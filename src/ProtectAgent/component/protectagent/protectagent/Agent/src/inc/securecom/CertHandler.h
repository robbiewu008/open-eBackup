/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file CertHandler.h
 * @brief functions for handle cert
 * @version 1.0.0
 * @date 2023-11-24
 * @author hejainfan 00668904
 */
#ifndef CERT_HANDLER_H_
#define CERT_HANDLER_H_

#include "common/Defines.h"
#include "common/Utils.h"
#include "openssl/ssl.h"

class AGENT_API CertHandler {
public:
    ~CertHandler()
    {
        if (m_pSslCtx != nullptr) {
            SSL_CTX_free(m_pSslCtx);
            m_pSslCtx = nullptr;
        }
        ClearString(m_passwd);
    }
    // 加密保存秘钥的密码，此处传引用，本函数内不进行拷贝，请注意在外层进行清理
    static mp_int32 SaveCertKeyPassword(const std::string& inStr);
    // 读取并解密秘钥的密码，此处传引用，本函数内不进行拷贝，请注意在外层进行清理
    static mp_int32 ReadCertKeyPassword(std::string& outStr);
    // 校验证书密码
    mp_int32 VerifyCertKeyPassword();
private:
    SSL_CTX* m_pSslCtx {nullptr} ;
    std::string m_passwd;
};

#endif