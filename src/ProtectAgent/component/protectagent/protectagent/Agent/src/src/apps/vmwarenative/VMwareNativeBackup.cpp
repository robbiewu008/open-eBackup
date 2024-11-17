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
#include "apps/vmwarenative/VMwareNativeBackup.h"
#include "apps/vmwarenative/VMwareDef.h"

#include <ctime>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "common/MpString.h"
#include "common/JsonUtils.h"
#include "common/CSystemExec.h"
#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/value.h"
#include "taskmanager/TaskContext.h"
#include "taskmanager/TaskManager.h"
#include "apps/vmwarenative/VMwareNativeVmBackupTask.h"
#include "apps/vmwarenative/VMwareNativeVmRestoreTask.h"
#include "apps/vmwarenative/VMwareNativeBackupPreparationTask.h"
#include "apps/vmwarenative/VMwareNativeRestorePreparationTask.h"
#include "apps/vmwarenative/VMwareNativeBackupCleanupTask.h"
#include "apps/vmwarenative/VMwareNativeInitVddkLibTask.h"
#include "apps/vmwarenative/VMwareNativeCleanupVddkLibTask.h"
#include "apps/vmwarenative/VMwareNativeBackupOpenDiskTask.h"
#include "apps/vmwarenative/VMwareNativeBackupCloseDiskTask.h"
#include "apps/vmwarenative/VMwareNativePrepareAfsBitmapTask.h"
#include "apps/vmwarenative/VmfsHandler.h"
#include "apps/vmwarenative/StorageLayerNas.h"
#include "common/Path.h"
#include "common/File.h"
#include "curl/curl.h"

using namespace std;
namespace {
constexpr mp_int32 MAX_MSG_BODY_SIZE = 1024 * 1024 * 10;
constexpr mp_int32 MAX_TASK_ID_SIZE = 512;
const mp_string DME_JSON_KEY_PRODUCTMANAGER = "ProductManager";
const mp_string DME_JSON_KEY_CAINFO = "Certs";
const mp_string DME_JSON_KEY_CRLINFO = "Cls";
const mp_string DME_JSON_KEY_IP = "IP";
const mp_string DME_JSON_KEY_PORT = "Port";
const mp_string DME_JSON_KEY_TLS = "TlsCompatible";
const mp_string TMP_VCENTER_CA_FILE = "tmpVcenter_CA.pem";
const mp_string TMP_VCENTER_CRL_FILE = "tmpVcenter_CRL.crl";
const string SSL_CIPHER_LIST =
    "DHE-RSA-AES128-GCM-SHA256:"\
    "DHE-RSA-AES256-GCM-SHA384:"\
    "DHE-DSS-AES128-GCM-SHA256:"\
    "DHE-DSS-AES256-GCM-SHA384:"\
    "ECDHE-ECDSA-AES128-GCM-SHA256:"\
    "ECDHE-ECDSA-AES256-GCM-SHA384:"\
    "ECDHE-RSA-AES128-GCM-SHA256:"\
    "ECDHE-RSA-AES256-GCM-SHA384:"\
    "ECDHE-RSA-CHACHA20-POLY1305:"\
    "TLS_DHE_RSA_WITH_AES_128_CCM:"\
    "TLS_DHE_RSA_WITH_AES_256_CCM:"\
    "dhe_rsa_chacha20_poly1305_sha_256:"\
    "TLS_ECDHE_ECDSA_WITH_AES_128_CCM:"\
    "TLS_ECDHE_ECDSA_WITH_AES_256_CCM:"\
    "ECDHE-ECDSA-CHACHA20-POLY1305"; // TLS使用的秘钥组件
constexpr mp_int32 MAX_CURL_RETRY = 3; // curl超时重试次数
constexpr mp_int32 MIN_CERT_LEN = 0; // CA内容为空字符串则说明Agent不需要认证Vcenter证书
constexpr mp_int32 MAX_CERT_LEN_1MB = 1048576; // 最大的CA与吊销列表长度1MB = 1024 * 1024
constexpr mp_int32 MAX_TEXT_LEN = 20;
constexpr mp_uint32 MIN_PORTS_NUM = 1;
constexpr mp_uint32 MAX_PORTS_NUM = 65535;
const mp_string PARAM_NAS_OVER_FC_STR = "accessNASOverFC";
const mp_string DEFAULT_NAS_MNT_POINT_PREFIX = "/opt/advbackup/vmware/data/";
const mp_string DORADO_NAS_SNAP_PATH = "/.snapshot/";
}  // namespace

VMwareNativeBackup::VMwareNativeBackup()
{}

VMwareNativeBackup::~VMwareNativeBackup()
{}

/* ------------------------------------------------------------
Description  :Use CA and CRL to verify Vcenter before init VDDK
------------------------------------------------------------- */
mp_int32 VMwareNativeBackup::VerifyVcenterCert(const mp_string &msgBody, mp_string &taskId)
{
    LOGGUARD("");
    VerifyVcenterParam VerifyParam;
    mp_int32 iRet = Parsejson(msgBody, taskId, VerifyParam);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Parse json from msgBody failed.");
        return iRet;
    }
    INFOLOG("Option of TLS compatibility is [%d].", VerifyParam.bTLSCompatible);

    // 若DME下发的CA内容为空则说明不需要Agent认证Vcenter
    if (VerifyParam.vecCAInfo[0].empty()) {
        INFOLOG("No verify Vcenter certificate required.");
        return MP_SUCCESS;
    }

    iRet = PreVerify(VerifyParam);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Prep work failed.");
        return iRet;
    }

    iRet = UseCurl2VerifyVcenter(taskId, VerifyParam);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Verify Vcenter failed.");
        return iRet;
    }

    iRet = CleanUpTmpCertFile(VerifyParam);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Clean Up tmp_CA and tmp_CRL failed.");
        return iRet;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :从json中解析出Vcenter的CA证书、CRL文件、IP、端口信息
