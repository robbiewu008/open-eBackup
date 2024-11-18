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
#include "plugins/vmwarenative/VMwareNativeBackupPlugin.h"
#include "message/tcp/CDppMessage.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "common/ErrorCode.h"
#include "common/ConfigXmlParse.h"
#include "host/host.h"
using namespace std;

REGISTER_PLUGIN(VMwareNativeBackupPlugin);
VMwareNativeBackupPlugin::VMwareNativeBackupPlugin()
{
    // backup related CMDs
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_PREPARE_BACKUP, &VMwareNativeBackupPlugin::PrepareVMwareNativeVmBackup);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_OPENDISK_BACKUP, &VMwareNativeBackupPlugin::OpenDiskVMwareNativeVmBackup);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_RUN_BACKUP, &VMwareNativeBackupPlugin::RunVMwareNativeVmBackup);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_QUERY_BACKUP_PROGRESS, &VMwareNativeBackupPlugin::QueryVMwareNativeBackupProgress);

    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_BACKUP, &VMwareNativeBackupPlugin::FinishDiskDataBlockBackup);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_FINISH_BACKUP_TASK, &VMwareNativeBackupPlugin::FinishVmwareNativeVmBackupAction);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_CANCEL_BACKUP_TASK, &VMwareNativeBackupPlugin::CancelVMwareNativeVmBackupAction);

    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_ALLDISK_AFS_BITMAP, &VMwareNativeBackupPlugin::PrepareAllDisksAfsBitmap);

    // recovery related CMDs
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_PREPARE_RECOVERY, &VMwareNativeBackupPlugin::PrepareVMwareNativeVmRecovery);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_VMWARENATIVE_RUN_RECOVERY, &VMwareNativeBackupPlugin::RunVMwareNativeVmRecovery);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_VMWARENATIVE_QUERY_RECOVERY_PROGRESS,
        &VMwareNativeBackupPlugin::QueryVMwareNativeRecoveryProgress);

    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_RECOVERY, &VMwareNativeBackupPlugin::FinishDiskDataBlockRecovery);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_FINISH_RECOVERY_TASK, &VMwareNativeBackupPlugin::FinishVmwareNativeVmRecoveryAction);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_CANCEL_RECOVERY_TASK, &VMwareNativeBackupPlugin::CancelVMwareNativeVmRecoveryAction);

    // common CMDs
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_VMWARENATIVE_INIT_VDDKLIB, &VMwareNativeBackupPlugin::InitVMwareNativeVddkLib);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_CLEANUP_VDDKLIB, &VMwareNativeBackupPlugin::CleanupVMwareNativeVddkLib);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_CLEANUP_RESOURCES, &VMwareNativeBackupPlugin::CleanupVMwareNativeResources);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_CHECK_VMFS_TOOL, &VMwareNativeBackupPlugin::VmnativeVmfsCheckTool);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_VMFS_MOUNT, &VMwareNativeBackupPlugin::VmnativeVmfsMount);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_VMFS_UMOUNT, &VMwareNativeBackupPlugin::VmnativeVmfsUmount);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_SLNAS_MOUNT, &VMwareNativeBackupPlugin::VmnativeStorageLayerNasMount);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_VMWARENATIVE_SLNAS_UMOUNT, &VMwareNativeBackupPlugin::VmnativeStorageLayerNasUnMount);
    REGISTER_DPP_ACTION(
        MANAGE_CMD_NO_HOST_LINK_ISCSI, &VMwareNativeBackupPlugin::VmnativeLoginiScsiTarget);
}
VMwareNativeBackupPlugin::~VMwareNativeBackupPlugin()
{}

/**
 * 注册命令和函数的对应关系，后面命令分配时，插件管理器查询插件是否支持该命令，找到对应的插件并调用对应的函数
 */
