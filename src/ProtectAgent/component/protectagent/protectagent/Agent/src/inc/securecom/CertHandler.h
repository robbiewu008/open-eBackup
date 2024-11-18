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