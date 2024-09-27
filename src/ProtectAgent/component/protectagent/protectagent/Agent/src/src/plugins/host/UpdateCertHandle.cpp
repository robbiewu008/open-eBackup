#include "plugins/host/UpdateCertHandle.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include "common/Log.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "common/Ip.h"
#include "securecom/RootCaller.h"
#include "securecom/CryptAlg.h"
#include "securecom/CertHandler.h"
#include "securecom/SecureUtils.h"
#include "host/CheckConnectStatus.h"
#include "common/File.h"
#include "host/host.h"
#include "message/curlclient/CurlHttpClient.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "message/curlclient/PmRestClient.h"

using namespace std;

namespace {
    const mp_int32 PM_CERT_TYPE = 1;
    const mp_int32 AGENT_CERT_TYPE = 2;

    const mp_int32 CHECK_TO_PM_CONNECTION_INTERVAL_TIME = 60 * 1000;
    const mp_int32 CHECK_TO_PM_CONNECTION_RETRY_TIMES = 20;
    const mp_int32 UPDATE_AGENT_TYPE = 9;     // 更新PM证书:0;更新agent证书:9;
    const mp_int32 UPDATE_PM_TYPE = 0;
    const mp_int32 MAX_STRING_LEN_MB = 1024 * 1024;
    const mp_int32 MIN_TASKID_SIZE = 36;
    const mp_int32 MAX_TASKID_SIZE = 128;

    const mp_string AGENTCA_PEM_NAME = "agentca.pem";
    const mp_string PMCA_PEM_NAM = "pmca.pem";
    const mp_string SERVER_PEM_NAME = "server.pem";
    const mp_string SERVER_KEY_NAME = "server.key";
    const mp_string KEY_PWD_NAME = "key.pwd";
    const mp_string CERT_UPDATING_STATUS = "cert_updating";
    const mp_string PM_TO_AGENT_STATUS_OK = "pm_to_agent_status_ok";
    const mp_string SERVER_CERT_CN = "OceanProtect-AGENT";
    const vector<mp_string> VEC_CERT_FILE_NAME_AGENT = {AGENTCA_PEM_NAME, SERVER_PEM_NAME, SERVER_KEY_NAME};
    const mp_string CLEAN_PAMA = "12";    // 清理参数
    const mp_string FALLBACK_PAMA = "11";    // 回退参数
    const mp_string UPDATE_PARAM = "0";   // 更新参数

    mp_string FLAG_FILE_UPDATING =  CPath::GetInstance().GetTmpPath() + PATH_SEPARATOR + CERT_UPDATING_STATUS;
    mp_string FLAG_FILE_CONNRCT = CPath::GetInstance().GetTmpPath() + PATH_SEPARATOR + PM_TO_AGENT_STATUS_OK;

}

void UpdateCertHandle::StartAutoCheck()
{
    if (!UpdateCertHandle::GetInstance().CheckIfInUpdatingCert()) {
        INFOLOG("Handle to check before cert update.");
        return;
    }
    if (!UpdateCertHandle::GetInstance().CreateUpdateThread()) {
        ERRLOG("Create update thread failed.");
        return;
    }
    INFOLOG("Craete Update thread success.");
}

