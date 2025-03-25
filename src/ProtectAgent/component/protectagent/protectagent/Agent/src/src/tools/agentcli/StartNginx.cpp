#include "tools/agentcli/StartNginx.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/Ip.h"
#include "common/Pipe.h"
#include "common/CMpThread.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "securecom/Password.h"
#include "securecom/CryptAlg.h"
#include "securecom/RootCaller.h"
#include "host/host.h"
using namespace std;

namespace {
const mp_string START_NGINX_MODE = "startnginx";    // 重启nginx
const mp_string RELOAD_NGINX_MODE = "reloadnginx";  // 重载nginx配置文件
}  // namespace

mp_string StartNginx::m_startNginxMode = START_NGINX_MODE;

mp_int32 StartNginx::GetPassword(mp_string& CipherStr)
{
    if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueString(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, CipherStr)) {
        COMMLOG(OS_LOG_ERROR, "get GetValueString of ssl_key_password failed");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 StartNginx::ExecNginxStart()
{
#ifdef WIN32
    // 启动Nginx
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(PROCESS_START_SCRIPT);
    // 校验脚本签名
    mp_int32 iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init crypt failed, ret = %d.", iRet);
        return iRet;
    }
    // 程序即将退出，此处不判断返回值
    (mp_void) FinalizeCrypt();

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Script sign check failed, script name is \"%s\", iRet = %d.", PROCESS_START_SCRIPT, iRet);
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    if (getuid() == 0) {
        strCmd = "su - rdadmin -s /bin/sh -c \" " + strCmd + " \" ";
    }
    COMMLOG(OS_LOG_DEBUG, "execute :%s", strCmd.c_str());
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif
    return MP_SUCCESS;
}

mp_int32 StartNginx::ReloadNginx()
{
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(RELOAD_NGINX_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    COMMLOG(OS_LOG_INFO, "execute :%s", strCmd.c_str());
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    return MP_SUCCESS;
}

#ifdef WIN32
int StartNginx::SetEnvWithWin(mp_string& envVal)
{
    errno_t err = _putenv_s("SSL_PWD", envVal.c_str());
    if (err != 0) {
        COMMLOG(OS_LOG_ERROR, "put env failed, errno is %d", err);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
#endif

mp_int32 StartNginx::Handle(const mp_string& actionType)
{
    if (actionType == RELOAD_NGINX_MODE) {
        // 重载nginx配置文件
        m_startNginxMode = RELOAD_NGINX_MODE;
    }

    mp_int32 iRet = MP_SUCCESS;
    // 设置防火墙
#ifndef WIN32
    mp_string strParam;
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ADD_FRIEWALL, strParam, NULL);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to configure firewalls.");
        printf("Failed to configure firewalls.\n");
        return MP_FAILED;
    }
#endif

    mp_string inStr;
    if (MP_SUCCESS != GetPassword(inStr)) {
        COMMLOG(OS_LOG_ERROR, "get encryptStr failed");
        printf("Process nginx of DataBackup ProtectAgent was started failed.\n");
        return MP_FAILED;
    }

    mp_string outStr;
    DecryptStr(inStr, outStr);
    if (outStr.empty()) {
        COMMLOG(OS_LOG_ERROR, "DecryptStr nginx password failed.");
        return MP_FAILED;
    }

    vector<mp_string> vecResult;
    vecResult.push_back(outStr.c_str());

mp_string strNginxPWDFile = CPath::GetInstance().GetNginxConfFilePath(SSL_PASSWORD_TEMP_FILE);
#ifdef WIN32
    if (CMpFile::CreateFile(strNginxPWDFile) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create file '%s' failure.", strNginxPWDFile.c_str());
        return MP_FAILED;
    }
    iRet = NginxStartHandle("SSL_PWD", vecResult);
#else
    iRet = NginxStartHandle(strNginxPWDFile, vecResult);
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "NginxStartHandle failed, ret is %d", iRet);
        printf("Process nginx of DataBackup ProtectAgent was started failed.\n");
        return MP_FAILED;
    }

    ClearString(outStr);
    vecResult.clear();
    return MP_SUCCESS;
}