------------------------------------------------------------- */
mp_int32 VMwareNativeBackup::Parsejson(const mp_string &msgBody, mp_string &taskId, VerifyVcenterParam &VerifyParam)
{
    LOGGUARD("");
    // 获取DPP帧中的json体
    Json::Value jsonMsgBody;
    if (GetRequestJsonBody(msgBody, jsonMsgBody) != MP_SUCCESS) {
        ERRLOG("Unable to parse attr 'body' from the request message!");
        return ERROR_COMMON_INVALID_PARAM;
    }
    
    // 获取任务id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // 获取body中的子json体ProductManager
    if (!jsonMsgBody.isObject() || !jsonMsgBody.isMember(DME_JSON_KEY_PRODUCTMANAGER)) {
        ERRLOG("[%s] key does not exist.", DME_JSON_KEY_PRODUCTMANAGER.c_str());
        return MP_FAILED;
    }
    Json::Value jsonProductManager = jsonMsgBody[DME_JSON_KEY_PRODUCTMANAGER];
    
    // 解析并获取json体中的证书与吊销列表内容
    GET_JSON_ARRAY_STRING(jsonProductManager, DME_JSON_KEY_CAINFO, VerifyParam.vecCAInfo);
    if (VerifyParam.vecCAInfo.size() < 1) {
        ERRLOG("CA info is null.");
        return MP_FAILED;
    }
    CHECK_FAIL_EX(CheckParamStringEnd(VerifyParam.vecCAInfo[0], MIN_CERT_LEN, MAX_CERT_LEN_1MB));
    mp_string strCRL;
    GET_JSON_STRING(jsonProductManager, DME_JSON_KEY_CRLINFO, strCRL);
    CHECK_FAIL_EX(CheckParamStringEnd(strCRL, MIN_CERT_LEN, MAX_CERT_LEN_1MB));
    VerifyParam.vecCRLInfo.push_back(strCRL);

    // 从json体中获取IP和Port
    GET_JSON_STRING(jsonProductManager, DME_JSON_KEY_IP, VerifyParam.strIP);
    CHECK_FAIL_EX(CheckParamStringIsIP(VerifyParam.strIP));
    GET_JSON_UINT32(jsonProductManager, DME_JSON_KEY_PORT, VerifyParam.uintProt);
    CHECK_FAIL_EX(CheckParamInteger32(VerifyParam.uintProt, MIN_PORTS_NUM, MAX_PORTS_NUM));

    // 从json体中获取TLS兼容开关
    GET_JSON_BOOL(jsonProductManager, DME_JSON_KEY_TLS, VerifyParam.bTLSCompatible);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :分别将CA与CRL信息写入指定文件，并拼装出URL
------------------------------------------------------------- */
mp_int32 VMwareNativeBackup::PreVerify(VerifyVcenterParam &VerifyParam)
{
    LOGGUARD("");
    // 获取证书存放临时文件路径
    VerifyParam.strTmpVcenterCAFile = CPath::GetInstance().GetTmpFilePath(TMP_VCENTER_CA_FILE);
    VerifyParam.strTmpVcenterCRLFile = CPath::GetInstance().GetTmpFilePath(TMP_VCENTER_CRL_FILE);
    // 将获取的CA证书与CRL文件的内容分别写入对应的临时问文件
    mp_int32 iRet = CIPCFile::WriteFile(VerifyParam.strTmpVcenterCAFile, VerifyParam.vecCAInfo);
    if (MP_SUCCESS != iRet) {
        ERRLOG("Write Vcenter CA info to file failed.");
        return MP_FAILED;
    }
    if (VerifyParam.vecCRLInfo.size() < 1) {
        ERRLOG("CRL info is null.");
        return MP_FAILED;
    }
    if (!VerifyParam.vecCRLInfo[0].empty()) {
        iRet = CIPCFile::WriteFile(VerifyParam.strTmpVcenterCRLFile, VerifyParam.vecCRLInfo);
        if (MP_SUCCESS != iRet) {
            ERRLOG("Write Vcenter CRL info to file failed.");
            return MP_FAILED;
        }
        INFOLOG("Vcenter CRL is imported.");
        VerifyParam.bCRLIsEmpty = MP_FALSE;
    }

    // 组装URL
    mp_string strProt;
    CMpString::UIntegerToString(VerifyParam.uintProt, strProt);
    VerifyParam.strVerifyURL = mp_string("https://") + VerifyParam.strIP + mp_string(":") + strProt;

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :使用curl通过DME传下CA与CRL检测对端生产环境
------------------------------------------------------------- */
mp_int32 VMwareNativeBackup::UseCurl2VerifyVcenter(const mp_string &taskId, const VerifyVcenterParam &VerifyParam)
{
    LOGGUARD("");
    // curl初始化
    uint32_t m_ErrorCode = 0;
    CURL* m_Curl = curl_easy_init();
    if (m_Curl == nullptr) {
        ERRLOG("Init curl failed.");
        return MP_FAILED;
    }
    SetCurlOption(VerifyParam, m_Curl);
    // 超时重试
    for (mp_int32 retryCount = 1; retryCount <= MAX_CURL_RETRY; retryCount++) {
        m_ErrorCode = curl_easy_perform(m_Curl);
        if (m_ErrorCode == CURLE_COULDNT_RESOLVE_HOST) {
            WARNLOG("Timeout retry %d.", retryCount);
            sleep(1);
            continue;
        }
        break;
    }
    if (m_Curl != nullptr) {
        curl_easy_cleanup(m_Curl);
        m_Curl = nullptr;
    }
    // 检测curl结果
    if (m_ErrorCode == CURLE_SSL_CIPHER) {
        ERRLOG("The peer Vcenter does not support the secure TLS cipher suite, taskId is [%s].", taskId.c_str());
        return MP_FAILED;
    }
    if (m_ErrorCode != CURLE_OK) {
        ERRLOG("Verify VcenterCert failed, taskId is [%s], curl_Erroecode is [%u].", taskId.c_str(), m_ErrorCode);
        return MP_FAILED;
    }
    INFOLOG("Verify VcenterCert success, taskId is [%s].", taskId.c_str());
    return MP_SUCCESS;
}


mp_int32 VMwareNativeBackup::SetCurlOption(const VerifyVcenterParam &VerifyParam, CURL* m_Curl)
{
    LOGGUARD("");
    char curl_error_str[CURL_ERROR_SIZE] = {0};
    // curl参数设置
    const mp_uint32 time_out = 90; // set timeout parameters
    curl_easy_setopt(m_Curl, CURLOPT_FORBID_REUSE, true);
    curl_easy_setopt(m_Curl, CURLOPT_NOSIGNAL, true); // 忽略所有的传递给进程的信号
    curl_easy_setopt(m_Curl, CURLOPT_CONNECTTIMEOUT, static_cast<int>(time_out)); // 尝试连接时等待的秒数
    curl_easy_setopt(m_Curl, CURLOPT_TIMEOUT, static_cast<int>(time_out)); // 允许执行的最长秒数
    curl_easy_setopt(m_Curl, CURLOPT_VERBOSE, false); // 不将SSL证书信息输出到STDERR
    curl_easy_setopt(m_Curl, CURLOPT_ERRORBUFFER, curl_error_str); // 错误信息缓冲区
    // curl请求URL设置
    curl_easy_setopt(m_Curl, CURLOPT_URL, VerifyParam.strVerifyURL.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_POST, true);
    // curl证书相关验证参数设置
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYPEER, 1); // 设置是否验证对端证书
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYHOST, 0); // 不从对端证书中认证主机名
    curl_easy_setopt(m_Curl, CURLOPT_CAINFO, VerifyParam.strTmpVcenterCAFile.c_str()); // 设置认证的CA证书路径
    // 根据需求设置认证的CRL路径
    if (!VerifyParam.bCRLIsEmpty) {
        curl_easy_setopt(m_Curl, CURLOPT_CRLFILE, VerifyParam.strTmpVcenterCRLFile.c_str());
    }
    // 设置使用的TLS
    if (!VerifyParam.bTLSCompatible) {
        curl_easy_setopt(m_Curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2); // 设置https采用的TLS版本为1.2及以上
        curl_easy_setopt(m_Curl, CURLOPT_SSL_CIPHER_LIST, SSL_CIPHER_LIST.c_str()); // 设置TLS密码套件组
    }
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  :清理临时的CA与CRL文件
------------------------------------------------------------- */
mp_int32 VMwareNativeBackup::CleanUpTmpCertFile(const VerifyVcenterParam &VerifyParam)
{
    LOGGUARD("");
    mp_int32 iRet = CMpFile::DelFile(VerifyParam.strTmpVcenterCAFile);
    if (MP_SUCCESS != iRet) {
        ERRLOG("Delet temporary CA file failed.");
        return MP_FAILED;
    }
    if (!VerifyParam.bCRLIsEmpty) {
        iRet = CMpFile::DelFile(VerifyParam.strTmpVcenterCRLFile);
        if (MP_SUCCESS != iRet) {
            ERRLOG("Delet temporary CRL file failed.");
            return MP_FAILED;
        }
    }
    
    INFOLOG("Clean up temporary CA and CRL files successfully");
    return MP_SUCCESS;
}

// init vddk both for backup and recovery task
mp_int32 VMwareNativeBackup::InitVddkLib(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter InitVddkLib...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask != NULL) {
        ERRLOG("Backup task[%s] alreadly exists.", taskId.c_str());
        return MP_FAILED;
    }

    // create backup task thread
    INFOLOG("Start vmware native protection task[%s].", taskId.c_str());
    Json::Value respMsg;
    return RunSyncTask<VMwareNativeInitVddkLibTask>(msgBody, taskId, respMsg);
}