mp_int32 UpdateCertHandle::HandleUpdateRequest(CRequestMsg& req)
{
    if (CheckJobID(req) != MP_SUCCESS) {
        ERRLOG("Check Job ID Failed.");
        return MP_FAILED;
    }
    if (CheckIfExistHandle(m_jobId)) {
        INFOLOG("The same updating job already exists, jobid is: %s.", m_jobId.c_str());
        return MP_SUCCESS;
    }
    INFOLOG("Handle to check before cert update.");
    mp_int32 iRet = CheckBeforeUpdate(m_certType);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to prepare for the upgration.");
        return MP_FAILED;
    }

    if (!CreateUpdateThread()) {
        ERRLOG("Create update thread failed.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

bool UpdateCertHandle::CreateUpdateThread()
{
    if (m_execUpdateCertThread != nullptr) {
        if (m_execTheadRunningFlag) {
            ERRLOG("Update or rollback thread is running.");
            return false;
        }
        m_execUpdateCertThread->join();
        m_execUpdateCertThread.reset();
        INFOLOG("Reset thread.");
    }
    m_execUpdateCertThread = std::make_unique<std::thread>(std::bind(&UpdateCertHandle::ExecUpdateCert, this));
    if (m_execUpdateCertThread == nullptr) {
        ERRLOG("Create update cert thread failed.");
        return MP_FAILED;
    }
    return true;
}

bool UpdateCertHandle::CreateRollbackThread()
{
    if (m_execRollbackCertThread != nullptr) {
        if (m_execTheadRunningFlag) {
            ERRLOG("Rollback or updating thread is running.");
            return false;
        }
        m_execRollbackCertThread->join();
        m_execRollbackCertThread.reset();
        INFOLOG("Reset Rollback thread.");
    }

    m_execRollbackCertThread = std::make_unique<std::thread>(std::bind(&UpdateCertHandle::ExecRollbackCert, this));
    if (m_execRollbackCertThread == nullptr) {
        ERRLOG("Create Rollback cert thread failed.");
        return false;
    }
    return true;
}

bool UpdateCertHandle::CheckIfInUpdatingCert()
{
    mp_string jobId;
    if (!GetJobidInTmpFile(jobId)) {
        WARNLOG("Get jobid from temp file failed when checking.");
        return false;
    }
    DBGLOG("jobId is %s.", jobId.c_str());
    mp_string oldNewPath = CPath::GetInstance().GetTmpPath() + PATH_SEPARATOR + "cert_updating_";
    oldNewPath += jobId + PATH_SEPARATOR + "oldCert";
    if (!CMpFile::DirExist(oldNewPath.c_str())) {
        WARNLOG("Not in cert updating status.");
        return false;
    }
    
    INFOLOG("In cert updating status.");
    return true;
}

bool UpdateCertHandle::GetJobidInTmpFile(mp_string& resJobid)
{
    if (!CMpFile::FileExist(FLAG_FILE_UPDATING)) {
        INFOLOG("Not in cert updating status.");
        return false;
    }
    vector<mp_string> vecJobId;
    mp_string jobId;
    CMpFile::ReadFile(FLAG_FILE_UPDATING, vecJobId);
    for (auto i : vecJobId) {
        jobId += i;
    }
    if (jobId.empty()) {
        ERRLOG("The file(%s) is empty.", FLAG_FILE_UPDATING.c_str());
        return false;
    }
    resJobid = jobId;
    CHECK_FAIL_EX(CheckParamStringEnd(jobId, MIN_TASKID_SIZE, MAX_TASKID_SIZE));
    return true;
}

// Cenectivity Check
void UpdateCertHandle::CheckNewCertConnectivity()
{
    CertHandler certHandler;
    if (certHandler.VerifyCertKeyPassword() != MP_SUCCESS) {
        ERRLOG("Verify cert key password fail.");
        return;
    }
    INFOLOG("Verify cert key password success.");

    WaitPmNotify();
}

// 请求PM网络接口
mp_int32 UpdateCertHandle::CheckConnectionToPM()
{
    INFOLOG("Start reporter to PM.");
    HttpResponse httpResponse;
    HttpReqCommonParam httpParam("GET", "/v1/host-agent/connection/check", "");
    if (PmRestClient::GetInstance().SendRequest(httpParam, httpResponse) != MP_SUCCESS) {
        ERRLOG("Check connection to PM failed.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

void UpdateCertHandle::ExecUpdateCert()
{
    m_execTheadRunningFlag = true;
    INFOLOG("Handle the update cert.");
    if (!CheckIfInUpdatingCert()) {
        // before restart, do update will restart
        mp_int32 iRet = UpdateCert();
        if (iRet != MP_SUCCESS) {
            ERRLOG("Failed to excute upgrade script.");
        }
    } else {
        // after restart, need check connect pm2agent
        if (CheckPmToAgentStatus()) {
            m_connectPmToAgent = true;
        }
    }

    CheckNewCertConnectivity();

    if (m_connectPmToAgent && m_connectAgentToPm) {
        INFOLOG("Check connect status success.");
    } else {
        ERRLOG("Time out! Check connect status failed.");
        ExecRollbackCert();
    }
    m_execTheadRunningFlag = false;
    return;
}

mp_int32 UpdateCertHandle::PushNewCert(CRequestMsg& req)
{
    mp_int32 iRet = CheckRequest(req);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Check requrst failed.");
        return MP_FAILED;
    }
    if (CheckIfExistHandle(m_jobId)) {
        INFOLOG("The same task already exists, jobid is: %s.", m_jobId.c_str());
        return MP_SUCCESS;
    }

    iRet = InitailizePath();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Initailize cert files path failed.");
        return MP_FAILED;
    }

    iRet = ParseFile();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Check Cert failed.");
        return MP_FAILED;
    }

    if (m_certType == AGENT_CERT_TYPE) {
        if (CheckIfSafe() != MP_SUCCESS) {
            ERRLOG("Checking cert security failed.");
            return MP_FAILED;
        }

        if (!AddCertCNToHosts()) {
            ERRLOG("Add cert cn to hosts failed.");
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}

// 当前使用openssl3.0，若后续openssl升级到其他版本，此处openssl库中的函数可能需要改动
mp_int32 UpdateCertHandle::CheckIfSafe()
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());
    if (!ctx) {
        ERRLOG("SSL_CTX_new error.");
        return MP_FAILED;
    }

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        ERRLOG("SSL_new error.");
        SSL_CTX_free(ctx);
        return MP_FAILED;
    }

    // Load the PEM certificate file
    if (SSL_CTX_use_certificate_file(ctx, (m_certPathNew + SERVER_PEM_NAME).c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERRLOG("Error loading certificate file: %s", (m_certPathNew + SERVER_PEM_NAME).c_str());
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        return MP_FAILED;
    }

    // Check if the specified ciphers are supported
    bool cipher1Supported = checkCipherSupport(ssl, "ECDHE-RSA-AES256-GCM-SHA384");
    bool cipher2Supported = checkCipherSupport(ssl, "ECDHE-RSA-AES128-GCM-SHA256");
    if (cipher1Supported || cipher2Supported) {
        INFOLOG("Certificate supports one or more ciphers.\n");
    } else {
        ERRLOG("Certificate does not support ciphers.\n");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        return MP_FAILED;
    }

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return MP_SUCCESS;
}

bool UpdateCertHandle::checkCipherSupport(SSL* ssl, const char* cipherName)
{
    if (ssl == NULL) {
        ERRLOG("The ssl is null.");
        return false;
    }
    STACK_OF(SSL_CIPHER)* ciphers_list = SSL_get1_supported_ciphers(ssl);
    if (!ciphers_list) {
        ERRLOG("Error getting supported ciphers.");
        return false;
    }

    bool foundFlag = false;
    for (int i = 0; i < sk_SSL_CIPHER_num(ciphers_list); ++i) {
        const SSL_CIPHER* cipher = sk_SSL_CIPHER_value(ciphers_list, i);
        if (std::string(cipherName) == SSL_CIPHER_get_name(cipher)) {
            foundFlag = true;
            break;
        }
    }
    sk_SSL_CIPHER_free(ciphers_list);
    return foundFlag;
}

mp_int32 UpdateCertHandle::InitailizePath()
{
    INFOLOG("Start to create file: %s!", FLAG_FILE_UPDATING.c_str());
    if (CMpFile::FileExist(FLAG_FILE_UPDATING)) {
        INFOLOG("The file(%s) exists, start to clean temp cert updating files!", FLAG_FILE_UPDATING.c_str());
        if (ExecCleanTempFiles() != MP_SUCCESS) {
            ERRLOG("Clean Temp File failed!.");
            return MP_FAILED;
        }
    }
    if (WriteInFile(FLAG_FILE_UPDATING, m_jobId) != MP_SUCCESS) {
        ERRLOG("Create file %s successfully.", FLAG_FILE_UPDATING.c_str());
        return MP_FAILED;
    }

    m_certUpdateDir = CPath::GetInstance().GetTmpPath() + PATH_SEPARATOR +"cert_updating_";
    // Use jobid name directory to distinguish between different update jobs.
    m_certUpdateDir += m_jobId + PATH_SEPARATOR;

    m_certPathNew = m_certUpdateDir + "newCert" + PATH_SEPARATOR;
    m_certPathOld = m_certUpdateDir + "oldCert" + PATH_SEPARATOR;

    return MP_SUCCESS;
}

mp_int32 UpdateCertHandle::CheckRequest(CRequestMsg& req)
{
    INFOLOG("Check request of pushing cert files.");
    const Json::Value& jReqBody = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_INT32(jReqBody, UPDATE_TYPE, m_certInfo.updateType);
    GET_JSON_STRING(jReqBody, JOB_ID, m_jobId);
    if (m_jobId.empty()) {
        ERRLOG("The jobid is empty!");
        return MP_FAILED;
    }
    INFOLOG("Job id is [%s].", m_jobId.c_str());
    CHECK_FAIL_EX(CheckParamStringEnd(m_jobId, MIN_TASKID_SIZE, MAX_TASKID_SIZE));

    if (m_certInfo.updateType == UPDATE_AGENT_TYPE) {
        GET_JSON_STRING(jReqBody, AGENTCA_FILE, m_certInfo.agentcaFile);
        CHECK_FAIL_EX(CheckParamStringEnd(m_certInfo.agentcaFile, 0, MAX_STRING_LEN_MB));
        GET_JSON_STRING(jReqBody, SERVE_FILE, m_certInfo.serverFile);
        CHECK_FAIL_EX(CheckParamStringEnd(m_certInfo.serverFile, 0, MAX_STRING_LEN_MB));
        GET_JSON_STRING(jReqBody, SERVER_KET_FILE, m_certInfo.serverKeyFile);
        CHECK_FAIL_EX(CheckParamStringEnd(m_certInfo.serverKeyFile, 0, MAX_STRING_LEN_MB));
        GET_JSON_STRING(jReqBody, SERVER_PASSWORD, m_certInfo.passWord);
        CHECK_FAIL_EX(CheckParamStringEnd(m_certInfo.passWord, 0, MAX_ERROR_MSG_LEN));

        if (m_certInfo.agentcaFile.empty() || m_certInfo.serverFile.empty() || m_certInfo.serverKeyFile.empty()) {
            ERRLOG("Number of cert files is error!");
            return MP_FAILED;
        }
        m_certType = AGENT_CERT_TYPE;
    } else if (m_certInfo.updateType == UPDATE_PM_TYPE) {
        GET_JSON_STRING(jReqBody, PMCA_FILE, m_certInfo.pmcaFile);
        CHECK_FAIL_EX(CheckParamStringEnd(m_certInfo.pmcaFile, 0, MAX_STRING_LEN_MB));
        if (m_certInfo.pmcaFile.empty()) {
            ERRLOG("The file pmca is empty!");
            return MP_FAILED;
        }
        m_certType = PM_CERT_TYPE;
    }

    return MP_SUCCESS;
}

bool UpdateCertHandle::AddCertCNToHosts()
{
    INFOLOG("Check if cert cn valid");
    mp_string cnNameOfServerCrt;
    SecureCom::GetHostFromCert(m_certPathNew + SERVER_PEM_NAME, cnNameOfServerCrt);
    INFOLOG("The cert CN is: %s.", cnNameOfServerCrt.c_str());
    mp_int32 iRet = MP_SUCCESS;
#ifdef WIN32
    iRet = WriteCertCNToWindowsHosts(cnNameOfServerCrt);
#else
    iRet = WriteCertCNToNoWindowsHosts(cnNameOfServerCrt);
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Add cert cn to hosts failed.");
        return MP_FAILED;
    }
    return MP_TRUE;
}

#ifdef WIN32
mp_int32 UpdateCertHandle::WriteCertCNToWindowsHosts(mp_string& cnNameOfServerCrt)
{
    mp_string hostsFile = "C:\\Windows\\System32\\drivers\\etc\\hosts";
    std::vector<mp_string> vecFileContent;
    mp_int32 iRet = CIPCFile::ReadFile(hostsFile, vecFileContent);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Read file %s failed.", hostsFile.c_str());
        return MP_FAILED;
    }

    mp_bool isExist1 = MP_FALSE;
    mp_bool isExist2 = MP_FALSE;
    mp_string addedContent1 = "127.0.0.1 " + cnNameOfServerCrt;
    mp_string addedContent2 = "::1 " + cnNameOfServerCrt;
    for (auto &iter : vecFileContent) {
        if (iter == addedContent1) {
            INFOLOG("The cert CN has been add to hosts.");
            isExist1 = MP_TRUE;
        } else if (iter == addedContent2) {
            INFOLOG("The cert CN has been add to hosts.");
            isExist2 = MP_TRUE;
        }
    }

    if (!isExist1) {
        vecFileContent.push_back(addedContent1);
        CIPCFile::WriteFile(hostsFile, vecFileContent);
        INFOLOG("Write %s to %s succ.", addedContent1.c_str(), hostsFile.c_str());
    }

    if (!isExist2) {
        vecFileContent.push_back(addedContent2);
        CIPCFile::WriteFile(hostsFile, vecFileContent);
        INFOLOG("Write %s to %s succ.", addedContent2.c_str(), hostsFile.c_str());
    }
    return MP_SUCCESS;
}
#else
mp_int32 UpdateCertHandle::WriteCertCNToNoWindowsHosts(mp_string& cnNameOfServerCrt)
{
    mp_string hostsFile = "/etc/hosts";
    mp_string content = "127.0.0.1\n" + cnNameOfServerCrt;

    vector<mp_string> vecResult;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_WRITE_CERT_CN, content, &vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Write cert cn %s to %s failed.", content.c_str(), hostsFile.c_str());
        return iRet;
    }

    vecResult.clear();
    content = "::1\n" + cnNameOfServerCrt;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_WRITE_CERT_CN, content, &vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Write cert cn %s to %s failed.", content.c_str(), hostsFile.c_str());
        return iRet;
    }
    return MP_SUCCESS;
}
#endif