#ifdef WIN32
DWORD WINAPI StartNginx::WriteParamFunc(void* pPipe)
#else
mp_void* StartNginx::WriteParamFunc(void* pPipe)
#endif
{
    LOGGUARD("");
    CMpPipe* pWritePipe = static_cast<CMpPipe*>(pPipe);
    if (!pWritePipe) {
        COMMLOG(OS_LOG_ERROR, "cast to CMpPipe pointer failed.");
        CMPTHREAD_RETURN;
    }
    std::vector<mp_string> vecInput;
    pWritePipe->GetVecInput(vecInput);
    if (pWritePipe->WritePipe(pWritePipe->GetStrInput(), vecInput) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "write pipe failed.");
    }
    delete pWritePipe;
    pWritePipe = NULL;
    CMPTHREAD_RETURN;
}

#ifdef WIN32
mp_int32 StartNginx::NginxStartHandle(const mp_string& strNginxPWDFile, vector<mp_string>& vecInput)
{
    if (MP_FAILED == SetEnvWithWin(vecInput.back())) {
        COMMLOG(OS_LOG_ERROR, "write password to File failed");
        printf("Process nginx of DataBackup ProtectAgent was started failed.\n");
        return MP_FAILED;
    }

    mp_int32 iRet;
    if (m_startNginxMode == START_NGINX_MODE) {
        iRet = ExecNginxStart();
    } else {
        iRet = ReloadNginx();
    }

    mp_string envVal = "";
    if (MP_SUCCESS != iRet) {
        iRet = CMpFile::DelFile(strNginxPWDFile);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_INFO, "Delete tmp file failed, ret is %d", iRet);
        }
        iRet = SetEnvWithWin(envVal);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_INFO, "reset env value failed, ret is %d", iRet);
        }
        COMMLOG(OS_LOG_ERROR, "start nginx failed");
        return MP_FAILED;
    }

    iRet = CMpFile::DelFile(strNginxPWDFile);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Delete tmp file failed, ret is %d", iRet);
        return MP_FAILED;
    }
    iRet = SetEnvWithWin(envVal);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "reset env value failed, ret is %d", iRet);
    }

    COMMLOG(OS_LOG_INFO, "start nginx success");
    printf("Process nginx of DataBackup ProtectAgent was started successfully.\n");
    return MP_SUCCESS;
}
#else
mp_int32 StartNginx::NginxStartHandle(const mp_string& strNginxPWDFile, vector<mp_string>& vecInput)
{
    thread_id_t tid;
    CMpPipe* pipe = new (std::nothrow) CMpPipe();
    if (pipe == NULL) {
        COMMLOG(OS_LOG_ERROR, "new pipe failed.");
        return MP_FAILED;
    }
    pipe->SetStrInput(strNginxPWDFile);
    pipe->SetVecInput(vecInput);
    if (pipe->CreatePipe(strNginxPWDFile) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create Pipe failed.");
        return MP_FAILED;
    }
    mp_int32 iRet = CMpThread::Create(&tid, WriteParamFunc, (mp_void*)pipe);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Create write thread failed, ret %d.", iRet);
        delete pipe;
        pipe = NULL;
        return MP_FAILED;
    }

    if (m_startNginxMode == START_NGINX_MODE) {
        iRet = ExecNginxStart();
    } else {
        iRet = ReloadNginx();
    }

    if (MP_SUCCESS != iRet) {
        iRet = CMpFile::DelFile(strNginxPWDFile);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_INFO, "Delete tmp file failed, ret is %d", iRet);
        }
        COMMLOG(OS_LOG_ERROR, "start nginx failed");
        return MP_FAILED;
    }

    iRet = CMpFile::DelFile(strNginxPWDFile);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Delete tmp file failed, ret is %d", iRet);
        return MP_FAILED;
    }

    if (tid.os_id != 0) {
        CMpThread::WaitForEnd(&tid, NULL);
    }

    COMMLOG(OS_LOG_INFO, "start nginx success");
    printf("Process nginx of DataBackup ProtectAgent was started successfully.\n");
    return MP_SUCCESS;
}
#endif