// cleanup vddk both for backup and recovery task
mp_int32 VMwareNativeBackup::CleanupVddkLib(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter CleanupVddkLib...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        RemoveTask(taskId);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask != NULL) {
        ERRLOG("Backup task[%s] alreadly exists.", taskId.c_str());
        RemoveTask(taskId);
        return MP_FAILED;
    }

    // create backup task thread
    INFOLOG("Start vmware native protection task[%s].", taskId.c_str());
    Json::Value respMsg;
    return RunSyncTask<VMwareNativeCleanupVddkLibTask>(msgBody, taskId, respMsg);
}

// prepare backup -- sync task
mp_int32 VMwareNativeBackup::PrepareBackup(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter PrepareBackup...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask != NULL) {
        ERRLOG("Backup task[%s] alreadly exists.", taskId.c_str());
        return MP_FAILED;
    }

    // set backend storage protocol type
    iRet = SetStorageProtocolForTask(taskId, jsonMsgBody);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Unabel to SetStorageProtocolForTask. task id: '%s' from request message body, iRet=%d.",
            taskId.c_str(), iRet);
    }

    // set disk type
    iRet = SetDiskTypeForTask(taskId, jsonMsgBody);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Unabel to SetDiskTypeForTask. task id: '%s' from request message body, iRet=%d.",
                taskId.c_str(), iRet);
    }

    // create backup task thread
    INFOLOG("Start vmware native backup task[%s].", taskId.c_str());
    return RunSyncTask<VMwareNativeBackupPreparationTask>(msgBody, taskId, respBody);
}

mp_int32 VMwareNativeBackup::OpenDiskBackup(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId,
    Json::Value &respBody, mp_string &strError)
{
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        strError = "Unabel to get backup task id from request message body.";
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask != NULL) {
        ERRLOG("Backup task[%s] already exist.", taskId.c_str());
        strError = "Backup task already exist.";
        return MP_FAILED;
    }

    INFOLOG("Start vmware native OpenDisk task[%s].", taskId.c_str());
    iRet = RunSyncTask<VMwareNativeBackupOpenDiskTask>(msgBody, taskId, respBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Run sychronous task(%s) failed.", taskId.c_str());
        strError = "Run sychronous task failed.";
        return iRet;
    }

    return MP_SUCCESS;
}

// do data block backup
mp_int32 VMwareNativeBackup::BackupDataBlocks(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter BackupDataBlocks...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask == NULL) {
        ERRLOG("Backup task[%s] does not exist, will start it.", taskId.c_str());
        if (MP_SUCCESS != CreateTask<VMwareNativeVmBackupTask>(msgBody, connIp, connPort, taskId)) {
            ERRLOG("create VMwareNativeVmBackupTask failed, task id '%s'.", taskId.c_str());
            return MP_FAILED;
        }
        TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_TASK_CMDNO, MANAGE_CMD_NO_VMWARENATIVE_RUN_BACKUP);
        // set backend storage protocol type
        iRet = SetStorageProtocolForTask(taskId, jsonMsgBody);
        if (iRet != MP_SUCCESS) {
            WARNLOG("Unabel to SetStorageProtocolForTask. task id: '%s' from request message body, iRet=%d.",
                taskId.c_str(), iRet);
        }
        INFOLOG("Start vmware native backup data block task[%s].", taskId.c_str());
        return TaskManager::GetInstance()->RunTask(taskId);
    }

    // due to update/progress task both use method 'Update(Json::Value&)' of Task.cpp, here pass the request command
    jsonMsgBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_VMWARENATIVE_RUN_BACKUP;
    iRet = pTask->UpdateTask(jsonMsgBody, respBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to update backup task %s, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }
    INFOLOG("Exit BackupDataBlocks...");
    return MP_SUCCESS;
}