mp_int32 UpdateCertHandle::ParseFile()
{
    INFOLOG("Start to save cert files.");
    mp_int32 iRet = CMpFile::CreateDir(m_certUpdateDir.c_str());
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create cert update dir error!");
    }
    iRet = CMpFile::CreateDir(m_certPathNew.c_str());
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create path of cert files error!");
    }

    if (m_certInfo.updateType == UPDATE_AGENT_TYPE) {
        WriteInFile(m_certPathNew + AGENTCA_PEM_NAME, m_certInfo.agentcaFile);
        WriteInFile(m_certPathNew + SERVER_PEM_NAME, m_certInfo.serverFile);
        WriteInFile(m_certPathNew + SERVER_KEY_NAME, m_certInfo.serverKeyFile);
        mp_string outStr;
        EncryptStr(m_certInfo.passWord, outStr);
        if (outStr.empty()) {
            ERRLOG("EncryptStr password fail!");
            return MP_FAILED;
        }
        WriteInFile(m_certPathNew + KEY_PWD_NAME, outStr);
        return MP_SUCCESS;
    } else if (m_certInfo.updateType == 0) {
        WriteInFile(m_certPathNew + PMCA_PEM_NAM, m_certInfo.pmcaFile);
        return MP_SUCCESS;
    }

    ERRLOG("Save cert files failed!");
    return MP_FAILED;
}

