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
#ifndef AGENT_VMWARENATIVE_BACKUP
#define AGENT_VMWARENATIVE_BACKUP

#include "apps/vmwarenative/VMwareDef.h"
#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/value.h"
#include <vector>
#include "curl/curl.h"

typedef struct VerifyVcenter {
    mp_string strIP = "";
    mp_uint32 uintProt = 0;
    mp_bool bCRLIsEmpty = MP_TRUE;
    mp_bool bTLSCompatible = MP_FALSE;
    mp_string strTmpVcenterCAFile;
    mp_string strTmpVcenterCRLFile;
    mp_string strVerifyURL;
    std::vector<mp_string> vecCAInfo;
    std::vector<mp_string> vecCRLInfo;
}VerifyVcenterParam;

class VMwareNativeBackup {
public:
    VMwareNativeBackup();
    virtual ~VMwareNativeBackup();

    // backup operations
    mp_int32 PrepareBackup(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId,
        Json::Value &respBody);
    mp_int32 OpenDiskBackup(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId,
        Json::Value &respBody, mp_string &strError);
    mp_int32 BackupDataBlocks(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId,
        Json::Value &respBody);
    mp_int32 QueryDataBlockBackupProgress(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort,
        mp_string &taskId, Json::Value &respBody);
    mp_int32 FinishDataBlockBackup(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort,
        mp_string &taskId, Json::Value &respBody);
    mp_int32 FinishVmBackupAction(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort,
        mp_string &taskId, Json::Value &respBody);
    mp_int32 CancelVmBackupAction(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort,
        mp_string &taskId, Json::Value &respBody);

    mp_int32 PrepareAfsBitmap(const mp_string &msgBody, mp_string &taskId, Json::Value &respBody, mp_string &strError);

    // restore operations
    mp_int32 PrepareRecovery(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId,
        Json::Value &respBody);
    mp_int32 RecoveryDataBlocks(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort,
        mp_string &taskId, Json::Value &respBody);
    mp_int32 QueryDataBlockRecoveryProgress(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort,
        mp_string &taskId, Json::Value &respBody);
    mp_int32 FinishDataBlockRecovery(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort,
        mp_string &taskId, Json::Value &respBody);
    mp_int32 FinishVmRecoveryAction(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort,
        mp_string &taskId, Json::Value &respBody);
    mp_int32 CancelVmRecoveryAction(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort,
        mp_string &taskId, Json::Value &respBody);

    // common operations
    mp_int32 VerifyVcenterCert(const mp_string &msgBody, mp_string &taskId);
    mp_int32 Parsejson(const mp_string &msgBody, mp_string &taskId, VerifyVcenterParam &VerifyParam);
    mp_int32 PreVerify(VerifyVcenterParam &VerifyParam);
    mp_int32 UseCurl2VerifyVcenter(const mp_string &taskId, const VerifyVcenterParam &VerifyParam);
    mp_int32 SetCurlOption(const VerifyVcenterParam &VerifyParam, CURL* m_Curl);
    mp_int32 CleanUpTmpCertFile(const VerifyVcenterParam &VerifyParam);
    mp_int32 InitVddkLib(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId,
        Json::Value &respBody);
    mp_int32 CleanupVddkLib(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId,
        Json::Value &respBody);
    mp_int32 CleanupResources(const mp_string &msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string &taskId,
        Json::Value &respBody);
    mp_int32 VmfsCheckTool(const mp_string &msgBody, mp_string &taskId);
    mp_int32 VmfsMount(const mp_string &msgBody, mp_string &taskId, Json::Value &respBody);
    mp_int32 VmfsUmount(const mp_string &msgBody, mp_string &taskId);
    mp_bool AgentRestartRecently();
    mp_int32 GetStorageLayerNasMountParams(const Json::Value &jsonMsgBody, mp_string &storeIp,
        mp_string &sharePath, mp_string &nasStorageType, mp_string &diskId);
    mp_int32 StorageLayerNasMount(const mp_string &msgBody, mp_string &taskId, Json::Value &respBody);
    mp_int32 StorageLayerNasUnMount(const mp_string &msgBody, mp_string &taskId);
    void GetAgentMgrIp(std::string& agentMgrIp);
    mp_int32 GetTaskId(const mp_string &reqMsg, Json::Value &jsonBody, mp_string &taskid);
private:
    mp_void RemoveTask(const mp_string &taskId);
    mp_int32 GetRequestJsonBody(const mp_string &msgBody, Json::Value &bodyAttr);
    mp_int32 SetStorageProtocolForTask(const mp_string &id, const Json::Value &param);
    mp_int32 SetDiskTypeForTask(const mp_string &id, const Json::Value &param);
};

#endif