mp_bool VMwareNativeBackup::AgentRestartRecently()
{
    DBGLOG("Enter AgentRestartRecently...");
    // strCmd无外部注入
    mp_string strCmd = "ps -eo etime,pid | grep '" + to_string(getpid()) +
        "' | grep -v 'grep' | grep -v 'gdb' | grep -v 'vi' | grep -v 'tail'";
    std::vector<mp_string> vecReturn;
    CSystemExec::ExecSystemWithEcho(strCmd, vecReturn);
    for (int i = 0; i < vecReturn.size(); i++) {
        mp_string strCMDReturn = vecReturn[i];
        std::vector<mp_string> vecCMDReturn;
        CMpString::StrSplit(vecCMDReturn, strCMDReturn, ' ');
        mp_string strRunningTime;

        // 获取到的cmd输出有空格，需要获取的时间为第一个非空字段,增加相关获取逻辑
        for (int j = 0; j < vecCMDReturn.size(); j++) {
            if (!vecCMDReturn[j].empty()) {
                strRunningTime = vecCMDReturn[j];
                break;
            }
        }

        // 时间有三种格式，如果超过一天则是d-hh:mm:ss,如果不超过一小时，则是mm:ss，其余情况则是hh:mm:ss
        // 查询进度是每30秒一次，考虑到执行时间耗时，以50秒内重启过作为判断依据，所以只判断mm:ss场景(字符长度为5），
        if (!strRunningTime.empty() && strRunningTime.size() == 5) {
            vecCMDReturn.clear();
            CMpString::StrSplit(vecCMDReturn, strRunningTime, ':');

            // 停止进程的过程可能发生在vmware给agent发送查询消息时，超时时间20min, 检查20min内是否重启
            if (vecCMDReturn.size() > 1 && vecCMDReturn[0] <= "20") {
                return true;
            }
        }
    }

    return false;
}

mp_int32 VMwareNativeBackup::PrepareAfsBitmap(
    const mp_string &msgBody, mp_string &taskId, Json::Value &respBody, mp_string &strError)
{
    INFOLOG("Enter PrepareAfsBitmap");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        strError = "Unabel to get backup task id from request message body.";
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = nullptr;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask != nullptr) {
        ERRLOG("Backup task[%s] already exist.", taskId.c_str());
        strError = "Backup task already exist.";
        return MP_FAILED;
    }

    // set backend storage protocol type
    iRet = SetStorageProtocolForTask(taskId, jsonMsgBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to SetStorageProtocolForTask. task id: '%s' from request message body, iRet=%d.",
            taskId.c_str(),
            iRet);
        strError = "Unabel to set storage protocol.";
        return iRet;
    }

    std::string jsonRepStr;
    (void)WipeSensitiveForJsonData(jsonMsgBody.toStyledString(), jsonRepStr);
    INFOLOG("Start vmware native get afs bitmap task[%s].", taskId.c_str());
    iRet = RunSyncTask<VMwareNativePrepareAfsBitmapTask>(msgBody, taskId, respBody);
    if (iRet != MP_SUCCESS) {
        TaskContext::GetInstance()->GetValueString(taskId, KEY_ERRMSG, strError);
        COMMLOG(OS_LOG_ERROR, "Run sychronous task(%s) failed, err:%s.", taskId.c_str(), strError.c_str());
        return iRet;
    }
    INFOLOG("Exit PrepareAfsBitmap");
    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackup::QueryDataBlockBackupProgress(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter QueryDataBlockBackupProgress...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask == NULL) {
        if (AgentRestartRecently()) {
            WARNLOG("Backup task[%s] will retry.", taskId.c_str());
            return ERROR_VM_PROCESS_RESTART_NEED_RETRY;
        }
        ERRLOG("Backup task[%s] does not exist.", taskId.c_str());
        return MP_FAILED;
    }

    mp_uint64 apiInvokeTimestamp = CMpTime::GetTimeUsec() / SECOND_AND_MICROSECOND_TIMES;
    TaskContext::GetInstance()->SetValueString(taskId, KEY_APIINVOKE_TIMESTAMP,
                                               CMpString::to_string(apiInvokeTimestamp));
    DBGLOG("The backup progress query interface is invoked at time: '%ld', task id '%s'.",
        apiInvokeTimestamp, taskId.c_str());

    // due to both update/progress task use method 'Update(Json::Value&)' of Task.cpp, here pass the request command
    jsonMsgBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_VMWARENATIVE_QUERY_BACKUP_PROGRESS;
    iRet = pTask->UpdateTask(jsonMsgBody, respBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to update backup task[%s], iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }
    INFOLOG("Exit QueryDataBlockBackupProgress...");

    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackup::FinishDataBlockBackup(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter FinishDataBlockBackup...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask == NULL) {
        ERRLOG("Backup task[%s] does not exist. Try close disk", taskId.c_str());
        Json::Value tempRespBody;
        iRet = RunSyncTask<VMwareNativeBackupCloseDiskTask>(msgBody, taskId, tempRespBody);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Run sychronous VMwareNativeBackupCloseDiskTask(%s) failed.", taskId.c_str());
        }
        if (AgentRestartRecently()) {
            WARNLOG("Backup task[%s] will retry.", taskId.c_str());
            return ERROR_VM_PROCESS_RESTART_NEED_RETRY;
        }
        return MP_FAILED;
    }

    mp_uint64 apiInvokeTimestamp = CMpTime::GetTimeUsec() / SECOND_AND_MICROSECOND_TIMES;
    TaskContext::GetInstance()->SetValueString(taskId, KEY_APIINVOKE_TIMESTAMP,
                                               CMpString::to_string(apiInvokeTimestamp));
    INFOLOG("The backup finish interface is invoked at time: '%ld', task id '%s'.",
        apiInvokeTimestamp, taskId.c_str());

    // due to only one interface 'Finish(Json::Value&)' provided in Task.cpp, here pass the request command
    jsonMsgBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_BACKUP;
    iRet = pTask->FinishTask(jsonMsgBody, respBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to finish backup task[%s], iRet=%d.", taskId.c_str(), iRet);
        RemoveTask(taskId);
        return iRet;
    }
    RemoveTask(taskId);
    INFOLOG("Exit FinishDataBlockBackup...");

    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackup::FinishVmBackupAction(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter FinishVmBackupAction...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask == NULL) {
        if (AgentRestartRecently()) {
            WARNLOG("Backup task[%s] will retry.", taskId.c_str());
            return ERROR_VM_PROCESS_RESTART_NEED_RETRY;
        }
        ERRLOG("Backup task[%s] does not exist.", taskId.c_str());
        return MP_FAILED;
    }

    // due to only one interface 'Finish(Json::Value&)' provided in Task.cpp, here pass the request command
    jsonMsgBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_VMWARENATIVE_FINISH_BACKUP_TASK;
    iRet = pTask->FinishTask(jsonMsgBody, respBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to terminate backup task[%s], iRet=%d.", taskId.c_str(), iRet);
        RemoveTask(taskId);
        return iRet;
    }

    RemoveTask(taskId);
    INFOLOG("Exit FinishVmBackupAction...");

    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackup::CancelVmBackupAction(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter FinishDataBlockBackup...");
    // parse task id
    Json::Value jsonMsgBody;
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get backup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask == NULL) {
        ERRLOG("Backup task[%s] does not exist.", taskId.c_str());
        return MP_FAILED;
    }

    iRet = pTask->CancelTask(respBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to cancel backup task[%s], iRet=%d.", taskId.c_str(), iRet);
        RemoveTask(taskId);
        return iRet;
    }

    RemoveTask(taskId);
    INFOLOG("Exit FinishDataBlockBackup...");

    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackup::PrepareRecovery(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter PrepareRecovery...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get restore task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask != NULL) {
        ERRLOG("Restore task[%s] alreadly exists.", taskId.c_str());
        return MP_FAILED;
    }

    // set backend storage protocol type
    iRet = SetStorageProtocolForTask(taskId, jsonMsgBody);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Unabel to SetStorageProtocolForTask. task id: '%s' from request message body, iRet=%d.",
            taskId.c_str(), iRet);
    }

    // set disk type
    iRet = SetDiskTypeForTask(taskId, jsonMsgBody);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Unabel to SetDiskTypeForTask. task id: '%s' from request message body, iRet=%d.",
                taskId.c_str(), iRet);
    }

    // create restore task thread
    INFOLOG("Start vmware native restore task[%s].", taskId.c_str());
    Json::Value respMsg;
    return RunSyncTask<VMwareNativeRestorePreparationTask>(msgBody, taskId, respMsg);
}