void UpdateCertHandle::WaitPmNotify()
{
    int time = 20 * 60;
    std::unique_lock<std::mutex> lck(m_pnMutex);
    if (!m_connectPmToAgent || !m_connectAgentToPm) {
        if (m_pnCond.wait_for(lck, std::chrono::seconds(time)) == std::cv_status::timeout) {
            INFOLOG("wake up!");
        } else {
            INFOLOG("timeout!");
        }
    } else {
        INFOLOG("PM to Agent already success!");
    }
}

mp_int32 UpdateCertHandle::PmNotifyAgentUseNewCert()
{
    if (!CMpFile::FileExist(FLAG_FILE_UPDATING)) {
        INFOLOG("Do not need to create connection flag file!");
        return MP_SUCCESS;
    }

    // save pm -> agent status
    if (CMpFile::CreateFile(FLAG_FILE_CONNRCT) != MP_SUCCESS) {
        WARNLOG("Save pm to agent status fail.");
        return MP_FAILED;
    }

    if (CheckConnectionToPM() != MP_SUCCESS) {
        m_connectAgentToPm = false;
        ERRLOG("Check connection of agent to pm failed.");
        return MP_FAILED;
    } else {
        m_connectAgentToPm = true;
    }

    std::unique_lock<std::mutex> lck(m_pnMutex);

    m_connectPmToAgent = true;
    m_pnCond.notify_all();

    INFOLOG("PM notify agent success!");
    return MP_SUCCESS;
}

