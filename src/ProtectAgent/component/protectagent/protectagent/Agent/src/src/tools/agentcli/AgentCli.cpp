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
#include <vector>
#include <algorithm>
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "securecom/CryptAlg.h"
#include "securecom/Password.h"
#include "securecom/SecureUtils.h"
#include "common/File.h"
#include "common/Utils.h"
#include "openssl/ssl.h"
#include "tools/agentcli/ShowStatus.h"
#include "tools/agentcli/ChgSnmp.h"
#include "tools/agentcli/ChgNgxPwd.h"
#include "tools/agentcli/CollectLog.h"
#include "tools/agentcli/ChangeIP.h"
#include "tools/agentcli/ChgHostSN.h"
#include "tools/agentcli/StartNginx.h"
#include "tools/agentcli/ChgCrtPwd.h"
#include "tools/agentcli/RegExtMk.h"
#include "tools/agentcli/RegisterHost.h"
#include "tools/agentcli/ReRegisterHost.h"
#include "tools/agentcli/TestHost.h"
#include "tools/agentcli/UnixTimeStamp.h"
#include "tools/agentcli/ReadPipe.h"
#include "tools/agentcli/MergeConfFiles.h"
#ifdef WIN32
#include "tools/agentcli/WinMountCifs.h"
#include <windows.h>
#include <lm.h>
#pragma comment(lib, "netapi32.lib")
#else
#include <pwd.h>
#include "array/array.h"
#include "tools/agentcli/GetFileFromArchive.h"
#endif
using namespace std;
namespace CMD_PARAM {
const mp_uchar CMD_PARAM_NUM = 2;
const mp_uchar CMD_PARAM_NUM_DEVICE = 3;
const mp_uchar CMD_PARAM_NUM_ENC_KEY1 = 5;
const mp_uchar CMD_PARAM_NUM_ENC_KEY2 = 4;
const mp_uchar CMD_PARAM_NUM_ENC_KEY3 = 3;
const mp_uchar CMD_PARAM_NUM_DPA_USER = 3;
const mp_uchar CMD_PARAM_NUM_REGISTER_HOST_1 = 3;
const mp_uchar CMD_PARAM_NUM_REGISTER_HOST_2 = 5;
const mp_uchar CMD_PARAM_NUM_SETUSER = 4;
}  // namespace CMD_PARAM

namespace {
const mp_string SET_USER = "setuser";
const mp_string CHG_PWD = "chgpwd";
const mp_string SHOW_STATUS = "show";
const mp_string CHG_SNMP = "chgsnmp";
const mp_string CHG_NGX_PWD = "chgkey";
const mp_string GEN_SECONDS = "genseconds";
const mp_string COLLECT_LOG = "collectlog";
const mp_string CHANGE_IP = "chgbindip";
const mp_string CHG_HOST_SN = "chghostsn";
const mp_string START_NGINX = "startnginx";
const mp_string RELOAD_NGINX = "reloadnginx";
const mp_string CHG_CRT_PWD = "chgcrtpwd";
const mp_string REPORT_LUN = "reportlun";  // get map lun id list by hostnum channelnum targetnum
const mp_string GETWWN = "getwwn";         // get wwn  by device name
const mp_string ENC_PWD = "encpwd";
const mp_string ENC_KEY = "enckey";
const mp_string VERIFY_KEY = "verifykey";
const mp_string REG_MK = "regmk";
const mp_string CREATE_MK = "cremk";
const mp_string DPA_USER = "dpaUser";
const mp_string REGISTER_HOST = "registerHost";
const mp_string RE_REGISTER_HOST = "reRegisterHost";
const mp_string CHECK_HOST = "testhost";
const mp_string UNIXTIMESTAMP = "UnixTimestamp";
const mp_string GET_FILE_FROM_ARCHIVE = "GetFileFromArchive";
const mp_string READ_PIPE = "ReadPipe";
const mp_string MOUNT_CIFS = "MountCifs";
const mp_string GET_HOSTNAME = "gethostname";
const mp_string CAINFO = "agentca.pem";
const mp_string SSLCERT = "server.pem";
const mp_string SSLKEY = "server.key";
const mp_string CLIENT_CRT_NAME = "/thrift/client/client.crt.pem";
const mp_string PUSH_INSTALL_OPRATE = "push";
const mp_string MANUAL_INSTALL_OPRATE = "manual";
const mp_string MERGE_CONF_FILE = "mergefile";

const std::vector<std::string> CMD_LIST_NOT_USE_KMC = {
    SHOW_STATUS, GEN_SECONDS, GET_HOSTNAME
};
}  // namespace
namespace AGENTCLI_NUM {
const mp_uchar AGENTCLI_NUM_2 = 2;
const mp_uchar AGENTCLI_NUM_3 = 3;
const mp_uchar AGENTCLI_NUM_4 = 4;
const mp_uchar AGENTCLI_NUM_5 = 5;
const mp_uchar AGENTCLI_NUM_6 = 6;
const mp_uchar AGENTCLI_NUM_7 = 7;
const mp_uchar AGENTCLI_NUM_8 = 8;
}  // namespace AGENTCLI_NUM