mp_int32 VMwareNativeBackup::RecoveryDataBlocks(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter RecoveryDataBlocks...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get restore task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask == NULL) {
        ERRLOG("Restore task[%s] does not exist, will start it.", taskId.c_str());
        if (MP_SUCCESS != CreateTask<VMwareNativeVmRestoreTask>(msgBody, connIp, connPort, taskId)) {
            ERRLOG("create VMwareNativeVmRestoreTask failed, task id '%s'.", taskId.c_str());
            return MP_FAILED;
        }
        TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_TASK_CMDNO, MANAGE_CMD_NO_VMWARENATIVE_RUN_RECOVERY);
        // set backend storage protocol type
        iRet = SetStorageProtocolForTask(taskId, jsonMsgBody);
        if (iRet != MP_SUCCESS) {
            WARNLOG("Unabel to SetStorageProtocolForTask. task id: '%s' from request message body, iRet=%d.",
                taskId.c_str(), iRet);
        }
        INFOLOG("Start vmware native restore data block task[%s].", taskId.c_str());
        return TaskManager::GetInstance()->RunTask(taskId);
    }

    // due to update/progress task both use method 'Update(Json::Value&)' of Task.cpp, here pass the request command
    jsonMsgBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_VMWARENATIVE_RUN_RECOVERY;
    iRet = pTask->UpdateTask(jsonMsgBody, respBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to update restore task[%s], iRet=%d.", taskId.c_str(), iRet);
        return MP_FAILED;
    }
    INFOLOG("Exit RecoveryDataBlocks...");

    return MP_SUCCESS;
}

// query recovery progress
mp_int32 VMwareNativeBackup::QueryDataBlockRecoveryProgress(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter QueryDataBlockRecoveryProgress...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get restore task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask == NULL) {
        if (AgentRestartRecently()) {
            WARNLOG("Restore task[%s] will retry.", taskId.c_str());
            return ERROR_VM_PROCESS_RESTART_NEED_RETRY;
        }
        ERRLOG("Restore task[%s] does not exist.", taskId.c_str());
        return MP_FAILED;
    }

    mp_uint64 apiInvokeTimestamp = CMpTime::GetTimeUsec() / SECOND_AND_MICROSECOND_TIMES;
    TaskContext::GetInstance()->SetValueString(taskId, KEY_APIINVOKE_TIMESTAMP,
                                               CMpString::to_string(apiInvokeTimestamp));
    DBGLOG("The restore progress query interface is invoked at time: '%ld', task id '%s'.",
        apiInvokeTimestamp, taskId.c_str());

    // due to both update/progress task use method 'Update(Json::Value&)' of Task.cpp, here pass the request command
    jsonMsgBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_VMWARENATIVE_QUERY_RECOVERY_PROGRESS;
    iRet = pTask->UpdateTask(jsonMsgBody, respBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to update backup task[%s], iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }
    INFOLOG("Exit QueryDataBlockRecoveryProgress...");

    return MP_SUCCESS;
}

// finish data block recovery
mp_int32 VMwareNativeBackup::FinishDataBlockRecovery(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter FinishDataBlockRecovery...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get restore task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask == NULL) {
        ERRLOG("Restore task[%s] does not exist.", taskId.c_str());
        return MP_FAILED;
    }

    mp_uint64 apiInvokeTimestamp = CMpTime::GetTimeUsec() / SECOND_AND_MICROSECOND_TIMES;
    TaskContext::GetInstance()->SetValueString(taskId, KEY_APIINVOKE_TIMESTAMP,
                                               CMpString::to_string(apiInvokeTimestamp));
    INFOLOG("The restore finish interface is invoked at time: '%ld', task id '%s'.",
        apiInvokeTimestamp, taskId.c_str());

    // due to only one interface 'Finish(Json::Value&)' provided in Task.cpp, here pass the request command
    jsonMsgBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_RECOVERY;
    iRet = pTask->FinishTask(jsonMsgBody, respBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to finish restore task[%s], iRet=%d.", taskId.c_str(), iRet);
        RemoveTask(taskId);
        return iRet;
    }
    RemoveTask(taskId);
    INFOLOG("Exit FinishDataBlockRecovery...");

    return MP_SUCCESS;
}