bool UpdateCertHandle::CheckPmToAgentStatus()
{
    if (!CMpFile::FileExist(FLAG_FILE_CONNRCT)) {
        INFOLOG("The pm to agent status is not ok at present.");
        return false;
    }
    INFOLOG("The pm to agent status is already ok.");
    return true;
}

mp_int32 UpdateCertHandle::WriteInFile(const mp_string& filePath, const mp_string& FileContent)
{
    INFOLOG("Write file to path: %s", filePath.c_str());
    mp_int32 iRet = CMpFile::CreateFile(filePath);
#ifndef WIN32
    ChangeFilePermission(filePath);
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create files failed, the filepath = %s", filePath.c_str());
        return MP_FAILED;
    }
    vector<mp_string> content;
    content.push_back(FileContent);
    iRet =  CIPCFile::WriteFile(filePath, content);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Write files failed, the filepath = %s", filePath.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

#ifndef WIN32
// 更改文件权限为600
void UpdateCertHandle::ChangeFilePermission(const mp_string& fileName)
{
    const char *filePath = fileName.c_str();
    if (chmod(filePath, S_IRUSR | S_IWUSR) == 0) {
        INFOLOG("Change permission of file %s successfully.", fileName.c_str());
    } else {
        ERRLOG("Change permission of file %s failed!", fileName.c_str());
    }
}
#endif

