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
#ifndef AGENT_UTILS_SECURE_H
#define AGENT_UTILS_SECURE_H

#include "common/Types.h"
#include "common/Defines.h"
#include "common/Log.h"

#include "openssl/lhash.h"
#include "openssl/crypto.h"
#include "openssl/buffer.h"
#include "openssl/x509.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

namespace SecureCom {
typedef mp_int32 (*pFunc)(mp_string);
static const mp_string SCRIPT_PACKLOG_WIN = "packlog.bat";

AGENT_API mp_int32 PackageLog(const mp_string& strLogName, bool isCollect = true);

// 获取OS类型
AGENT_API mp_void GetOSType(mp_int32& iOSType);
// 获取OS版本信息
AGENT_API mp_int32 GetOSVersion(mp_int32 iOSType, mp_string& strOSVersion);

AGENT_API mp_int32 GenScriptSign(const mp_string& strFileName, mp_string& strFileSign);
AGENT_API mp_int32 GetAdminUserInfo(mp_string& userName, mp_string& userPwd);
AGENT_API mp_int32 GenSignFile();
AGENT_API mp_void GetHostFromCert(const mp_string &certPath, mp_string &hostName);

// 非root权限执行脚本
AGENT_API mp_int32 SysExecScript(const mp_string& strScriptFileName, const mp_string& strParam,
    std::vector<mp_string> pvecResult[], mp_bool bNeedCheckSign = MP_TRUE, pFunc cb = NULL);
AGENT_API mp_int32 SysExecUserScript(const mp_string& strScriptFileName, const mp_string &strParam,
    std::vector<mp_string> pvecResult[], mp_bool bNeedCheckSign = MP_TRUE, pFunc cb = NULL);
// openssl安全函数设置
AGENT_API mp_bool CryptoThreadSetup(mp_void);
AGENT_API mp_void CryptoThreadCleanup(mp_void);
}  // namespace SecureCom

#endif