// finish vm recovery
mp_int32 VMwareNativeBackup::FinishVmRecoveryAction(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter FinishVmRecoveryAction...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get restore task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask == NULL) {
        ERRLOG("Restore task[%s] does not exist.", taskId.c_str());
        return MP_FAILED;
    }

    // due to only one interface 'Finish(Json::Value&)' provided in Task.cpp, here pass the request command
    jsonMsgBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_VMWARENATIVE_FINISH_RECOVERY_TASK;
    iRet = pTask->FinishTask(jsonMsgBody, respBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to terminate restore task[%s], iRet=%d.", taskId.c_str(), iRet);
        RemoveTask(taskId);
        return iRet;
    }
    RemoveTask(taskId);
    INFOLOG("Exit FinishVmRecoveryAction...");

    return MP_SUCCESS;
}

// cancel vm recovery task
mp_int32 VMwareNativeBackup::CancelVmRecoveryAction(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter CancelVmRecoveryAction...");
    // parse task id
    Json::Value jsonMsgBody;
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get restore task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask == NULL) {
        ERRLOG("Restore task[%s] does not exist.", taskId.c_str());
        return MP_FAILED;
    }

    iRet = pTask->CancelTask(respBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to cancel restore task[%s], iRet=%d.", taskId.c_str(), iRet);
        RemoveTask(taskId);
        return iRet;
    }
    RemoveTask(taskId);
    INFOLOG("Exit CancelVmRecoveryAction...");

    return MP_SUCCESS;
}

// cleanup resource -- sync task
mp_int32 VMwareNativeBackup::CleanupResources(
    const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter CleanupResources...");
    Json::Value jsonMsgBody;
    // parse task id
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get cleanup task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }

    // find task in taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask != NULL) {
        ERRLOG("Cleanup task[%s] alreadly exists.", taskId.c_str());
        return MP_FAILED;
    }
    iRet = SetStorageProtocolForTask(taskId, jsonMsgBody);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Unabel to SetStorageProtocolForTask. task id: '%s' from request message body, iRet=%d.",
            taskId.c_str(), iRet);
    }
    INFOLOG("Start vmware native cleanup task[%s].", taskId.c_str());
    Json::Value respMsg;
    return RunSyncTask<VMwareNativeBackupCleanupTask>(msgBody, taskId, respMsg);
}

mp_int32 VMwareNativeBackup::VmfsCheckTool(const mp_string &msgBody, mp_string &taskId)
{
    INFOLOG("Enter VmfsCheckTool...");
    Json::Value jsonMsgBody;
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get VmfsCheckTool task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }
    VmfsHandler vhObj;
    return vhObj.CheckTool();
}

mp_int32 VMwareNativeBackup::VmfsMount(const mp_string &msgBody, mp_string &taskId, Json::Value &respBody)
{
    INFOLOG("Enter VmfsMount...");
    Json::Value jsonMsgBody;
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get VmfsMount task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }
    if (!jsonMsgBody.isObject() || !jsonMsgBody.isMember(PARAM_KEY_VOLUME_WWN)) {
        ERRLOG("The request message has no key: '%s', task id '%s'.", PARAM_KEY_VOLUME_WWN.c_str(), taskId.c_str());
        return MP_FAILED;
    }
    std::vector<mp_string> inputWwnVec;
    CJsonUtils::GetJsonArrayString(jsonMsgBody, PARAM_KEY_VOLUME_WWN, inputWwnVec);
    std::set<mp_string> inputWwnSet(inputWwnVec.begin(), inputWwnVec.end()); // 去重
    std::vector<mp_string> inputWwn(inputWwnSet.begin(), inputWwnSet.end());
    if (inputWwn.size() > MAX_VMFS_MOUNT_NUM) {
        ERRLOG("VmfsMount task[%s] too many to mount. max num is %d, current num is %d!",
            taskId.c_str(), MAX_VMFS_MOUNT_NUM, inputWwn.size());
        return MP_FAILED;
    }
    mp_string mntPth = "";
    VmfsHandler vhObj;
    iRet = vhObj.Mount(inputWwn, mntPth);
    if (iRet != MP_SUCCESS) {
        ERRLOG("VmfsMount task[%s] failed. ", taskId.c_str());
        return MP_FAILED;
    }
    respBody[PARAM_KEY_VMFS_MOUNT_POINT] = mntPth;
    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackup::VmfsUmount(const mp_string &msgBody, mp_string &taskId)
{
    INFOLOG("Enter VmfsUmount...");
    Json::Value jsonMsgBody;
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get VmfsUmount task id '%s' from request message body, iRet=%d.", taskId.c_str(), iRet);
        return iRet;
    }
    if (!jsonMsgBody.isObject() || !jsonMsgBody.isMember(PARAM_KEY_VMFS_MOUNT_PATH)) {
        ERRLOG("The request message has no key: '%s', task id '%s'.",
            PARAM_KEY_VMFS_MOUNT_PATH.c_str(), taskId.c_str());
        return MP_FAILED;
    }
    std::vector<mp_string> inputMntPath;
    CJsonUtils::GetJsonArrayString(jsonMsgBody, PARAM_KEY_VMFS_MOUNT_PATH, inputMntPath);
    std::set<mp_string> inputPathSet(inputMntPath.begin(), inputMntPath.end());
    std::vector<mp_string> inputPathVec(inputPathSet.begin(), inputPathSet.end());
    if (inputPathVec.size() > MAX_VMFS_MOUNT_NUM) {
        ERRLOG("VmfsUmount task[%s] too many to umount. max num is %d, current num is %d!",
            taskId.c_str(), MAX_VMFS_MOUNT_NUM, inputPathVec.size());
        return MP_FAILED;
    }
    mp_int32 retValue = MP_SUCCESS;
    VmfsHandler vhObj;
    for (const auto &iter : inputPathVec) {
        iRet = vhObj.Umount(iter);
        if (iRet != MP_SUCCESS) {
            ERRLOG("VmfsUmount task[%s] failed. umount path is %s ", taskId.c_str(), iter);
            retValue = MP_FAILED;
        }
    }
    return retValue;
}