mp_int32 VMwareNativeBackupPlugin::Init(vector<mp_uint32> &cmds)
{
    // 初始化支持的命令列表
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_INIT_VDDKLIB);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_CLEANUP_VDDKLIB);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_PREPARE_BACKUP);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_OPENDISK_BACKUP);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_RUN_BACKUP);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_QUERY_BACKUP_PROGRESS);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_BACKUP);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_FINISH_BACKUP_TASK);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_CANCEL_BACKUP_TASK);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_PREPARE_RECOVERY);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_RUN_RECOVERY);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_QUERY_RECOVERY_PROGRESS);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_RECOVERY);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_FINISH_RECOVERY_TASK);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_CANCEL_RECOVERY_TASK);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_CLEANUP_RESOURCES);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_CHECK_VMFS_TOOL);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_VMFS_MOUNT);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_VMFS_UMOUNT);

    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_SLNAS_MOUNT);
    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_SLNAS_UMOUNT);

    cmds.push_back(MANAGE_CMD_NO_HOST_LINK_ISCSI);

    cmds.push_back(MANAGE_CMD_NO_VMWARENATIVE_ALLDISK_AFS_BITMAP);
    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackupPlugin::DoAction(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    DO_DPP_ACTION(VMwareNativeBackupPlugin, reqMsg, rspMsg);
}

EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::InitVMwareNativeVddkLib(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter InitVMwareNativeVddkLib...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.VerifyVcenterCert(reqMsg.GetBuffer(), strTaskID);
    rspBody[MANAGECMD_KEY_ERRORCODE] = iRet;
    if (iRet != MP_SUCCESS) {
        strError = "Check Vcenter Certificate failed!";
        ERRLOG("Check Vcenter Certificate failed!");
    } else {
        iRet = m_vmwareNativeBackup.InitVddkLib(reqMsg.GetBuffer(),
            reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
        rspBody[MANAGECMD_KEY_ERRORCODE] = iRet;
        if (iRet != MP_SUCCESS) {
            strError = "Init VDDK lib failed!";
            ERRLOG("Init VDDK lib failed!");
        }
    }
    mp_uint64 ifKeepVmsnapOnFail = 0;
    if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueUint64(CFG_BACKUP_SECTION,
        CFG_IF_KEEP_VMSNAP_ON_FAIL, ifKeepVmsnapOnFail)) {
        strError = "get CFG_IF_KEEP_VMSNAP_ON_FAIL failed!";
        ERRLOG("get CFG_IF_KEEP_VMSNAP_ON_FAIL failed! set 0 as default");
        ifKeepVmsnapOnFail = 0;
    } else {
        INFOLOG("get CFG_IF_KEEP_VMSNAP_ON_FAIL secc. ifKeepVmsnapOnFail=%d", ifKeepVmsnapOnFail);
    }
    rspBody["ifKeepVmsnapOnFail"] = static_cast<Json::UInt64>(ifKeepVmsnapOnFail);
    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_INIT_VDDKLIB_ACK, strTaskID, rspBody);
    INFOLOG("Exit InitVMwareNativeVddkLib...");

    return iRet;
}

// uninit vddk at vm level task
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::CleanupVMwareNativeVddkLib(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter CleanupVMwareNativeVddkLib...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.CleanupVddkLib(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    rspBody[MANAGECMD_KEY_ERRORCODE] = iRet;
    if (iRet != MP_SUCCESS) {
        strError = "Cleanup VDDK lib failed!";
        ERRLOG("Cleanup VDDK lib failed!");
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_CLEANUP_VDDKLIB_ACK, strTaskID, rspBody);
    INFOLOG("Exit CleanupVMwareNativeVddkLib...");

    return iRet;
}

// prepare for vm backup based on a sync task
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::PrepareVMwareNativeVmBackup(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter PrepareVMwareNativeVmBackup...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.PrepareBackup(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);

    rspBody[MANAGECMD_KEY_ERRORCODE] = iRet;
    if (iRet != MP_SUCCESS) {
        strError = "Prepare vmware native backup failed!";
        ERRLOG("Prepare vmware native backup failed!");
    }
    std::string agentMgrIp;
    m_vmwareNativeBackup.GetAgentMgrIp(agentMgrIp);

    mp_uint64 segSize = 0;
    CConfigXmlParser::GetInstance().GetValueUint64(CFG_DATAPROCESS_SECTION, CFG_BACKUP_SEGMENT_SIZE, segSize);
    rspBody["body"]["segmentSize"] = static_cast<Json::UInt64>(segSize);
    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    rspBody["body"][MANAGECMD_KEY_ERRORCODE] = rspBody[MANAGECMD_KEY_ERRORCODE];
    rspBody["body"][MANAGECMD_KEY_ERRORDETAIL] = rspBody[MANAGECMD_KEY_ERRORDETAIL];

    rspBody["body"][PARAM_KEY_LOGLABEL] = rspBody[PARAM_KEY_LOGLABEL];
    rspBody["body"][PARAM_KEY_LOGLABELPARAM] = rspBody[PARAM_KEY_LOGLABELPARAM];
    rspBody["body"][PARAM_KEY_LOGDETAIL] = rspBody[PARAM_KEY_LOGDETAIL];
    rspBody["body"][PARAM_KEY_LOGPARAMS] = rspBody[PARAM_KEY_LOGPARAMS];

    rspBody["body"]["agentMgrIp"] = agentMgrIp;
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_PREPARE_BACKUP_ACK, strTaskID, rspBody);
    Json::Value tmprspBody;
    rspMsg.GetManageBody(tmprspBody);
    INFOLOG("Exit PrepareVMwareNativeVmBackup... rspMsg=%s", tmprspBody.toStyledString().c_str());

    return iRet;
}

EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::OpenDiskVMwareNativeVmBackup(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError;

    iRet = m_vmwareNativeBackup.OpenDiskBackup(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody, strError);
    if (iRet != MP_SUCCESS) {
        if (strError.empty()) {
            strError = "Open disk vmware native backup failed!";
        }
        ERRLOG("Open disk vmware native backup failed!");
    }
    rspBody[MANAGECMD_KEY_ERRORCODE] = iRet;
    rspBody[MANAGECMD_KEY_ERRORDETAIL] = strError;

    CheckAndUpdateResponseMsg(rspBody["body"], iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_OPENDISK_BACKUP_ACK, strTaskID, rspBody);

    return iRet;
}

// cleanup resources for disk level task which is a sync task
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::CleanupVMwareNativeResources(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter CleanupVMwareNativeResources...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.CleanupResources(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    rspBody[MANAGECMD_KEY_ERRORCODE] = iRet;
    if (iRet != MP_SUCCESS) {
        strError = "Cleanup vmware native vm resources failed!";
        ERRLOG("Cleanup vmware native vm resources failed!");
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_CLEANUP_RESOURCES_ACK, strTaskID, rspBody);
    INFOLOG("Exit CleanupVMwareNativeResources...");

    return iRet;
}

// run vm backup
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::RunVMwareNativeVmBackup(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter RunVMwareNativeVmBackup...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.BackupDataBlocks(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        strError = "Run vmware native disk data block backup failed!";
        ERRLOG("Run vmware native disk data block backup failed!");
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_RUN_BACKUP_ACK, strTaskID, rspBody);
    INFOLOG("Exit RunVMwareNativeVmBackup...");

    return iRet;
}

EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::PrepareAllDisksAfsBitmap(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter PrepareAllDisksAfsBitmap...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError;

    iRet = m_vmwareNativeBackup.PrepareAfsBitmap(reqMsg.GetBuffer(), strTaskID, rspBody, strError);
    if (iRet != MP_SUCCESS) {
        if (strError.empty()) {
            strError = "Run vmware afs bitmap failed!";
        }
        ERRLOG("Run vmware afs bitmap failed, err:%s", strError.c_str());
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_ALLDISK_AFS_BIITMAP_ACK, strTaskID, rspBody);
    INFOLOG("Exit PrepareAllDisksAfsBitmap...");
    return iRet;
}


// query vm backup progress -- need use another thread
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::QueryVMwareNativeBackupProgress(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter QueryVMwareNativeBackupProgress...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.QueryDataBlockBackupProgress(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        strError = "Query vmware native disk data block backup progress failed!";
        ERRLOG("Query vmware native disk data block backup progress failed!");
    }

    CheckAndUpdateMsgForProgressQuery(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_QUERY_BACKUP_PROGRESS_ACK, strTaskID, rspBody);
    INFOLOG("Exit QueryVMwareNativeBackupProgress...");

    return iRet;
}

// finish vm's disk data block backup
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::FinishDiskDataBlockBackup(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter FinishDiskDataBlockBackup...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.FinishDataBlockBackup(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        strError = "Finish vmware native disk data block backup failed!";
        ERRLOG("Finish vmware native disk data block backup failed!");
    }

    Json::Value jsonBody = rspBody[MANAGECMD_KEY_BODY];
    if (jsonBody.isObject() && jsonBody.isMember("ResAttachedDiskPath")) {
        iRet = 0x5F02571A;
        INFOLOG("The residual attached strTaskID: '%s'.", strTaskID.c_str());
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_BACKUP_ACK, strTaskID, rspBody);
    INFOLOG("Exit FinishDiskDataBlockBackup...");

    return iRet;
}

// finish vm backup task
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::FinishVmwareNativeVmBackupAction(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter FinishVmwareNativeVmBackupAction...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.FinishVmBackupAction(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        strError = "Finish vmware native virtual machine backup action failed!";
        ERRLOG("Finish vmware native virtual machine backup action failed!");
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_FINISH_BACKUP_TASK_ACK, strTaskID, rspBody);
    INFOLOG("Exit FinishVmwareNativeVmBackupAction...");

    return iRet;
}

// cancel vm backup task
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::CancelVMwareNativeVmBackupAction(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter CancelVMwareNativeVmBackupAction...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.CancelVmBackupAction(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        strError = "Cancel vmware native virtual machine backup action failed!";
        ERRLOG("Cancel vmware native virtual machine backup action failed!");
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_CANCEL_BACKUP_TASK_ACK, strTaskID, rspBody);
    INFOLOG("Exit CancelVMwareNativeVmBackupAction...");

    return iRet;
}

// prepare for vm recovery
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::PrepareVMwareNativeVmRecovery(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter PrepareVMwareNativeVmRecovery...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.PrepareRecovery(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        rspBody[MANAGECMD_KEY_ERRORDETAIL] = "Prepare vmware native recovery failed!";
        ERRLOG("Prepare vmware native recovery failed!");
    }
    mp_uint64 segSize = 0;
    CConfigXmlParser::GetInstance().GetValueUint64(CFG_DATAPROCESS_SECTION, CFG_RESTORE_SEGMENT_SIZE, segSize);
    rspBody["body"]["segmentSize"] = static_cast<Json::UInt64>(segSize);
    rspBody["body"][MANAGECMD_KEY_ERRORCODE] = rspBody[MANAGECMD_KEY_ERRORCODE];
    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    rspBody["body"][MANAGECMD_KEY_ERRORCODE] = rspBody[MANAGECMD_KEY_ERRORCODE];
    rspBody["body"][MANAGECMD_KEY_ERRORDETAIL] = rspBody[MANAGECMD_KEY_ERRORDETAIL];
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_PREPARE_RECOVERY_ACK, strTaskID, rspBody);
    INFOLOG("Exit PrepareVMwareNativeVmRecovery...");

    return iRet;
}

