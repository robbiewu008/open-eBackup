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
#ifndef __AGENT_VMWARE_NATIVE_BACKUP_PLUGIN_H__
#define __AGENT_VMWARE_NATIVE_BACKUP_PLUGIN_H__

#include <regex>
#include "apps/vmwarenative/VMwareNativeBackup.h"
#include "message/tcp/CDppMessage.h"
#include "plugins/ServicePlugin.h"

class VMwareNativeBackupPlugin : public CServicePlugin {
public:
    VMwareNativeBackupPlugin();
    virtual ~VMwareNativeBackupPlugin();

    mp_int32 Init(std::vector<mp_uint32> &cmds);
    mp_int32 DoAction(CDppMessage &reqMsg, CDppMessage &rspMsg);

private:
    // backup
    EXTER_ATTACK mp_int32 PrepareVMwareNativeVmBackup(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 OpenDiskVMwareNativeVmBackup(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 RunVMwareNativeVmBackup(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 QueryVMwareNativeBackupProgress(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 FinishDiskDataBlockBackup(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 FinishVmwareNativeVmBackupAction(
        CDppMessage &reqMsg, CDppMessage &rspMsg);  // discarded in multi-disk protection
    EXTER_ATTACK mp_int32 CancelVMwareNativeVmBackupAction(
        CDppMessage &reqMsg, CDppMessage &rspMsg);  // discarded in multi-disk protection

    // invalid data identify
    EXTER_ATTACK mp_int32 PrepareAllDisksAfsBitmap(CDppMessage &reqMsg, CDppMessage &rspMsg);

    // recovery
    EXTER_ATTACK mp_int32 PrepareVMwareNativeVmRecovery(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 RunVMwareNativeVmRecovery(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 QueryVMwareNativeRecoveryProgress(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 FinishDiskDataBlockRecovery(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 FinishVmwareNativeVmRecoveryAction(
        CDppMessage &reqMsg, CDppMessage &rspMsg);  // discarded in multi-disk protection
    EXTER_ATTACK mp_int32 CancelVMwareNativeVmRecoveryAction(
        CDppMessage &reqMsg, CDppMessage &rspMsg);  // discarded in multi-disk protection

    // common
    EXTER_ATTACK mp_int32 InitVMwareNativeVddkLib(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 CleanupVMwareNativeVddkLib(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 CleanupVMwareNativeResources(CDppMessage &reqMsg, CDppMessage &rspMsg);

    EXTER_ATTACK mp_int32 VmnativeVmfsCheckTool(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 VmnativeVmfsMount(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 VmnativeVmfsUmount(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 VmnativeStorageLayerNasMount(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 VmnativeStorageLayerNasUnMount(CDppMessage &reqMsg, CDppMessage &rspMsg);
    EXTER_ATTACK mp_int32 VmnativeLoginiScsiTarget(CDppMessage &reqMsg, CDppMessage &rspMsg);

private:
    mp_void CheckAndUpdateResponseMsg(
        Json::Value &rspBody, mp_int32 errCode, const mp_string &errDesc);

    mp_void CheckAndUpdateMsgForProgressQuery(
        Json::Value &rspBody, mp_int32 errCode, const mp_string &errDesc);

    mp_int32 SetResponseMsg(
        CDppMessage &rspMsg, mp_int32 status, mp_int32 cmd, const mp_string &taskId, Json::Value &rspBody);

    void SendAttachedDiskAlarm(const std::string &strDiskPath);

private:
    VMwareNativeBackup m_vmwareNativeBackup;
};

#endif