mp_int32 VMwareNativeBackup::GetStorageLayerNasMountParams(const Json::Value &jsonMsgBody,
    mp_string &storeIp, mp_string &sharePath, mp_string &nasStorageType, mp_string &diskId)
{
    DBGLOG("Enter GetStorageLayerNasMountParams...");
    if (!jsonMsgBody.isObject()) {
        ERRLOG("The request message is illigal.");
        return MP_FAILED;
    }
    if (!jsonMsgBody.isMember(PARAM_KEY_STOR_IP) ||
        !jsonMsgBody.isMember(VMWAREDEF::KEY_NAS_FILESYSTEM_NAME)) {
        ERRLOG("The request message err. key: '%s', key: '%s'.",
            PARAM_KEY_STOR_IP.c_str(), VMWAREDEF::KEY_NAS_FILESYSTEM_NAME.c_str());
        return MP_FAILED;
    }
    mp_int32 iRet = CJsonUtils::GetJsonString(jsonMsgBody, PARAM_KEY_STOR_IP, storeIp);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get StorageLayerNasMount stor ip '%s' from request message body, iRet=%d.",
            PARAM_KEY_STOR_IP.c_str(), iRet);
        return iRet;
    }
    iRet = CJsonUtils::GetJsonString(jsonMsgBody, VMWAREDEF::KEY_NAS_FILESYSTEM_NAME, sharePath);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get StorageLayerNasMount filesystem name '%s' from request message body, iRet=%d.",
            VMWAREDEF::KEY_NAS_FILESYSTEM_NAME.c_str(), iRet);
        return iRet;
    }
    iRet = CJsonUtils::GetJsonString(jsonMsgBody, PARAM_KEY_VOLUME_DISKID, diskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get StorageLayerNasMount '%s' from request message body, iRet=%d.",
            PARAM_KEY_VOLUME_DISKID.c_str(), iRet);
        return iRet;
    }
    if (jsonMsgBody.isMember(PARAM_KEY_STORAGE_TYPE)) {
        iRet = CJsonUtils::GetJsonString(jsonMsgBody, PARAM_KEY_STORAGE_TYPE, nasStorageType);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Unabel to get StorageLayerNasMount storagetype '%s' from request message body, iRet=%d.",
                PARAM_KEY_STORAGE_TYPE.c_str(), iRet);
            return iRet;
        }
    } else {
        WARNLOG("in case version not compatible , make default storagetype dorado.");
        nasStorageType = "dorado";
    }
    DBGLOG("EXIT GetStorageLayerNasMountParams...");
    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackup::StorageLayerNasMount(const mp_string &msgBody, mp_string &taskId, Json::Value &respBody)
{
    DBGLOG("Enter StorageLayerNasMount...");
    Json::Value jsonMsgBody;
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get StorageLayerNasMount task id '%s' from request message body, iRet=%d.",
            taskId.c_str(), iRet);
        return iRet;
    }
    std::string storeIp;
    std::string sharePath;
    std::string nasStorageType;
    std::string diskId;
    if (MP_SUCCESS != GetStorageLayerNasMountParams(jsonMsgBody, storeIp, sharePath, nasStorageType, diskId)) {
        ERRLOG("GetStorageLayerNasMountParams failed, task id '%s'.", taskId.c_str());
        return MP_FAILED;
    }
    mp_string mntPth = "";
    mp_string mntPathSpecify;
    mntPth += DEFAULT_NAS_MNT_POINT_PREFIX + diskId;
    if (nasStorageType == "dorado") {
        mntPth += DORADO_NAS_SNAP_PATH;
    } else if (nasStorageType == "netapp") {
        mntPth += "/";
    } else {
        ERRLOG("nasStorageType [%s] not supported. ", nasStorageType.c_str());
        return MP_FAILED;
    }
    std::ostringstream cmdParam;
    cmdParam << "serviceType=vmware\n"
        << PARAM_KEY_STOR_IP << "=" << storeIp << "\n"
        << VMWAREDEF::KEY_PARENTTASK_ID << "=" << taskId << "\n"
        << VMWAREDEF::KEY_BACKUP_ID << "=" << diskId << "\n"
        << VMWAREDEF::KEY_NAS_FILESYSTEM_NAME << "=" << sharePath << "\n";
    iRet = StorageLayerNas::NasMount(cmdParam);
    if (iRet != MP_SUCCESS) {
        ERRLOG("NasMount task[%s] failed. ", taskId.c_str());
        return MP_FAILED;
    }
    respBody[PARAM_KEY_VMFS_MOUNT_POINT] = mntPth;
    DBGLOG("EXIT StorageLayerNasMount...");
    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackup::StorageLayerNasUnMount(const mp_string &msgBody, mp_string &taskId)
{
    DBGLOG("Enter StorageLayerNasUnMount...");
    mp_int32 retValue = MP_SUCCESS;
    Json::Value jsonMsgBody;
    mp_int32 iRet = GetTaskId(msgBody, jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get StorageLayerNasUnMount task id '%s' from request message body, iRet=%d.",
            taskId.c_str(), iRet);
        return iRet;
    }
    if (!jsonMsgBody.isObject()) {
        ERRLOG("The request message is illigal, task id '%s'.", taskId.c_str());
        return MP_FAILED;
    }
    if (!jsonMsgBody.isMember(PARAM_KEY_STOR_IP) || !jsonMsgBody.isMember(PARAM_KEY_VOLUME_DISKID) ||
        !jsonMsgBody.isMember(VMWAREDEF::KEY_NAS_FILESYSTEM_NAME)) {
        ERRLOG("The request message err. task id '%s'.", taskId.c_str());
        return MP_FAILED;
    }
    std::string storeIp;
    std::string sharePath;
    std::string diskID;
    if (CJsonUtils::GetJsonString(jsonMsgBody, PARAM_KEY_STOR_IP, storeIp) != MP_SUCCESS) {
        ERRLOG("Unabel to get StorageLayerNasUnMount stor ip '%s' from request message body.",
            PARAM_KEY_STOR_IP.c_str());
        return MP_FAILED;
    }
    if (CJsonUtils::GetJsonString(jsonMsgBody, VMWAREDEF::KEY_NAS_FILESYSTEM_NAME, sharePath) != MP_SUCCESS) {
        ERRLOG("Unabel to get StorageLayerNasUnMount stor ip '%s' from request message body.",
            VMWAREDEF::KEY_NAS_FILESYSTEM_NAME.c_str());
        return MP_FAILED;
    }
    if (CJsonUtils::GetJsonString(jsonMsgBody, PARAM_KEY_VOLUME_DISKID, diskID) != MP_SUCCESS) {
        ERRLOG("Unabel to get StorageLayerNasUnMount stor ip '%s' from request message body.",
            VMWAREDEF::KEY_NAS_FILESYSTEM_NAME.c_str());
        return MP_FAILED;
    }
    std::ostringstream cmdParam;
    cmdParam << "serviceType=vmware\n"
        << PARAM_KEY_STOR_IP << "=" << storeIp << "\n"
        << VMWAREDEF::KEY_BACKUP_ID << "=" << diskID << "\n"
        << VMWAREDEF::KEY_NAS_FILESYSTEM_NAME << "=" << sharePath << "\n";
    if (StorageLayerNas::NasUnMount(cmdParam) != MP_SUCCESS) {
        ERRLOG("NasUnMount task[%s] failed. ", taskId.c_str());
        return MP_FAILED;
    }
    DBGLOG("EXIT StorageLayerNasUnMount...");
    return MP_SUCCESS;
}