// run vm recovery
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::RunVMwareNativeVmRecovery(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter RunVMwareNativeVmRecovery...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.RecoveryDataBlocks(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        strError = "Run vmware native data block recovery failed!";
        ERRLOG("Run vmware native data block recovery failed!");
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_RUN_RECOVERY_ACK, strTaskID, rspBody);
    INFOLOG("Exit RunVMwareNativeVmRecovery...");

    return iRet;
}

// query vm recovery progress
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::QueryVMwareNativeRecoveryProgress(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter QueryVMwareNativeRecoveryProgress...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.QueryDataBlockRecoveryProgress(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        strError = "Query vmware native data block recovery progress failed!";
        ERRLOG("Query vmware native data block recovery progress failed!");
    }

    CheckAndUpdateMsgForProgressQuery(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_QUERY_RECOVERY_PROGRESS_ACK, strTaskID, rspBody);
    INFOLOG("Exit QueryVMwareNativeRecoveryProgress...");

    return iRet;
}

// finish vm's disk data block recovery
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::FinishDiskDataBlockRecovery(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter FinishDiskDataBlockRecovery...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.FinishDataBlockRecovery(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        rspBody[MANAGECMD_KEY_ERRORDETAIL] = "Finish vmware native disk data block recovery action failed!";
        ERRLOG("Finish vmware native disk data block recovery action failed!");
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_FINISH_DISK_RECOVERY_ACK, strTaskID, rspBody);
    INFOLOG("Exit FinishDiskDataBlockRecovery...");

    return iRet;
}

