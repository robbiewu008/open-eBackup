#include "tools/agentcli/ChgCrtPwd.h"

#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "securecom/Password.h"
#include "common/CSystemExec.h"
#include "common/ErrorCode.h"
#include "tools/agentcli/ChgNgxPwd.h"
using namespace std;
void ChgCrtPwd::ReSetPwdMem(mp_string &strCrtOldPwd, mp_string &strCrtNewPwd, mp_string &strCrtNewEncryptPwd)
{
    ClearString(strCrtOldPwd);
    ClearString(strCrtNewPwd);
    ClearString(strCrtNewEncryptPwd);
}

/* ------------------------------------------------------------
Description  : 处理函数，无输入
------------------------------------------------------------- */
mp_int32 ChgCrtPwd::Handle()
{
    // 校验当前管理员密码
    mp_string strUsrName;
    mp_int32 iRet = CPassword::VerifyAgentUser(strUsrName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Verify user(%s) failed.", strUsrName.c_str());
        return iRet;
    }

    mp_string strCrtOldPwd;
    iRet = InputCrtOldPwd(strCrtOldPwd, strUsrName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get input old password failed");
        return iRet;
    }

    mp_string strCrtNewPwd;
    mp_uint32 iInputFailedTimes = 0;
    while (iInputFailedTimes <= MAX_FAILED_COUNT) {
        iRet = InputCrtNewPwd(strCrtNewPwd, strCrtOldPwd);
        if (iRet == MP_FAILED) {
            COMMLOG(OS_LOG_ERROR, "Get input new password failed");
            iInputFailedTimes++;
            continue;
        } else if (iRet == CHECK_PASSWORD_OVER_TIMES) {
            printf("Input invalid password over 3 times.\n%s\n", OPERATION_PROCESS_FAIL_HINT);
            return MP_FAILED;
        } else {
            break;
        }
    }

    if (iInputFailedTimes > MAX_FAILED_COUNT) {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }

    iRet = ChangeCrtPwd(strCrtNewPwd, strCrtOldPwd);
    if (iRet != MP_SUCCESS) {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
        return iRet;
    }

    mp_string strCrtNewEncryptPwd;
    EncryptStr(strCrtNewPwd, strCrtNewEncryptPwd);

    iRet = CConfigXmlParser::GetInstance().SetValue(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, strCrtNewEncryptPwd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "set value of ssl_key_password failed");
        return MP_FAILED;
    }

    printf("%s\nPlease restart agent to enable the new password.\n", OPERATION_PROCESS_SUCCESS_HINT);
    COMMLOG(OS_LOG_INFO, "Nginx information of certificate is modified successfully.");
    ReSetPwdMem(strCrtOldPwd, strCrtNewPwd, strCrtNewEncryptPwd);

    return MP_SUCCESS;
}

mp_int32 ChgCrtPwd::InputCrtOldPwd(mp_string& strCrtOldPwd, const mp_string& strUserName)
{
    mp_uint32 iInputPwdTimes = 0;
    mp_int32 iRet = 0;
    mp_string strOldDecryptPwd, strOldEncryptPwd, strInputOldPwd;

    if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueString(CFG_MONITOR_SECTION, CFG_NGINX_SECTION,
        CFG_SSL_KEY_PASSWORD, strOldEncryptPwd)) {
        COMMLOG(OS_LOG_ERROR, "Get GetValueString of ssl_key_password failed");
        return MP_FAILED;
    }
    DecryptStr(strOldEncryptPwd, strOldDecryptPwd);

    while (iInputPwdTimes <= MAX_FAILED_COUNT) {
        CPassword::InputUserPwd(strUserName, strInputOldPwd, INPUT_SNMP_OLD_PWD, -1);

        iRet = strOldDecryptPwd.compare(strInputOldPwd.c_str());
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Input old Password is wrong.");
            printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            iInputPwdTimes++;
            continue;
        } else {
            COMMLOG(OS_LOG_INFO, "Input old Password success.");
            break;
        }
    }

    if (iInputPwdTimes > MAX_FAILED_COUNT) {
        printf("Input invalid password over 3 times.\n");
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
        return MP_FAILED;
    }

    // 清除解密信息
    ClearString(strOldDecryptPwd);
    ClearString(strCrtOldPwd);

    return MP_SUCCESS;
}

mp_int32 ChgCrtPwd::InputCrtNewPwd(mp_string& strCrtNewPwd, mp_string &strCrtOldPwd)
{
    mp_uint32 uiInputFailedTimes = 0;
    mp_string strUsrName;
    mp_string strNewPwd;
    mp_int32 iRet = 0;

    // 输入新密码
    while (uiInputFailedTimes <= MAX_FAILED_COUNT) {
        CPassword::InputUserPwd(strUsrName, strNewPwd, INPUT_DEFAULT, -1);
        iRet = strCrtOldPwd.compare(strNewPwd.c_str());
        if (iRet == MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Input new Password is wrong.");
            printf("The new password cann't be same as old password.\n");
            uiInputFailedTimes++;
            continue;
        }

        iRet = CPassword::CheckCommon(strNewPwd);
        if (iRet != MP_TRUE) {
            COMMLOG(OS_LOG_ERROR, "Input new Password is irregular.");
            uiInputFailedTimes++;
            continue;
        } else {
            break;
        }
    }

    if (uiInputFailedTimes > MAX_FAILED_COUNT) {
        COMMLOG(OS_LOG_ERROR, "Input invalid password over 3 times.");
        return CHECK_PASSWORD_OVER_TIMES;
    }

    mp_string strConfirmedPwd;
    CPassword::InputUserPwd(strUsrName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD, -1);
    if (strConfirmedPwd != strNewPwd) {
        printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
        return MP_FAILED;
    }

    strCrtNewPwd = std::move(strNewPwd);

    return MP_SUCCESS;
}