SSL_CTX* pSslCtx = nullptr;
namespace AGENTCLI_FUNC {
/* ------------------------------------------------------------
Description  : 显示agentcli帮助
------------------------------------------------------------- */
void PrintHelp()
{
    printf("usage:\n");
    printf("[path]agentcli %s\n", SHOW_STATUS.c_str());
    printf("[path]agentcli %s\n", CHG_PWD.c_str());
    printf("[path]agentcli %s\n", CHG_SNMP.c_str());
    printf("[path]agentcli %s\n", CHG_NGX_PWD.c_str());
    printf("[path]agentcli %s\n", COLLECT_LOG.c_str());
    printf("[path]agentcli %s\n", CHANGE_IP.c_str());
    printf("[path]agentcli %s\n", GEN_SECONDS.c_str());
    printf("[path]agentcli %s\n", CHG_HOST_SN.c_str());
    printf("[path]agentcli %s\n", CHG_CRT_PWD.c_str());
    printf("[path]agentcli %s\n", START_NGINX.c_str());
    printf("[path]agentcli %s\n", RELOAD_NGINX.c_str());
    printf("[path]agentcli %s\n", ENC_PWD.c_str());
    printf("[path]agentcli %s\n", VERIFY_KEY.c_str());
    printf("[path]agentcli %s\n", REG_MK.c_str());
    printf("[path]agentcli %s\n", CREATE_MK.c_str());
    printf("[path]agentcli %s <oldconffile_path> <newconffile_path>\n", MERGE_CONF_FILE.c_str());
    printf("[path]agentcli %s <cipherfile_name> <Output/Input> <value>\n", ENC_KEY.c_str());
    printf("[path]agentcli %s <addUserr/deleteUser>\n", DPA_USER.c_str());
    printf("[path]agentcli %s <DeleteHost>\n", REGISTER_HOST.c_str());
    printf("[path]agentcli %s <RegisterHost> <pm_ip> <pm_port>\n", REGISTER_HOST.c_str());
    printf("[path]agentcli %s\n", RE_REGISTER_HOST.c_str());
    printf("[path]agentcli %s <host_ip> <host_port> <timeout>\n", CHECK_HOST.c_str());
    printf("[path]agentcli %s <DateTime/TimeStamp> <mode>\n", UNIXTIMESTAMP.c_str());
    printf("[path]agentcli %s <PipeFileName> <timeout>\n", READ_PIPE.c_str());
#ifdef WIN32
    printf("[path]agentcli %s <CMD>\n", MOUNT_CIFS.c_str());
    printf("[path]agentcli %s\n", SET_USER.c_str());
#else
    printf("[path]agentcli %s <backup_id> <archive_ip:archive_port> <local_path> <remote_path>\n",
        GET_FILE_FROM_ARCHIVE.c_str());
#endif
}


/* ------------------------------------------------------------
Description  : 初始化SSL上下文
------------------------------------------------------------- */
mp_int32 InitSsl()
{
    if (pSslCtx == nullptr) {
        // 初始化openssl和载入所有ssl错误信息
        OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, NULL);
        // 载入所有SSL算法
        OpenSSL_add_all_algorithms();

        // 设置安全会话环境
        pSslCtx = SSL_CTX_new(TLSv1_2_client_method());
        if (!pSslCtx) {
            COMMLOG(OS_LOG_ERROR, "Init client ssl context failed.");
            return MP_FAILED;
        }
    }
    /* 设置信任根证书 */
    mp_string caInfoPath = CPath::GetInstance().GetNginxConfFilePath(CAINFO);
    mp_int32 ret = SSL_CTX_load_verify_locations(pSslCtx, caInfoPath.c_str(), NULL);
    if (ret <= 0) {
        COMMLOG(OS_LOG_ERROR, "Failed to set the trust root certificate.");
        SSL_CTX_free(pSslCtx);
        pSslCtx = nullptr;
        return MP_FAILED;
    }
    /* 载入用户的数字证书 */
    mp_string sslCertPath = CPath::GetInstance().GetNginxConfFilePath(SSLCERT);
    ret = SSL_CTX_use_certificate_file(pSslCtx, sslCertPath.c_str(), SSL_FILETYPE_PEM);
    if (ret <= 0) {
        COMMLOG(OS_LOG_ERROR, "Load the user's digital certificate failed.");
        SSL_CTX_free(pSslCtx);
        pSslCtx = nullptr;
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 Verifykey()
{
    mp_string ciPherStr;
    if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueString(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, ciPherStr)) {
        COMMLOG(OS_LOG_ERROR, "get GetValueString of ssl_key_password failed.");
        SSL_CTX_free(pSslCtx);
        pSslCtx = nullptr;
        return MP_FAILED;
    }

    mp_string outStr;
    DecryptStr(ciPherStr, outStr);
    if (outStr.empty()) {
        COMMLOG(OS_LOG_ERROR, "DecryptStr private key password failed.");
        SSL_CTX_free(pSslCtx);
        pSslCtx = nullptr;
        return MP_FAILED;
    }
    /* 设置证书密码 */
    SSL_CTX_set_default_passwd_cb_userdata(pSslCtx, (void*)outStr.c_str());

    /* 载入用户的私钥文件 */
    mp_string sslKeyPath = CPath::GetInstance().GetNginxConfFilePath(SSLKEY);
    mp_int32 ret = SSL_CTX_use_PrivateKey_file(pSslCtx, sslKeyPath.c_str(), SSL_FILETYPE_PEM);
    if (ret <= 0) {
        COMMLOG(OS_LOG_ERROR, "Load the user private key failed.");
        ClearString(outStr);
        SSL_CTX_free(pSslCtx);
        pSslCtx = nullptr;
        return MP_FAILED;
    }

    /* 检查用户私钥是否正确 */
    if (SSL_CTX_check_private_key(pSslCtx) == 0) {
        COMMLOG(OS_LOG_ERROR, "The user private key is incorrect.");
        ClearString(outStr);
        SSL_CTX_free(pSslCtx);
        pSslCtx = nullptr;
        return MP_FAILED;
    }
    ClearString(outStr);
    SSL_CTX_free(pSslCtx);
    pSslCtx = nullptr;
    return MP_SUCCESS;
}

#ifndef WIN32
/* ------------------------------------------------------------
Description  : 获取wwn
------------------------------------------------------------- */
mp_int32 GetHostLunIDInfo(const mp_string& strCmd, const mp_string& strParam)
{
    (mp_void) strCmd;
    vector<mp_int32> vecHostLunID;
    mp_int32 iRet = Array::GetHostLunIDInfo(strParam, vecHostLunID);
    if (iRet != MP_SUCCESS) {
        printf("report lun failed.\n");
        COMMLOG(OS_LOG_ERROR, "Get host lun list(%s) failed.", strParam.c_str());
        return MP_FAILED;
    } else {
        printf("Lun list length = %lu which imples %lu lun entr%s\n",
            vecHostLunID.size() * AGENTCLI_NUM::AGENTCLI_NUM_8,
            vecHostLunID.size(),
            ((vecHostLunID.size() == 1) ? "y" : "ies"));
        for (vector<mp_int32>::iterator iter = vecHostLunID.begin(); iter != vecHostLunID.end(); ++iter) {
            printf("Peripheral device addressing: lun=%d\n", *iter);
        }
        return MP_SUCCESS;
    }
}

/* ------------------------------------------------------------
Description  : 获取 lun
------------------------------------------------------------- */
mp_int32 GetLunInfo(const mp_string& strCmd, const mp_string& strParam)
{
    (mp_void) strCmd;
    mp_string strWWN;
    mp_string strLUNID;
    mp_string strDev = strParam;
    mp_int32 iRet = Array::GetLunInfo(strDev, strWWN, strLUNID);
    if (iRet != MP_SUCCESS) {
        printf("get wwn failed.\n");
        COMMLOG(OS_LOG_ERROR, "Get wwn(%s) failed.", strParam.c_str());
        return MP_FAILED;
    } else {
        printf("Designation descriptor number\n");
        printf("[0x");
        printf("%s", strWWN.c_str());
        printf("]\n");
        return MP_SUCCESS;
    }
}
#endif

/* ------------------------------------------------------------
Description  : 加密密码--有参数：加密的文件名称
------------------------------------------------------------- */
mp_int32 EncPwd(const mp_string& cipherFileName, const mp_string& obtions = "", const mp_string& strvalue = "")
{
    mp_string cipherText = "";
    mp_int32 iRet;

    if (!((strcmp(obtions.c_str(), "Output") == 0) || (strcmp(obtions.c_str(), "Input") == 0) ||
        (strcmp(obtions.c_str(), "") == 0))) {
        AGENTCLI_FUNC::PrintHelp();
        return MP_FAILED;
    }
    iRet = CIPCFile::IsValidFilename(cipherFileName);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Filename contains invalid characters.");
        return MP_FAILED;
    }

