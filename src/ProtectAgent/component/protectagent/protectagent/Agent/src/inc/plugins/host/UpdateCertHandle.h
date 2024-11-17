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
#ifndef UPDATE_CERT_HANDLE_H_
#define UPDATE_CERT_HANDLE_H_

#include <atomic>
#include <functional>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "common/Types.h"
#include "common/Defines.h"
#include "message/curlclient/CurlHttpClient.h"
#include "common/JsonHelper.h"
#include "message/curlclient/RestClientCommon.h"

#include "common/Path.h"
#include "common/Utils.h"
#include "message/rest/message_process.h"
#include "message/curlclient/PmRestClient.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/x509v3.h"
#include "openssl/x509.h"
#include "openssl/obj_mac.h"
#include "openssl/objects.h"
#include "openssl/pem.h"

static const mp_string PMCA_FILE = "pmCaCertificate";
static const mp_string UPDATE_TYPE = "type";
static const mp_string AGENTCA_FILE = "agentCaCertificate";
static const mp_string SERVE_FILE = "serverCertificate";
static const mp_string SERVER_KET_FILE = "serverKey";
static const mp_string SERVER_PASSWORD = "serverPass";
static const mp_string JOB_ID = "jobId";

struct CertUpdateInfo {
    mp_string pmcaFile;
    mp_string agentcaFile;
    mp_string serverFile;
    mp_string serverKeyFile;
    mp_string passWord;
    mp_int32 updateType;

    ~CertUpdateInfo()
    {
        if (!passWord.empty()) {
            ClearString(passWord);
        }
    }
};
class UpdateCertHandle {
public:
    ~UpdateCertHandle()
    {
        if (m_execUpdateCertThread != nullptr) {
            m_execUpdateCertThread->join();
            m_execUpdateCertThread.reset();
        }
    };

    static UpdateCertHandle& GetInstance()
    {
        static UpdateCertHandle m_Instance;
        return m_Instance;
    }
    static void StartAutoCheck();                // 启动自检，检查是否处于更新证书时发生的启动
    mp_int32 WriteInFile(const mp_string& filePath, const mp_string& FileContent);
    mp_int32 HandleUpdateRequest(CRequestMsg& req);
    mp_int32 FallbackCertHandle(CRequestMsg& req);
    mp_int32 FallbackCert(CRequestMsg& req);          // 回退功能实现

    mp_int32 PushNewCert(CRequestMsg& req);           // 支持推送证书，参数解析，证书校验
    mp_int32 RequestFallbackCert();     // PM调用 回退接口
    mp_int32 QueryNetworkToAgent();     // PM检查agent更换证书后的连通性
    mp_int32 CleanCertFilesHandle(CRequestMsg& req);       // 清理证书
    mp_int32 PmNotifyAgentUseNewCert();

private:
    UpdateCertHandle()
    {}

    bool CheckIfInUpdatingCert();
    bool CreateUpdateThread();
    bool CreateRollbackThread();

    // Cenectivity Check
    void CheckNewCertConnectivity();
    mp_void WaitPmNotify();
    bool CheckPmToAgentStatus();      // 根据存盘状态检查pm -> agent
    mp_int32 CheckConnectionToPM();

    mp_int32 InitailizePath();
    mp_int32 CheckRequest(CRequestMsg& req);             // 检查证书大小
    bool AddCertCNToHosts();      // 检查证书域名
#ifdef WIN32
    mp_int32 WriteCertCNToWindowsHosts(mp_string& cnNameOfServerCrt);
#else
    mp_int32 WriteCertCNToNoWindowsHosts(mp_string& cnNameOfServerCrt);
#endif
    mp_int32 CheckIfSafe();
    bool checkCipherSupport(SSL* ssl, const char* cipherName);
    mp_int32 CheckBeforeUpdate(mp_int32 certType);     // 前置检查
    mp_int32 UpdateCert();            // 调用脚本更新，更新成功后先不要删除备份的旧证书
    mp_int32 ParseFile();     // 解析消息体中传来证书文件信息
    // thread func
    void ExecUpdateCert();
    mp_int32 ExecCleanTempFiles();
    mp_int32 ExecRollbackCert();
    mp_int32 CheckJobID(CRequestMsg& req);
    bool GetJobidInTmpFile(mp_string& resJobid);
    mp_bool CheckIfExistHandle(mp_string &jobId);
    mp_bool CheckIfJobidEqual(mp_string &jobId);
#ifndef WIN32
    void ChangeFilePermission(const mp_string& fileName);
#endif

private:
    CertUpdateInfo m_certInfo;
    mp_string m_jobId;
    mp_int32 m_certType = 0;
    std::atomic<bool> m_connectAgentToPm { false };
    std::atomic<bool> m_connectPmToAgent { false };
    mp_string m_certUpdateDir;
    mp_string m_certPathOld;
    mp_string m_certPathNew;

    std::unique_ptr<std::thread> m_execUpdateCertThread;
    std::unique_ptr<std::thread> m_execRollbackCertThread;
    std::atomic<bool> m_execTheadRunningFlag { false };

    std::condition_variable m_pnCond;
    std::mutex m_pnMutex;
};

#endif    // _UPDATE_CERT_HANDLE_H_