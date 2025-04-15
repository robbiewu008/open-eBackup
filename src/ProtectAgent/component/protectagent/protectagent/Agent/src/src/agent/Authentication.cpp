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
#include "agent/Authentication.h"
#include "common/Log.h"
#include "common/Types.h"
#include "common/ErrorCode.h"
#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "common/CMpThread.h"
using namespace std;

namespace security {
Authentication Authentication::m_instance;

/* ------------------------------------------------------------
Function Name: init
Description  : 初始化实例，读取密码相关文件
Others       :-------------------------------------------------------- */
mp_int32 Authentication::Init()
{
    // 从配置文件中读取用户名/密码信息
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, m_strUsr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "parse xml config failed ,key is name");
        return MP_FAILED;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_HASH_VALUE, m_strPwd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "parse xml config failed, key is hash");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: Auth
Description  : 对登录客户端进行鉴权
Others       :-------------------------------------------------------- */
EXTER_ATTACK mp_int32 Authentication::Auth(
    mp_string& strClientIP, mp_string& strUsr, mp_string& strPw, const mp_string& strClientCertDN)
{
    LOGGUARD("");
    // 从配置文件中获取鉴权类型 m_strAuthType
    mp_string strAuthType;  // 鉴权类型
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_AUTH_TYPE, strAuthType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "parse xml config failed ,key is name");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    if (strAuthType == AGENT_PASSWORD_AUTH) {
        iRet = CheckUserPwd(strClientIP, strUsr, strPw);
    } else if (strAuthType == AGENT_CERTIFICATE_AUTH) {
#ifndef HDRS
        iRet = CheckCert(strClientIP, strClientCertDN);
#else
        iRet = MP_SUCCESS;
#endif
    } else {
        COMMLOG(OS_LOG_INFO, "Client IP address %s Auth failed,authType error.", strClientIP.c_str());
        return MP_FAILED;
    }

    return iRet;
}

/* ------------------------------------------------------------
Function Name: IsLocked
Description  : 判断登录客户端是否被锁定
Others       :-------------------------------------------------------- */
mp_bool Authentication::IsLocked(const mp_string& strClientIP)
{
    CThreadAutoLock lock(&m_lockedIPListMutex);
    for (vector<locked_client_info>::iterator it = m_lockedIPList.begin(); it != m_lockedIPList.end(); ++it) {
        if (strcmp(it->strClientIP.c_str(), strClientIP.c_str()) == 0) {
            if (it->isLocked) {
                // 获取系统当前时间
                mp_uint64 ullCurrentTime = CMpTime::GetTimeSec();
                // 如果锁定时间已经超时,清除该条锁定记录
                if (ullCurrentTime - it->lockedTime >= LOCKED_TIME) {
                    (mp_void)m_lockedIPList.erase(it);
                    return MP_FALSE;
                }
            }
            return it->isLocked;
        }
    }

    return MP_FALSE;
}

/* ------------------------------------------------------------
Function Name: Check
Description  : 对用户名和密码进行校验
Others       :-------------------------------------------------------- */
mp_bool Authentication::Check(const mp_string& strUsr, const mp_string& strPwd) const
{
    // 将server端传输过来的明文密码，加盐值进行散列
    mp_string strSalt;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSalt);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get salt from xml failed.");
        return MP_FALSE;
    }

    // 支持从老版本升级
    mp_string outHashHex;
    mp_string strInput = strPwd + strSalt;
    iRet = GetSha256Hash(strInput, strInput.length(), outHashHex, SHA256_BLOCK_SIZE + 1);
    (mp_void)strInput.replace((std::size_t)0, strInput.length(), "");
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetSha256Hash failed, iRet = %d.", iRet);
        return MP_FALSE;
    }

    // 新版本均采用PBKDF2进行散列
    mp_string strOut;
    iRet = PBKDF2Hash(strPwd, strSalt, strOut);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "PBKDF2Hash failed, iRet = %d.", iRet);
        return MP_FALSE;
    }

    if ((strUsr == m_strUsr) && (outHashHex == m_strPwd || strOut == m_strPwd)) {
        return MP_TRUE;
    } else {
        COMMLOG(OS_LOG_ERROR, "Check failed, auth user: %s.", strUsr.c_str());
        return MP_FALSE;
    }
}