// finish vm recovery task
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::FinishVmwareNativeVmRecoveryAction(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter FinishVmwareNativeVmRecoveryAction...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.FinishVmRecoveryAction(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        rspBody[MANAGECMD_KEY_ERRORDETAIL] = "Finish vmware native virtual machine recovery action failed!";
        ERRLOG("Finish vmware native virtual machine recovery action failed!");
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_FINISH_RECOVERY_TASK_ACK, strTaskID, rspBody);
    INFOLOG("Exit FinishVmwareNativeVmRecoveryAction...");

    return iRet;
}

// cancel vm recovery task
EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::CancelVMwareNativeVmRecoveryAction(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter CancelVMwareNativeVmRecoveryAction...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";

    iRet = m_vmwareNativeBackup.CancelVmRecoveryAction(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        strError = "Cancel vmware native virtual machine recovery action failed!";
        ERRLOG("Cancel vmware native virtual machine recovery action failed!");
    }

    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_CANCEL_RECOVERY_TASK_ACK, strTaskID, rspBody);
    INFOLOG("Exit CancelVMwareNativeVmRecoveryAction...");

    return iRet;
}

mp_int32 VMwareNativeBackupPlugin::SetResponseMsg(
    CDppMessage &rspMsg, mp_int32 status, mp_int32 cmd, const mp_string &taskId, Json::Value &rspBody)
{
    if (!rspBody.isObject()) {
        ERRLOG("Rsp body is not json object.");
        return MP_FAILED;
    }
    Json::Value rspContent;
    // parse body from response of dp service
    if (rspBody.isMember(MANAGECMD_KEY_BODY)) {
        rspBody = rspBody[MANAGECMD_KEY_BODY];
    }

    // ensure the outer body has attr 'errordetail'
    if (rspBody.isMember(MANAGECMD_KEY_ERRORDETAIL)) {
        mp_string errDetail;
        GET_JSON_STRING(rspBody, MANAGECMD_KEY_ERRORDETAIL, errDetail);
        rspContent[MANAGECMD_KEY_ERRORDETAIL] = errDetail;
    } else {
        rspContent[MANAGECMD_KEY_ERRORDETAIL] = "";
    }

    // add task id for each response body
    rspBody[MANAGECMD_KEY_TASKID] = taskId;
    rspContent[MANAGECMD_KEY_BODY] = rspBody;
    rspContent[MANAGECMD_KEY_CMDNO] = cmd;
    rspContent[MANAGECMD_KEY_ERRORCODE] = status;
    rspMsg.SetMsgBody(rspContent);
    return MP_SUCCESS;
}

mp_void VMwareNativeBackupPlugin::CheckAndUpdateResponseMsg(
    Json::Value &rspBody, mp_int32 errCode, const mp_string& errDesc)
{
    if (!rspBody.isObject() || !rspBody.isMember(MANAGECMD_KEY_ERRORDETAIL)) {
        rspBody[MANAGECMD_KEY_ERRORDETAIL] = errDesc;
    }

    if (!rspBody.isObject() || !rspBody.isMember(MANAGECMD_KEY_ERRORCODE)) {
        rspBody[MANAGECMD_KEY_ERRORCODE] = errCode;
    }
}

mp_void VMwareNativeBackupPlugin::CheckAndUpdateMsgForProgressQuery(
    Json::Value &rspBody, mp_int32 errCode, const mp_string &errDesc)
{
    CheckAndUpdateResponseMsg(rspBody, errCode, errDesc);
    // construct progress attrs
    if (!rspBody.isMember(PARAM_KEY_TASKDESC)) {
        rspBody[PARAM_KEY_TASKDESC] = "failure";
    }

    if (!rspBody.isMember(PARAM_KEY_TASKPROGRESS)) {
        rspBody[PARAM_KEY_TASKPROGRESS] = VMWAREDEF::DEFAULT_PROGRESS_VALUE;
    }

    if (!rspBody.isMember(PARAM_KEY_TASKSTATUS)) {
        rspBody[PARAM_KEY_TASKSTATUS] = VMWAREDEF::PROGRESS_QUERY_FAILURE_VALUE;
    }

    if (!rspBody.isMember(PARAM_KEY_DATA_TRANS_MODE)) {
        rspBody[PARAM_KEY_DATA_TRANS_MODE] = "";
    }
}

EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::VmnativeVmfsCheckTool(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter VmfsCheckTool...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";
    iRet = m_vmwareNativeBackup.VmfsCheckTool(
        reqMsg.GetBuffer(), strTaskID);
    if (iRet != MP_SUCCESS) {
        strError = "error when find VMFS TOOL!";
        ERRLOG("error when find VMFS TOOL!");
    }
    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_CHECK_VMFS_TOOL_ACK, strTaskID, rspBody);
    INFOLOG("Exit VmfsCheckTool...");
    return iRet;
}

EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::VmnativeVmfsMount(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter VmfsMount...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";
    mp_string retString = "";
    iRet = m_vmwareNativeBackup.VmfsMount(
        reqMsg.GetBuffer(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        strError = "error in VMFS Mount!";
        ERRLOG("error in VMFS Mount!");
    }
    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_VMFS_MOUNT_ACK, strTaskID, rspBody);
    INFOLOG("Exit VmfsMount...");
    return iRet;
}

EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::VmnativeVmfsUmount(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter VmfsUnmount...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";
    iRet = m_vmwareNativeBackup.VmfsUmount(reqMsg.GetBuffer(), strTaskID);
    if (iRet != MP_SUCCESS) {
        strError = "error in VMFS Unmount!";
        ERRLOG("error in VMFS Unmount!");
    }
    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_VMFS_UMOUNT_ACK, strTaskID, rspBody);
    INFOLOG("Exit VmfsUnmount...");
    return iRet;
}

EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::VmnativeStorageLayerNasMount(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter VmnativeStorageLayerNasMount...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";
    mp_string retString = "";
    iRet = m_vmwareNativeBackup.StorageLayerNasMount(
        reqMsg.GetBuffer(), strTaskID, rspBody);
    if (iRet != MP_SUCCESS) {
        strError = "error in StorageLayerNasMount!";
        ERRLOG("error in StorageLayerNasMount!");
    }
    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_SLNAS_MOUNT_ACK, strTaskID, rspBody);
    INFOLOG("Exit VmnativeStorageLayerNasMount...");
    return iRet;
}

EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::VmnativeStorageLayerNasUnMount(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter VmnativeStorageLayerNasUnMount...");
    mp_int32 iRet = MP_FAILED;
    mp_string strTaskID;
    Json::Value rspBody;
    mp_string strError = "";
    mp_string retString = "";
    iRet = m_vmwareNativeBackup.StorageLayerNasUnMount(
        reqMsg.GetBuffer(), strTaskID);
    if (iRet != MP_SUCCESS) {
        strError = "error in StorageLayerNasUnMount!";
        ERRLOG("error in StorageLayerNasUnMount!");
    }
    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_VMWARENATIVE_SLNAS_UMOUNT_ACK, strTaskID, rspBody);
    INFOLOG("Exit VmnativeStorageLayerNasUnMount...");
    return iRet;
}

EXTER_ATTACK mp_int32 VMwareNativeBackupPlugin::VmnativeLoginiScsiTarget(
    CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    INFOLOG("Enter VmnativeLoginiScsiTarget...");
    mp_int32 iRet = MP_FAILED;
    mp_string taskId;
    Json::Value rspBody;
    mp_string strError = "";
    mp_string retString = "";
    Json::Value jsonMsgBody;
    iRet = m_vmwareNativeBackup.GetTaskId(reqMsg.GetBuffer(), jsonMsgBody, taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unabel to get VmnativeLoginiScsiTarget task id '%s' from request message body, iRet=%d.",
            taskId.c_str(), iRet);
        return iRet;
    }
    CHost host;
    iRet = host.LinkiScsiTarget(jsonMsgBody);
    if (iRet != MP_SUCCESS) {
        ERRLOG("LinkiScsiTarget failed, task id '%s' from request message body, iRet=%d.",
            taskId.c_str(), iRet);
        strError = "error in VmnativeLoginiScsiTarget!";
    }
    CheckAndUpdateResponseMsg(rspBody, iRet, strError);
    SetResponseMsg(rspMsg, iRet, MANAGE_CMD_NO_HOST_LINK_ISCSI_ACK, taskId, rspBody);
    INFOLOG("Exit VmnativeLoginiScsiTarget...");
    return iRet;
}