    iRet = CPassword::EncPwd(cipherText, strvalue);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Enciyption failed.");
        ClearString(cipherText);
        return MP_FAILED;
    }

    if (obtions == "Output") {
        printf("%s\n", cipherText.c_str());
        ClearString(cipherText);
        return MP_SUCCESS;
    } else {
        iRet = CIPCFile::WriteInput(cipherFileName, cipherText);
        if (MP_SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "Write ciphertext to the specified file failed.");
            ClearString(cipherText);
            return MP_FAILED;
        }
    }

    ClearString(cipherText);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 验证密码
------------------------------------------------------------- */
mp_int32 VerifyPwd()
{
    if (InitSsl() == MP_SUCCESS && Verifykey() == MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "verify private key sucess.");
        return MP_SUCCESS;
    } else {
        printf("failed");
        COMMLOG(OS_LOG_ERROR, "verify private key failed.");
        return MP_FAILED;
    }
}

#ifdef WIN32
LPWSTR stringToLpwstr(const mp_string &inStr)
{
    int len = MultiByteToWideChar(CP_ACP, 0, inStr.c_str(), -1, NULL, 0);
    LPWSTR lpwstr = new WCHAR[len];
    MultiByteToWideChar(CP_ACP, 0, inStr.c_str(), -1, lpwstr, len);
    return lpwstr;
}