/* ------------------------------------------------------------
Function Name: Lock
Description  : 对客户端ip地址进行锁定，前提是当前用户名密码校验已经失败
-------------------------------------------------------- */
mp_void Authentication::Lock(const mp_string& strClientIP)
{
    CThreadAutoLock lock(&m_lockedIPListMutex);
    for (vector<locked_client_info>::iterator it = m_lockedIPList.begin(); it != m_lockedIPList.end(); ++it) {
        // 之前已存在记录
        if (strcmp(it->strClientIP.c_str(), strClientIP.c_str()) == 0) {
            mp_uint64 ullCurrentTime = CMpTime::GetTimeSec();
            // 如果已经锁定，直接返回
            if (it->isLocked) {
                COMMLOG(OS_LOG_INFO, "current client %s is locked.", it->strClientIP.c_str());
                it->lastFailedTime = ullCurrentTime;
                return;
            }

            // 如果当前登录时间距离上一次失败已经超过一段时间
            if (ullCurrentTime - it->lastFailedTime >= CONTINUOUS_FAILURE_TIME) {
                // 锁定状态重新更新
                it->failedTimes = 0;
                it->isLocked = MP_FALSE;
            }

            // 连续失败次数增加一次
            it->failedTimes++;
            if (it->failedTimes >= MAX_TRY_TIME) {
                // 超过MAX_TRY_TIME，锁定
                COMMLOG(OS_LOG_INFO,
                    "Failed trying times is over %d, %s is locked.",
                    MAX_TRY_TIME,
                    it->strClientIP.c_str());
                it->isLocked = MP_TRUE;
                it->lockedTime = ullCurrentTime;
            }
            it->lastFailedTime = ullCurrentTime;
            return;
        }
    }

    // 遍历vector，没有找到指定ip的锁定记录，重新创建一条
    locked_client_info newLockedInfo;
    newLockedInfo.failedTimes = 1;
    newLockedInfo.isLocked = MP_FALSE;
    newLockedInfo.lastFailedTime = CMpTime::GetTimeSec();
    newLockedInfo.lockedTime = 0;
    newLockedInfo.strClientIP = strClientIP;
    m_lockedIPList.push_back(newLockedInfo);
}

/* ------------------------------------------------------------
Function Name: Unlock
Description  : 对客户端ip地址解除锁定
-------------------------------------------------------- */
mp_void Authentication::Unlock(const mp_string& strClientIP)
{
    CThreadAutoLock lock(&m_lockedIPListMutex);
    for (vector<locked_client_info>::iterator it = m_lockedIPList.begin(); it != m_lockedIPList.end(); ++it) {
        // 找到记录
        if (strcmp(it->strClientIP.c_str(), strClientIP.c_str()) == 0) {
            // 清除该条记录
            (mp_void)m_lockedIPList.erase(it);
            return;
        }
    }
}

mp_int32 Authentication::CheckUserPwd(const mp_string& strClientIP, const mp_string& strUsr,
    const mp_string& strPw)
{
    // 重新从密码文件中读取，更新用户名和密码
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, m_strUsr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "parse xml config failed ,key is name");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_HASH_VALUE, m_strPwd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "parse xml config failed, key is hash");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    // 判断客户端是否被锁定
    if (IsLocked(strClientIP)) {
        COMMLOG(OS_LOG_ERROR, "Client IP address %s is locked.", strClientIP.c_str());
        // 返回对应错误码
        return ERROR_COMMON_CLIENT_IS_LOCKED;
    }

    // 对用户名密码进行校验
    mp_bool bRet = Check(strUsr, strPw);
    // 失败，对ip地址进行锁定
    if (MP_FALSE == bRet) {
        COMMLOG(OS_LOG_ERROR, "Check user %s failed.", strUsr.c_str());
        // 对满足锁定条件的ip地址进行锁定
        Lock(strClientIP);
        // 返回鉴权失败错误码
        return ERROR_COMMON_USER_OR_PASSWD_IS_WRONG;
    } else { // 成功，解除锁定
        // 对满足条件的ip地址进行解锁
        Unlock(strClientIP);
        COMMLOG(OS_LOG_INFO, "Client IP address %s is unlocked.", strClientIP.c_str());
    }

    return MP_SUCCESS;
}

mp_int32 Authentication::CheckCert(const mp_string& strClientIP, const mp_string& strClientCertDN)
{
    return MP_SUCCESS;

    mp_string strCNValue;
    mp_string strCNKey;
    vector<mp_string> vecAnalyse;
    CMpString::StrSplit(vecAnalyse, strClientCertDN, CHAR_COMMA);
    for (vector<mp_string>::iterator iter = vecAnalyse.begin(); iter != vecAnalyse.end(); ++iter) {
        std::size_t iPos = (*iter).find(AGENT_CERT_DN_KEY);
        if (iPos != mp_string::npos) {
            iPos = (*iter).find(STR_EQUAL);
            strCNKey = (*iter).substr((std::size_t)0, iPos + 1);
            strCNValue = (*iter).substr(iPos + 1);
            break;
        }
    }
    // 判断客户端是否被锁定
    if (IsLocked(strClientIP)) {
        COMMLOG(OS_LOG_ERROR, "Client IP address %s is locked.", strClientIP.c_str());
        // 返回对应错误码
        return ERROR_COMMON_CLIENT_IS_LOCKED;
    }
    if (strCNValue == AGENT_CERT_DN_VALUE) {
        // 对满足条件的ip地址进行解锁
        Unlock(strClientIP);
        COMMLOG(OS_LOG_INFO, "Client IP address %s Auth success.", strClientIP.c_str());
    } else {
        COMMLOG(OS_LOG_ERROR, "Certificate auth failed.");
        // 对满足锁定条件的ip地址进行锁定
        Lock(strClientIP);
        // 返回鉴权失败错误码
        return ERROR_COMMON_USER_OR_PASSWD_IS_WRONG;
    }
    return MP_SUCCESS;
}
}