mp_int32 UpdateCertHandle::CheckBeforeUpdate(mp_int32 certType)
{
    mp_int32 iRet = MP_SUCCESS;
    // 检查路径是否存在  参考脚本文件是否存在判断
    if (certType == PM_CERT_TYPE) {
        mp_string certFileNew = m_certPathNew + PMCA_PEM_NAM;
        if (!CMpFile::FileExist(certFileNew)) {
            ERRLOG("Cert files not exist! filePath=%s.errno=%d.", certFileNew.c_str());
            return MP_FAILED;
        }
    } else {
        for (auto tmpName : VEC_CERT_FILE_NAME_AGENT) {
            mp_string certFileNew = m_certPathNew + tmpName;
            if (!CMpFile::FileExist(certFileNew)) {
                ERRLOG("Cert files not exist! filePath=%s.errno=%d.", certFileNew.c_str());
                return MP_FAILED;
            }
        }
    }
    return iRet;
}

mp_int32 UpdateCertHandle::CleanCertFilesHandle(CRequestMsg& req)
{
    INFOLOG("Begin to clean temp files.");
    if (CheckJobID(req) != MP_SUCCESS) {
        ERRLOG("Check Job ID Failed.");
        return MP_FAILED;
    }

    return ExecCleanTempFiles();
}

mp_int32 UpdateCertHandle::ExecCleanTempFiles()
{
    mp_int32 iRet = MP_SUCCESS;
    // 调用脚本删除临时文件
#ifdef WIN32
    mp_string strScriptPath =  CPath::GetInstance().GetBinFilePath(PUSH_UPDATE_CERT);
    mp_string strCmd = "cmd.exe /c " + strScriptPath + " " + CLEAN_PAMA + + " " +  m_jobId;
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
#else
    CRootCaller rootCaller;
    std::ostringstream scriptParam;
    scriptParam << "jobID=" << m_jobId << NODE_COLON << "fallBackType=" << CLEAN_PAMA << NODE_COLON;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_PUSHUPDATECERT, scriptParam.str(), NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_THAW_FAILED);
#endif
    return MP_SUCCESS;
}