mp_int32 ChgCrtPwd::ChangeCrtPwd(mp_string &strCrtNewPwd, mp_string &strCrtOldPwd)
{
    mp_string strCrtFileName;
    mp_int32 iRet = GetCertFileName(strCrtFileName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get cert file name failed");
        return MP_FAILED;
    }
    mp_string strCrtFilePath = CPath::GetInstance().GetNginxConfFilePath(strCrtFileName);
    strCrtFilePath = CMpString::BlankComma(strCrtFilePath);

    mp_string strOpensslPath = CPath::GetInstance().GetBinFilePath(OPENSSL_FILE_NAME);
    strOpensslPath = CMpString::BlankComma(strOpensslPath);

    mp_string strNewCrtFilePath = CPath::GetInstance().GetNginxPath() +
        PATH_SEPARATOR + AGENT_NGINX_CONF + PATH_SEPARATOR + CERT_NEW_NAME;
    strNewCrtFilePath = CMpString::BlankComma(strNewCrtFilePath);

    mp_string strCmd = strOpensslPath + " rsa -in " + strCrtFilePath + " -out " + strNewCrtFilePath +
             " -aes256 -passin pass:" + strCrtOldPwd + " -passout pass:" + strCrtNewPwd;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
#ifdef WIN32
    mp_string strNginxConfFilePath = CPath::GetInstance().GetConfFilePath(OPENSSL_CONF_FILE_NAME);
    mp_string strEnv = "OPENSSL_CONF=" + strNginxConfFilePath;
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd, strEnv, 1);
    if (iRet != MP_SUCCESS) {
        iRet = CMpFile::DelFile(strNewCrtFilePath);
        if (iRet != 0) {
            COMMLOG(OS_LOG_ERROR, "Remove cert file failed, errno[%d]:%s.", errno, strerror(errno));
            return MP_FAILED;
        }
        return MP_FAILED;
    }
#else
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (iRet != MP_SUCCESS) {
        iRet = CMpFile::DelFile(strNewCrtFilePath);
        if (iRet != 0) {
            COMMLOG(OS_LOG_ERROR, "Remove cert file failed, errno[%d]:%s.", errno, strerror(errno));
            return MP_FAILED;
        }
        return MP_FAILED;
    }
#endif
    iRet = BuildCertFile(strCrtFileName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Rebuild cert file failed");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Change cert file password success");
    return MP_SUCCESS;
}

mp_int32 ChgCrtPwd::GetCertFileName(mp_string& strCrtFileName)
{
    mp_string strNginxConfFilePath = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    mp_string strTmp, strLine;
    vector<mp_string> vecResult;
    vector<mp_string> lstStrName;
    vector<mp_string> lstStrNameTmp;
    mp_int32 iRet = CMpFile::ReadFile(strNginxConfFilePath, vecResult);
    if (iRet != MP_SUCCESS || vecResult.size() == 0) {
        COMMLOG(OS_LOG_ERROR,
            "Read nginx config file failed, iRet = %d, size of vecResult is %d.",
            iRet,
            vecResult.size());
        return MP_FAILED;
    }

    mp_string::size_type iPosCrtFile;
    for (mp_uint32 i = 0; i < vecResult.size(); i++) {
        strTmp = vecResult[i];
        iPosCrtFile = strTmp.find(NGINX_SSL_CRT_KEY_FILE, 0);
        if (iPosCrtFile != mp_string::npos) {
            strLine = strTmp;
            CMpString::StrSplit(lstStrNameTmp, strTmp, ';');
            if (lstStrNameTmp.empty()) {
                continue;
            }

            strTmp = lstStrNameTmp.front();
            CMpString::StrSplit(lstStrName, strTmp, ' ');
            if (lstStrName.empty()) {
                continue;
            }
            strCrtFileName = lstStrName.back();
            break;
        }
    }

    if (!strCrtFileName.empty()) {
        COMMLOG(OS_LOG_INFO, "Get cert file name success.");
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "Get cert file name failed");
        return MP_FAILED;
    }
}

mp_int32 ChgCrtPwd::BuildCertFile(const mp_string& strNewCrtFileName)
{
    vector<mp_string> vecResult;
    mp_int32 iRet;
    mp_string strNginxFilePath, strNewCrtFilePath, strCrtFilePath;
    strNginxFilePath = CPath::GetInstance().GetNginxPath();
    strNewCrtFilePath = strNginxFilePath + PATH_SEPARATOR + AGENT_NGINX_CONF + PATH_SEPARATOR + CERT_NEW_NAME;
    strCrtFilePath = CPath::GetInstance().GetNginxConfFilePath(strNewCrtFileName);
    CHECK_FAIL_EX(CheckCmdDelimiter(strCrtFilePath));

    iRet = CIPCFile::ReadFile(strNewCrtFilePath, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Read cert file failed, errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }

    iRet = CMpFile::DelFile(strCrtFilePath);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "Remove cert file failed, errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }

    iRet = CIPCFile::WriteFile(strCrtFilePath, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "write cert file failed, errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }
#ifdef WIN32
    mp_string strCommand = "cmd.exe /c echo Y | cacls.exe \"" + strCrtFilePath + "\" /E /R Users > c:\\nul";

    iRet = CSystemExec::ExecSystemWithoutEcho(strCommand);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "cacls cert file failed %d.", iRet);

        return MP_FAILED;
    }
#else
    if (ChmodFile(strCrtFilePath, S_IRUSR) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "chmod crt file failed.");
        return MP_FAILED;
    }
#endif

    COMMLOG(OS_LOG_INFO, "Recover cert file name success");
    return MP_SUCCESS;
}
