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
#ifndef _AGENTCLI_CHGCRTPWD_H_
#define _AGENTCLI_CHGCRTPWD_H_

#include "common/Types.h"

static const mp_string CERT_NEW_NAME = "server_new.key";
static const mp_uchar  CHECK_PASSWORD_OVER_TIMES = 3;
#ifdef WIN32
static const mp_string OPENSSL_FILE_NAME = "openssl.exe";
static const mp_string OPENSSL_CONF_FILE_NAME = "openssl.cnf";
#else
static const mp_string OPENSSL_FILE_NAME = "openssl";
#endif

class ChgCrtPwd {
public:
    static mp_int32 Handle();

private:
    static void ReSetPwdMem(mp_string &strCrtOldPwd, mp_string &strCrtNewPwd, mp_string &strCrtNewEncryptPwd);
    static mp_int32 InputCrtOldPwd(mp_string& strCrtOldPwd, const mp_string& strUserName);
    static mp_int32 InputCrtNewPwd(mp_string& strCrtNewPwd, mp_string &strCrtOldPwd);
    static mp_int32 ChangeCrtPwd(mp_string &strCrtNewPwd, mp_string &strCrtOldPwd);
    static mp_int32 GetCertFileName(mp_string& strCrtFileName);
    static mp_int32 BuildCertFile(const mp_string& strNewCrtFileName);
};

#endif /* _AGENTCLI_CHGCRTPWD_H_ */
