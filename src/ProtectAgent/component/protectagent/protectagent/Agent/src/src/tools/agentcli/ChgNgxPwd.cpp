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
#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "securecom/Password.h"
#include "common/Path.h"
#include "tools/agentcli/ChgNgxPwd.h"
using namespace std;
/* ------------------------------------------------------------
Description  : 处理函数，无输入
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 ChgNgxPwd::Handle()
{
    // 校验当前管理员密码
    mp_string strUsrName;
    mp_int32 iRet = CPassword::VerifyAgentUser(strUsrName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Verify user(%s) failed.", strUsrName.c_str());
        return iRet;
    }

    // 输入证书名称和证书key名称
    mp_string strCrtFile, strCrtKeyFile, strKeyPwd;
    iRet = InputNginxInfo(strCrtFile, strCrtKeyFile, strKeyPwd);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // 修改证书名称和证书key名称
    iRet = ChgNginxInfo(strCrtFile, strCrtKeyFile, strKeyPwd);
    if (iRet != MP_SUCCESS) {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
    } else {
        printf("%s\n", OPERATION_PROCESS_SUCCESS_HINT);
        printf("Please restart agent to enable the new password.\n");
        COMMLOG(OS_LOG_INFO, "Nginx information of certificate is modified successfully.");
    }
    ClearString(strKeyPwd);

    return iRet;
}

mp_int32 ChgNgxPwd::InputNginxInfo(mp_string& strCertificate, mp_string& strKeyFile, mp_string& strNewPwd)
{
    mp_uint32 iInputFailedTimes = 0;
    mp_string strNginxFileFullPath;
    while (iInputFailedTimes <= MAX_FAILED_COUNT) {
        printf("%s\n", SET_NGINX_SSL_CRT_HINT);
        // Nginx证书名称长度不做限制
        CPassword::GetInput(SET_NGINX_SSL_CRT_HINT, strCertificate, MAX_PATH_LEN);
        strNginxFileFullPath = CPath::GetInstance().GetNginxConfFilePath(strCertificate);
        // 判断证书文件是否存在
        if (!CMpFile::FileExist(strNginxFileFullPath)) {
            printf("%s\n", OPERATION_INPUT_CRT_FAIL_HINT);
            ++iInputFailedTimes;
            continue;
        } else {
            break;
        }
    }

    if (iInputFailedTimes > MAX_FAILED_COUNT) {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
        return MP_FAILED;
    }

    iInputFailedTimes = 0;
    while (iInputFailedTimes <= MAX_FAILED_COUNT) {
        printf("%s\n", SET_NGINX_SSL_CRT_KEY_HINT);
        // Nginx证书名称长度不做限制
        CPassword::GetInput(SET_NGINX_SSL_CRT_KEY_HINT, strKeyFile, MAX_PATH_LEN);
        strNginxFileFullPath = CPath::GetInstance().GetNginxConfFilePath(strKeyFile);
        // 判断证书文件是否存在
        if (!CMpFile::FileExist(strNginxFileFullPath)) {
            printf("%s\n", OPERATION_INPUT_CRT_KEY_FAIL_HINT);
            ++iInputFailedTimes;
            continue;
        } else {
            break;
        }
    }

    if (iInputFailedTimes > MAX_FAILED_COUNT) {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
        return MP_FAILED;
    }

    // 输入新密码
    mp_string strInputPwd;
    mp_int32 iRet = CPassword::ChgPwdNoCheck(strInputPwd);
    if (iRet != MP_SUCCESS) {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
        return iRet;
    }

    EncryptStr(strInputPwd, strNewPwd);
    ClearString(strInputPwd);
    return MP_SUCCESS;
}

mp_int32 ChgNgxPwd::ChgNginxInfo(const mp_string& strCertificate,
    const mp_string& strKeyFile, const mp_string& strNewPwd)
{
    vector<mp_string> vecResult;
    mp_string strNginxConf = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConf)) {
        printf("Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE.c_str());
        COMMLOG(OS_LOG_ERROR, "Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE.c_str());
        return MP_FAILED;
    }

    mp_int32 iRet = CMpFile::ReadFile(strNginxConf, vecResult);
    if (iRet != MP_SUCCESS || vecResult.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Read nginx config file failed, iRet %d, vecResult size %d.", iRet, vecResult.size());
        return MP_FAILED;
    }

    mp_uchar iFlgReplace = 0;
    for (mp_uint32 i = 0; i < vecResult.size(); i++) {
        mp_string strTmp = vecResult[i];
        std::size_t iPosCrtFile = strTmp.find(NGINX_SSL_CRT_FILE, 0);
        if (iPosCrtFile != mp_string::npos) {
            // NGINX_SSL_CRT_FILE最后空格，去掉1
            iPosCrtFile += strlen(NGINX_SSL_CRT_FILE) - 1;
            mp_string strInsert = "   " + strCertificate + STR_SEMICOLON;
            (mp_void) vecResult[i].replace(iPosCrtFile, strTmp.length() - iPosCrtFile, strInsert);
            iFlgReplace |= 0x01;
        }

        mp_string::size_type iPosCrtKeyFile = strTmp.find(NGINX_SSL_CRT_KEY_FILE, 0);
        if (iPosCrtKeyFile != mp_string::npos) {
            // NGINX_SSL_CRT_FILE最后空格，去掉1
            iPosCrtKeyFile += strlen(NGINX_SSL_CRT_KEY_FILE) - 1;
            mp_string strInsert = "   " + strKeyFile + STR_SEMICOLON;
            (mp_void) vecResult[i].replace(iPosCrtKeyFile, strTmp.length() - iPosCrtKeyFile, strInsert);
            iFlgReplace |= 0x02;
        }
    }

    // 如果能找到iFlgReplace为7，否则找不到需要替换的全部数组则不能替换
    if (iFlgReplace != 0x03) {
        COMMLOG(OS_LOG_ERROR, "Replace nginx config file failed, iFlgReplace=%x.", iFlgReplace);
        return MP_FAILED;
    }

    iRet = CIPCFile::WriteFile(strNginxConf, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Write nginx config file failed, iRet %d, vecResult size %d.", iRet, vecResult.size());
        return MP_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, strNewPwd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "set value of ssl_key_password failed");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