mp_int32 VMwareNativeBackup::GetTaskId(const mp_string &reqMsg, Json::Value &jsonBody, mp_string &taskid)
{
    if ((reqMsg.length() > MAX_MSG_BODY_SIZE) || (taskid.length() > MAX_TASK_ID_SIZE)) {
        ERRLOG("msg body length=%d, or task id length=%d is out of range", reqMsg.length(), taskid.length());
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (GetRequestJsonBody(reqMsg, jsonBody) != MP_SUCCESS) {
        ERRLOG("Unable to parse attr 'body' from the request message!");
        return ERROR_COMMON_INVALID_PARAM;
    }

    GET_JSON_STRING(jsonBody, KEY_TASKID, taskid);
    if (CalibrationFormatTaskId(taskid) == MP_FAILED) {
        return MP_FAILED;
    }

    DBGLOG("Get task id '%s' from the request message body successfully!", taskid.c_str());
    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackup::GetRequestJsonBody(const mp_string &reqMsg, Json::Value &jsonBody)
{
    mp_int32 iRet = MP_FAILED;
    Json::Value jsonMsgContent;
    // Convert request content from string to json format
    iRet = CJsonUtils::ConvertStringtoJson(reqMsg, jsonMsgContent);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Convert string to json format failed.");
        return iRet;
    }

    // Ensure the message body exists
    if (!jsonMsgContent.isObject() || !jsonMsgContent.isMember(MANAGECMD_KEY_BODY)) {
        ERRLOG("Request message content has no attr 'body'");
        return ERROR_COMMON_INVALID_PARAM;
    } else {
        jsonBody = jsonMsgContent[MANAGECMD_KEY_BODY];
        iRet = MP_SUCCESS;
    }

    return iRet;
}

// set backend storage protocol type
mp_int32 VMwareNativeBackup::SetStorageProtocolForTask(const mp_string &id, const Json::Value &param)
{
    mp_int32 iStorageProtocol = VMWAREDEF::VMWARE_STORAGE_PROTOCOL_ISCSI;
    if (!param.isObject() || !param.isMember(VMWAREDEF::PARAM_STORAGE_STR)) {
        ERRLOG("The request message has no key: '%s', task id '%s'.", VMWAREDEF::PARAM_STORAGE_STR.c_str(), id.c_str());
        return MP_FAILED;
    }
    if (param[VMWAREDEF::PARAM_STORAGE_STR].isObject() &&
        param[VMWAREDEF::PARAM_STORAGE_STR].isMember(PARAM_NAS_OVER_FC_STR) &&
        param[VMWAREDEF::PARAM_STORAGE_STR][PARAM_NAS_OVER_FC_STR].isBool()) {
        INFOLOG("received accessNASOverFC: %d, id:%s",
            param[VMWAREDEF::PARAM_STORAGE_STR][PARAM_NAS_OVER_FC_STR].asBool(), id.c_str());
    }
    Json::Value mediaInfo = param[VMWAREDEF::PARAM_STORAGE_STR];
    if (mediaInfo.isObject() && mediaInfo.isMember(VMWAREDEF::PARAM_STORAGE_PROTOCOL_STR)) {
        GET_JSON_INT32(mediaInfo, VMWAREDEF::PARAM_STORAGE_PROTOCOL_STR, iStorageProtocol);
        if (CalibrationFormatStorProtocol(iStorageProtocol) == MP_FAILED) {
            return MP_FAILED;
        }
    }
    TaskContext::GetInstance()->SetValueInt32(id, KEY_STORAGE_PROTOCOL, iStorageProtocol);
    return MP_SUCCESS;
}

// set disk type
mp_int32 VMwareNativeBackup::SetDiskTypeForTask(const mp_string &id, const Json::Value &param)
{
    mp_string diskType = VMWAREDEF::VMWARE_NORMAL_DISK;
    if (!param.isObject() || !param.isMember(VMWAREDEF::PARAM_DISKTYPE_STR)) {
        ERRLOG("The request message has no key: '%s', task id '%s'.", VMWAREDEF::PARAM_DISKTYPE_STR.c_str(),
            id.c_str());
        return MP_FAILED;
    }
    GET_JSON_STRING(param, VMWAREDEF::PARAM_DISKTYPE_STR, diskType);
    if (diskType != VMWAREDEF::VMWARE_NORMAL_DISK && diskType != VMWAREDEF::VMWARE_RDM_DISK) {
        ERRLOG("Disk type error, disk type : '%s'.", diskType.c_str());
        return MP_FAILED;
    }

    TaskContext::GetInstance()->SetValueString(id, KEY_DISK_TYPE, diskType);
    return MP_SUCCESS;
}

mp_void VMwareNativeBackup::RemoveTask(const mp_string &taskId)
{
    // remove task related attrs from cache
    TaskContext::GetInstance()->RemoveTaskContext(taskId);

    // remove task from cache of taskmanager
    Task *pTask = NULL;
    TaskManager::GetInstance()->FindTask(taskId, pTask);
    if (pTask != NULL) {
        TaskManager::GetInstance()->RemoveTaskFromMap(taskId);
    }
    INFOLOG("Remove resources of task '%s' successfully!", taskId.c_str());
}

void VMwareNativeBackup::GetAgentMgrIp(std::string &agentMgrIp)
{
    std::vector<string> ips;
    std::string strCmd = "ifconfig|grep -w inet|grep -v '127.0.0.1'|head -n 1|awk '{print $2}'";
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, ips);
    if (iRet != MP_SUCCESS || ips.size() == 0) {
        COMMLOG(OS_LOG_WARN, "Get first ip failed.");
        return;
    }
    agentMgrIp = ips[0];
    INFOLOG("Get mgr ip ok '%s'", agentMgrIp.c_str());
}