mp_int32 SetUserWin(const mp_string& userName, const mp_string& obtions = "", const mp_string& userPwd = "")
{
    COMMLOG(OS_LOG_INFO, "Begin to set user %s", userName.c_str());
    
    USER_INFO_1 ui;
    DWORD dwLevel = 1;
    DWORD dwError = 0;
    NET_API_STATUS nStatus;

    ui.usri1_name = stringToLpwstr(userName); // 用户名
    mp_string pwd;
    if (obtions == MANUAL_INSTALL_OPRATE) {
        CPassword::InputUserPwd("", pwd, INPUT_PWD);
        if (!CPassword::CheckCommon(pwd)) {
            COMMLOG(OS_LOG_ERROR, "Check passwd rules failed, Ret is %d.", MP_FAILED);
            return MP_FAILED;
        }
        COMMLOG(OS_LOG_INFO, "Manual installation.");
    } else if (obtions == PUSH_INSTALL_OPRATE) {
        pwd = userPwd;
        COMMLOG(OS_LOG_INFO, "Push installation");
    } else {
        COMMLOG(OS_LOG_ERROR, "Parameter error.");
        return MP_FAILED;
    }
    ui.usri1_password = stringToLpwstr(pwd); // 密码
    mp_string ciphertext;
    EncryptStr(pwd, ciphertext);
    pwd.clear();
    if (MP_SUCCESS != CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, WORKING_USER_PASSWORD, ciphertext)) {
        COMMLOG(OS_LOG_ERROR, "Set user pwd failed!");
        return MP_FAILED;
    }

    ui.usri1_priv = USER_PRIV_USER; // 权限
    ui.usri1_home_dir = NULL;
    ui.usri1_comment = NULL;
    ui.usri1_flags = UF_SCRIPT;
    ui.usri1_script_path = NULL;

    nStatus = NetUserAdd(NULL, dwLevel, (LPBYTE)&ui, &dwError);
    if (nStatus == NERR_Success) {
        COMMLOG(OS_LOG_INFO, "User %s has been added successfully.", userName.c_str());
    } else if (nStatus == NERR_UserExists) {
        COMMLOG(OS_LOG_ERROR, "User s already exists.", userName.c_str());
        return MP_FAILED;
    } else {
        COMMLOG(OS_LOG_ERROR, "Add user failed, error status: %d", nStatus);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
#endif

mp_void GetThriftHostName()
{
    std::string hostName;
    std::string certFilePath = CPath::GetInstance().GetConfFilePath(CLIENT_CRT_NAME);
    SecureCom::GetHostFromCert(certFilePath, hostName);
    printf("%s\n", hostName.c_str());
}

mp_int32 HandleCommand(const mp_string& strCmd,
    const mp_string& strParam, const mp_string& strParam2, const mp_string& strParam3)
{
    if (strcmp(strCmd.c_str(), REGISTER_HOST.c_str()) == 0) {
        return RegisterHost::Handle(strParam, strParam2, strParam3);
    } else if (strcmp(strCmd.c_str(), RE_REGISTER_HOST.c_str()) == 0) {
        ReRegisterHost handler;
        return handler.Handle();
    } else if (strcmp(strCmd.c_str(), ENC_KEY.c_str()) == 0) {
        return EncPwd(strParam, strParam2, strParam3);
    } else if (strcmp(strCmd.c_str(), VERIFY_KEY.c_str()) == 0) {
        return VerifyPwd();
    } else if (strcmp(strCmd.c_str(), CHECK_HOST.c_str()) == 0) {
        return TestHost::Handle(strParam, strParam2, strParam3);
    } else if (strcmp(strCmd.c_str(), GET_HOSTNAME.c_str()) == 0) {
        GetThriftHostName();
        return MP_SUCCESS;
    } else if (strcmp(strCmd.c_str(), UNIXTIMESTAMP.c_str()) == 0) {
        return UnixTimeStamp::Handle(strParam, strParam2);
    } else if (strcmp(strCmd.c_str(), READ_PIPE.c_str()) == 0) {
        ReadPipe handler;
        return handler.Handle(strParam, strParam2);
    } else if (strcmp(strCmd.c_str(), MERGE_CONF_FILE.c_str()) == 0) {
        MergeConfFiles mergeConfFiles;
        return mergeConfFiles.MergeFileHandle (strParam, strParam2);
    }
#ifdef WIN32
    if (strcmp(strCmd.c_str(), MOUNT_CIFS.c_str()) == 0) {
        WinMountCifs handler;
        return handler.Handle(strParam);
    } else if (strcmp(strCmd.c_str(), SET_USER.c_str()) == 0) {
        return SetUserWin(strParam, strParam2, strParam3);
    }
#endif

    PrintHelp();
    return MP_FAILED;
}

mp_bool HandleCmd1(const mp_string& strCmd, mp_int32& iRet)
{
    mp_bool flag = MP_FALSE;
    if (strcmp(strCmd.c_str(), CHG_PWD.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = CPassword::ChgAdminPwd();
    } else if (strcmp(strCmd.c_str(), SHOW_STATUS.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = ShowStatus::Handle();
    } else if (strcmp(strCmd.c_str(), CHG_NGX_PWD.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = ChgNgxPwd::Handle();
    } else if (strcmp(strCmd.c_str(), COLLECT_LOG.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = CollectLog::Handle();
    } else if (strcmp(strCmd.c_str(), CHANGE_IP.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = ChangeIP::Handle();
    } else if (strcmp(strCmd.c_str(), GEN_SECONDS.c_str()) == 0) {
        flag = MP_TRUE;
        mp_double seconds = CMpTime::GenSeconds();
        printf("%.f\n", seconds);
        iRet = MP_SUCCESS;
    }

    return flag;
}

mp_bool HandleCmd2(const mp_string& strCmd, mp_int32& iRet)
{
    mp_bool flag = MP_FALSE;
    if (strcmp(strCmd.c_str(), CHG_HOST_SN.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = ChgHostSN::Handle();
    } else if (strcmp(strCmd.c_str(), START_NGINX.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = StartNginx::Handle(START_NGINX);
    } else if (strcmp(strCmd.c_str(), RELOAD_NGINX.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = StartNginx::Handle(RELOAD_NGINX);
    } else if (strcmp(strCmd.c_str(), CHG_CRT_PWD.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = ChgCrtPwd::Handle();
    } else if (strcmp(strCmd.c_str(), REG_MK.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = RegExtMk::Handle();
    } else if (strcmp(strCmd.c_str(), CREATE_MK.c_str()) == 0) {
        flag = MP_TRUE;
        iRet = ManualUpdateDmcKey();
    }

    return flag;
}

mp_int32 HandleCmd(const mp_string& strCmd,
    const mp_string& strParam, const mp_string& strParam2, const mp_string& strParam3, const mp_string& strParam4)
{
    // 根据输入参数分别进行处理
    mp_int32 iRet;
    if (HandleCmd1(strCmd, iRet) == MP_TRUE) {
        return iRet;
    }

    if (HandleCmd2(strCmd, iRet) == MP_TRUE) {
        return iRet;
    }

#ifndef WIN32
    if (strcmp(strCmd.c_str(), REPORT_LUN.c_str()) == 0) {
        return GetHostLunIDInfo(strCmd, strParam);
    } else if (strcmp(strCmd.c_str(), GETWWN.c_str()) == 0) {
        return GetLunInfo(strCmd, strParam);
    } else if (strcmp(strCmd.c_str(), GET_FILE_FROM_ARCHIVE.c_str()) == 0) {
        return GetFileFromArchive::Handle(strParam, strParam2, strParam3, strParam4);
    }
#endif

    return HandleCommand(strCmd, strParam, strParam2, strParam3);
}

/* --------------------------------------------------------
Function Name: CheckParam
Description  : 检查参数
-------------------------------------------------------- */
mp_bool CheckParam(mp_int32 argc, mp_char** argv)
{
    mp_bool bCheck = (argc == CMD_PARAM::CMD_PARAM_NUM) &&
                     (strcmp(argv[1], CHG_PWD.c_str()) == 0 || strcmp(argv[1], SHOW_STATUS.c_str()) == 0 ||
                         strcmp(argv[1], CHG_SNMP.c_str()) == 0 || strcmp(argv[1], CHG_NGX_PWD.c_str()) == 0 ||
                         strcmp(argv[1], COLLECT_LOG.c_str()) == 0 || strcmp(argv[1], CHANGE_IP.c_str()) == 0 ||
                         strcmp(argv[1], GEN_SECONDS.c_str()) == 0 || strcmp(argv[1], CHG_HOST_SN.c_str()) == 0 ||
                         strcmp(argv[1], START_NGINX.c_str()) == 0 || strcmp(argv[1], RELOAD_NGINX.c_str()) == 0 ||
                         strcmp(argv[1], CHG_CRT_PWD.c_str()) == 0 || strcmp(argv[1], ENC_PWD.c_str()) == 0 ||
                         strcmp(argv[1], VERIFY_KEY.c_str()) == 0 || strcmp(argv[1], REG_MK.c_str()) == 0 ||
                         strcmp(argv[1], CREATE_MK.c_str()) == 0 || strcmp(argv[1], RE_REGISTER_HOST.c_str()) == 0);
    bCheck = bCheck || ((CMD_PARAM::CMD_PARAM_NUM_ENC_KEY1 == argc) && (strcmp(argv[1], ENC_KEY.c_str())) == 0);
    bCheck = bCheck || ((CMD_PARAM::CMD_PARAM_NUM_ENC_KEY2 == argc) && (strcmp(argv[1], ENC_KEY.c_str())) == 0);
    bCheck = bCheck || ((CMD_PARAM::CMD_PARAM_NUM_ENC_KEY3 == argc) && (strcmp(argv[1], ENC_KEY.c_str())) == 0);
    bCheck = bCheck || ((CMD_PARAM::CMD_PARAM_NUM_DPA_USER == argc) && (strcmp(argv[1], DPA_USER.c_str())) == 0);
    bCheck = bCheck || ((AGENTCLI_NUM::AGENTCLI_NUM_5 == argc) && (strcmp(argv[1], CHECK_HOST.c_str()) == 0));
    bCheck = bCheck ||
             (((CMD_PARAM::CMD_PARAM_NUM_REGISTER_HOST_1 == argc) || (CMD_PARAM::CMD_PARAM_NUM_ENC_KEY2 == argc) ||
                  (CMD_PARAM::CMD_PARAM_NUM_REGISTER_HOST_2 == argc)) &&
                 (strcmp(argv[1], REGISTER_HOST.c_str()) == 0));
    bCheck = bCheck || ((CMD_PARAM::CMD_PARAM_NUM == argc) && (strcmp(argv[1], GET_HOSTNAME.c_str()) == 0));
    bCheck = bCheck || ((AGENTCLI_NUM::AGENTCLI_NUM_4 == argc) && (strcmp(argv[1], UNIXTIMESTAMP.c_str()) == 0));
    bCheck = bCheck || ((AGENTCLI_NUM::AGENTCLI_NUM_4 == argc) && (strcmp(argv[1], READ_PIPE.c_str()) == 0));
    bCheck = bCheck || ((AGENTCLI_NUM::AGENTCLI_NUM_4 == argc) && (strcmp(argv[1], MERGE_CONF_FILE.c_str()) == 0));

#ifdef WIN32
    bCheck = bCheck || ((AGENTCLI_NUM::AGENTCLI_NUM_3 == argc) && (strcmp(argv[1], MOUNT_CIFS.c_str()) == 0));
    bCheck = bCheck || ((AGENTCLI_NUM::AGENTCLI_NUM_4 == argc) && (strcmp(argv[1], SET_USER.c_str()) == 0));
    bCheck = bCheck || ((AGENTCLI_NUM::AGENTCLI_NUM_5 == argc) && (strcmp(argv[1], SET_USER.c_str()) == 0));
#else
    bCheck = bCheck || ((argc == CMD_PARAM::CMD_PARAM_NUM_DEVICE) &&
        (0 == strcmp(argv[1], REPORT_LUN.c_str()) || 0 == strcmp(argv[1], GETWWN.c_str())));
    bCheck = bCheck || ((AGENTCLI_NUM::AGENTCLI_NUM_6 == argc) &&
        (strcmp(argv[1], GET_FILE_FROM_ARCHIVE.c_str()) == 0));
#endif
    return bCheck;
}

/* --------------------------------------------------------
Function Name: CheckRole
Description  : 检查当前操作用户是不是root
Return       : 0: 成功, 其他失败.
-------------------------------------------------------- */
mp_int32 CheckRole(const mp_string& cmd)
{
#ifndef WIN32
    if (strcmp(cmd.c_str(), ENC_PWD.c_str()) != 0) {
        struct passwd* currentUser;
        // CodeDex误报，Missing Check against Null
        currentUser = getpwuid(getuid());
        if (strncmp(currentUser->pw_name, "root", strlen("root")) == 0) {
            printf("You can not execute this command as user \"root\".\n");
            return MP_FAILED;
        }
    }
#endif
    return MP_SUCCESS;
}

/* --------------------------------------------------------
Function Name: CheckIfNeedKMC
Description  : 检查是否需要使用KMC
Return       : true: 需要, false: 不需要.
-------------------------------------------------------- */
bool CheckIfNeedKMC(const mp_string& cmd)
{
    auto findResult = std::find(CMD_LIST_NOT_USE_KMC.begin(), CMD_LIST_NOT_USE_KMC.end(), cmd);
    return (findResult == CMD_LIST_NOT_USE_KMC.end()) ? true : false;
}

/* --------------------------------------------------------
Function Name: InitKMC
Description  : 初始化KMC
Return       : 0: 成功, 其他失败.
-------------------------------------------------------- */
mp_int32 InitKMC(const mp_string& cmd)
{
    mp_int32 iRet = MP_FAILED;
    if (strcmp(cmd.c_str(), ENC_PWD.c_str()) == 0) {
        mp_string kmcConfPath = CPath::GetInstance().GetConfFilePath("kmc");
        mp_string kmcStoreFile = kmcConfPath + PATH_SEPARATOR + "agentcli_store.txt";
        mp_string kmcStoreFileBak = kmcConfPath + PATH_SEPARATOR + "agentcli_stroe_bak.txt";
        mp_string kmcConfFileBak = kmcConfPath + PATH_SEPARATOR + "agentcli_config_bak.txt";
        iRet = InitCryptByFile(kmcStoreFile, kmcStoreFileBak, KMC_ROLE_TYPE_MASTER, kmcConfFileBak);
    } else {
        iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
    }

    return iRet;
}

/* --------------------------------------------------------
Function Name: InitModule
Description  : 初始化模块
Return       : 0: 成功, 其他失败.
-------------------------------------------------------- */
mp_int32 InitModule(const std::string& cmd)
{
    // 初始化配置文件模块
    mp_int32 iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF));
    if (iRet != MP_SUCCESS) {
        printf("Init conf file %s failed.\n", AGENT_XML_CONF.c_str());
        return iRet;
    }

    // 初始化日志模块
    CLogger::GetInstance().Init(AGENT_CLI_LOG_NAME.c_str(), CPath::GetInstance().GetLogPath());

    mp_uint64 currentTime = CMpTime::GetTimeSec();
    mp_uint64 lockTime = CPassword::GetLockTime();
    if (lockTime != 0) {
        if (currentTime - lockTime < LOCK_MAX_TIME) {
            printf("agentcli is locked, please try again later.\n");
            return MP_FAILED;
        } else {
            CPassword::ClearLock();
        }
    }

    if (CheckIfNeedKMC(cmd)) {
        SecureCom::CryptoThreadSetup();

        // 初始化KMC
        iRet = InitKMC(cmd);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "InitKMC failed!");
            return iRet;
        }
        COMMLOG(OS_LOG_INFO, "Init kmc success.");
    }

    return MP_SUCCESS;
}

void ReleaseModule(const std::string& cmd)
{
    if (CheckIfNeedKMC(cmd)) {
        (mp_void) FinalizeCrypt();
        SecureCom::CryptoThreadCleanup();
    }
}
}  // namespace AGENTCLI_FUNC

/* --------------------------------------------------------
Function Name: main
Description  : agentcli进程主函数
-------------------------------------------------------- */
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_bool bCheck = AGENTCLI_FUNC::CheckParam(argc, argv);
    if (!bCheck) {
        AGENTCLI_FUNC::PrintHelp();
        return MP_FAILED;
    }

    mp_string strCmd = argv[1];
    mp_int32 iRet = AGENTCLI_FUNC::CheckRole(strCmd);
    if (iRet != MP_SUCCESS) {
        printf("Check cmd(%s) role failed.", strCmd.c_str());
        return iRet;
    }

    // 初始化agentcli路径
    iRet = CPath::GetInstance().Init(argv[0]);
    if (iRet != MP_SUCCESS) {
        printf("Init agentcli path failed.");
        return iRet;
    }

    // 初始化基础模块
    iRet = AGENTCLI_FUNC::InitModule(strCmd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init module Fail.");
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Init module success.");

    // 根据输入参数分别进行处理
    mp_string strParam;
    mp_string strParam2;
    mp_string strParam3;
    mp_string strParam4;

    if (argc >= AGENTCLI_NUM::AGENTCLI_NUM_3) {
        strParam = argv[AGENTCLI_NUM::AGENTCLI_NUM_2];
    }

    if (argc >= AGENTCLI_NUM::AGENTCLI_NUM_4) {
        strParam2 = argv[AGENTCLI_NUM::AGENTCLI_NUM_3];
    }

    if (argc >= AGENTCLI_NUM::AGENTCLI_NUM_5) {
        strParam3 = argv[AGENTCLI_NUM::AGENTCLI_NUM_4];
    }
    if (argc >= AGENTCLI_NUM::AGENTCLI_NUM_6) {
        strParam4 = argv[AGENTCLI_NUM::AGENTCLI_NUM_5];
    }

    iRet = AGENTCLI_FUNC::HandleCmd(strCmd, strParam, strParam2, strParam3, strParam4);
    // 程序即将退出，此处不判断返回值
    AGENTCLI_FUNC::ReleaseModule(strCmd);

#ifndef WIN32
    (mp_void) ChangeGmonDir();  // change profile out put dir
#endif

    COMMLOG(OS_LOG_INFO, "handle cmd %s finish.", strCmd.c_str());
    return iRet;
}