mp_int32 UpdateCertHandle::UpdateCert()
{
    INFOLOG("Begin to execute the update script.");
    mp_int32 iRet = MP_SUCCESS;
    // 执行脚本  参考agent更新
#ifdef WIN32
    mp_string strScriptPath =  CPath::GetInstance().GetBinFilePath(PUSH_UPDATE_CERT);
    mp_string strCmd = "cmd.exe /c " + strScriptPath + " " + UPDATE_PARAM + " " + m_jobId;
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
#else
    CRootCaller rootCaller;
    std::ostringstream scriptParam;
    scriptParam << "jobID=" << m_jobId << NODE_COLON << "fallBackType=" << UPDATE_PARAM << NODE_COLON;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_PUSHUPDATECERT, scriptParam.str(), NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_THAW_FAILED);
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Excute script failed, iRet = %d.", iRet);
        return iRet;
    }
    DBGLOG("Succeed to excute the upgrade script.");

    return MP_SUCCESS;
}

mp_int32 UpdateCertHandle::FallbackCertHandle(CRequestMsg& req)
{
    INFOLOG("Begin to execute the fallback script.");
    if (CheckJobID(req) != MP_SUCCESS) {
        ERRLOG("Check Job ID Failed.");
        return MP_FAILED;
    }

    if (!CreateRollbackThread()) {
        ERRLOG("Create rollback cert thread failed.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 UpdateCertHandle::ExecRollbackCert()
{
    m_execTheadRunningFlag = true;
    mp_int32 iRet = MP_SUCCESS;
    if (m_jobId.empty() && !GetJobidInTmpFile(m_jobId)) {
        ERRLOG("Get jobId from temp file failed before rolling back.");
        m_execTheadRunningFlag = false;
        return MP_FAILED;
    }

#ifdef WIN32
    mp_string strScriptPath =  CPath::GetInstance().GetBinFilePath(PUSH_UPDATE_CERT);
    mp_string strCmd = "cmd.exe /c " + strScriptPath + " " + FALLBACK_PAMA + " " + m_jobId;
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
#else
    CRootCaller rootCaller;
    std::ostringstream scriptParam;
    scriptParam << "jobID=" << m_jobId << NODE_COLON << "fallBackType=" << FALLBACK_PAMA << NODE_COLON;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_PUSHUPDATECERT, scriptParam.str(), NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_THAW_FAILED);
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Excute rallback script failed, iRet = %d.", iRet);
        m_execTheadRunningFlag = false;
        return iRet;
    }
    DBGLOG("Succeed to rallback excute the upgrade script.");
    m_execTheadRunningFlag = false;
    return iRet;
}

mp_int32 UpdateCertHandle::CheckJobID(CRequestMsg& req)
{
    mp_string strJobID;
    const Json::Value& jReqBody = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jReqBody, JOB_ID, strJobID);
    if (strJobID.empty()) {
        ERRLOG("The jobid is empty!");
        return MP_FAILED;
    }
    CHECK_FAIL_EX(CheckParamStringEnd(strJobID, MIN_TASKID_SIZE, MAX_TASKID_SIZE));
    
    mp_string newCertDirName = CPath::GetInstance().GetTmpPath() + PATH_SEPARATOR + "cert_updating_";
    newCertDirName += strJobID + PATH_SEPARATOR;
    if (!CMpFile::DirExist(newCertDirName.c_str())) {
        ERRLOG("New cert files not exist, jobid is %s.", strJobID.c_str());
        return MP_FAILED;
    }

    if (m_certPathNew.empty()) {
        m_certPathNew = newCertDirName + "newCert" + PATH_SEPARATOR;
    }
    if (m_certPathOld.empty()) {
        m_certPathOld = newCertDirName + "oldCert" + PATH_SEPARATOR;
    }

    if (strJobID != m_jobId) {
        m_jobId = strJobID;
    }
    return MP_SUCCESS;
}

// Support PM reentrancy
mp_bool UpdateCertHandle::CheckIfExistHandle(mp_string &jobId)
{
    if (m_execTheadRunningFlag && CheckIfJobidEqual(jobId)) {
        return MP_TRUE;
    }
    return MP_FALSE;
}

mp_bool UpdateCertHandle::CheckIfJobidEqual(mp_string &jobId)
{
    if (jobId.empty()) {
        return MP_FALSE;
    }
    mp_string resJobid;
    if (!GetJobidInTmpFile(resJobid)) {
        return MP_FALSE;
    }
    if (resJobid == jobId) {
        return MP_TRUE;
    }
    return MP_FALSE;
}