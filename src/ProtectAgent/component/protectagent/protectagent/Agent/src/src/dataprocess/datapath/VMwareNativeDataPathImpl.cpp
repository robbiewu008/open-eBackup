/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareNativeDataPathImpl.cpp
 * @brief  Contains function declarations Vmware vddk opeartions
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "dataprocess/datapath/VMwareNativeDataPathImpl.h"
#include <map>
#include <iostream>
#include "common/File.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/ScopeExit.h"
#include "common/CSystemExec.h"
#include "common/Ip.h"
#include "common/Path.h"

#include "dataprocess/ioscheduler/IOTask.h"
#include "dataprocess/ioscheduler/VMwareIOEngine.h"
#include "dataprocess/ioscheduler/FileIOEngine.h"
#include "dataprocess/ioscheduler/VmfsIO.h"
#include "dataprocess/ioscheduler/Vmfs5IOEngine.h"
#include "dataprocess/ioscheduler/Vmfs6IOEngine.h"
#include "dataprocess/jobqosmanager/JobQosManager.h"
#include "dataprocess/datapath/AsioDataMoverForBackup.h"
#include "dataprocess/datapath/AsioDataMoverForRestore.h"

using namespace NSVmfsIO;

namespace {
const mp_int32 COMPLETED_PROGRESS = 100;
const mp_double ONE_USEC = 1000.0;
const mp_uint64 MAX_DISK_SIZE = 0x800000000000;  // max disk size 128T
const mp_uint64 DT_DATA_BLOCK = 0x400000;        // 4MB DirtyRange data block size
const mp_int32 PORT_RANGE = 100000;
const mp_int32 SNAP_TYPE_RANGE = 2;
const mp_int32 PROTOCOL_RANGE = 2;  // 0-http 1-https
const mp_int32 BLOCK_SIZE_KB = 4096;
const mp_uint32 UTILS_NUM_VMNAME_MAX_LEN = 128;
const mp_uint32 UTILS_NUM_FOTMAT_STRING_MAX_LEN = 512;
const mp_uint32 MAX_THREAD_NUM = 50;
const int SLEEP_TIME_INTERVAL = 20000;
const int RETRY_COUNT = 5;
const mp_string VMWARE_TRANSMODE_STORAGE_SAN = "storage san access";
const mp_string VMWARE_TRANSMODE_STORAGE_LUN = "storage lun access";
const mp_string VMWARE_TRANSMODE_STORAGE_NAS = "storage nas access";
const mp_string VMWARE_TRANSMODE_RDM = "rdm";
const std::unordered_map<mp_uint32, std::string> TRANSPORT_MODE_MAP = {
{VMWAREDEF::TRANSPORT_MODE_SAN, "san"},
{VMWAREDEF::TRANSPORT_MODE_HOT_ADD, "hotadd"},
{VMWAREDEF::TRANSPORT_MODE_NBDSSL, "nbdssl"}
};
}  // namespace

VMwareNativeDataPathImpl::VMwareNativeDataPathImpl() : m_reader(nullptr), m_writer(nullptr)
{
    m_iDirtyRangeSize = -1;
    m_protectType = -1;
    m_snapType = VMWARE_VM_FULL_BACKUP;
    m_isDiskOpened = false;
    m_backendStorageProtocol = -1;
    m_completedBlocks = 0;
    m_zeroBlocks.store(0);
    m_totalCompletedBlocks.store(0);
    m_reducedBlocks.store(0);
    m_preDataSizeFinished = 0;
    m_preTime = 0;
    m_nowSpeed = 0;
    m_progress = 0;
    m_isTaskSuccess = true;
    m_exitFlag = false;
    m_isClose = false;
    m_systemVirt = 0;
}

VMwareNativeDataPathImpl::~VMwareNativeDataPathImpl()
{
    m_iDirtyRangeSize = -1;
    m_protectType = -1;
    m_snapType = VMWARE_VM_FULL_BACKUP;
    m_isDiskOpened = false;
    m_backendStorageProtocol = -1;
    m_completedBlocks = 0;
    m_zeroBlocks.store(0);
    m_totalCompletedBlocks.store(0);
    m_reducedBlocks.store(0);
    m_reader.reset();
    m_writer.reset();
    m_systemVirt = 0;
    ClearString(m_vddkConnParams.password);
    ClearString(m_vmProtectionParams.pmInfo.strPassword);
}

mp_int32 VMwareNativeDataPathImpl::VMwareNativeVddkInit(Json::Value &bodyMsg, mp_bool vddkInited)
{
    // 1. parse protection params: task/parent task id, vm, product manager, backend storage
    if (InitVmProtectionParams(bodyMsg) != MP_SUCCESS) {
        ERRLOG("Unable to init vm protection parameters, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    // check rdm disktype
    mp_string strVmDiskType;
    if (ParseDiskType(bodyMsg, strVmDiskType) == MP_SUCCESS && strVmDiskType == "rdm") {
        INFOLOG("disktype is rdm disk, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        // rdm disk, skip init vddk
        return MP_SUCCESS;
    }
    // 2. init vddk connection params
    if (VMwareDiskLib::GetInstance()->BuildConnectParams(
        m_vmProtectionParams.vmInfo.strVmRef, m_vmProtectionParams.pmInfo, m_vddkConnParams) != MP_SUCCESS) {
        ERRLOG("Unable to generate vddk connection parameters, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }
    INFOLOG(
        "The VDDK connection params are: vm '%s', moRef '%s', task '%s', parent task '%s'.",
        m_vmProtectionParams.vmInfo.strVmName.c_str(),
        m_vmProtectionParams.vmInfo.strVmRef.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    // 3. init vddk lib
    if (InitVddkLib(bodyMsg, vddkInited) != MP_SUCCESS) {
        ERRLOG(
            "Unable to init VDDK lib,  task '%s', parent task '%s'.", m_strTaskID.c_str(), m_strParentTaskID.c_str());
        return MP_FAILED;
    }
    INFOLOG(
        "The VDDK lib has been initialized successfully,  task '%s', parent task '%s'.",
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::VMwareNativeVddkCleanup(Json::Value &bodyMsg)
{
    INFOLOG(
        "Protect of vmware vm has been completed, will try to release all resources, task '%s', parent task '%s'.",
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    // 1. parse protection params: task/parent task id, vm, product manager, backend storage
    if (InitVmProtectionParams(bodyMsg) != MP_SUCCESS) {
        ERRLOG(
            "Unable to init vm protection parameters, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    // check rdm disktype
    mp_string strVmDiskType;
    if (ParseDiskType(bodyMsg, strVmDiskType) == MP_SUCCESS && strVmDiskType == "rdm") {
        INFOLOG("disktype is rdm disk, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        // rdm disk, skip init vddk
        return MP_SUCCESS;
    }

    // 2. init vddk connection params
    if (VMwareDiskLib::GetInstance()->BuildConnectParams(
        m_vmProtectionParams.vmInfo.strVmRef, m_vmProtectionParams.pmInfo, m_vddkConnParams) != MP_SUCCESS) {
        ERRLOG("Unable to generate vddk connection parameters, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    mp_string strVddkLibPath;
    if (InitVddkLibPath(bodyMsg, strVddkLibPath) != MP_SUCCESS) {
        ERRLOG("Unable to init vddk lib path, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }
    // stoi转换前要判断字符是否符合要求
    if (m_vmProtectionParams.pmInfo.strVersion.empty() || m_vmProtectionParams.pmInfo.strVersion[0] < '0' ||
        m_vmProtectionParams.pmInfo.strVersion[0] > '9') {
        ERRLOG("The version is invalid, task '%s', parent task '%s', version: '%s'.",
            m_strTaskID.c_str(), m_strParentTaskID.c_str(), m_vmProtectionParams.pmInfo.strVersion.c_str());
        return MP_FAILED;
    }
    VMwareDiskLib::GetInstance()->SetVddkLibPathAndVersion(strVddkLibPath,
        std::stoi(m_vmProtectionParams.pmInfo.strVersion.substr(0, 1)),
        VMWAREDEF::VMWARE_VDDK_MINJOR_VERSION,
        m_vmProtectionParams.pmInfo.strVersion);

    InnerCleanup();
    return MP_SUCCESS;
}

// parse backend storage info, vm info and backup level
mp_int32 VMwareNativeDataPathImpl::VMwareNativePreparation(Json::Value &bodyMsg)
{
    Json::Value jsonBodyContent = bodyMsg[MANAGECMD_KEY_BODY];

    // 1.parse backend storage type: 0 - nas, 1 - iscsi
    if (ParseStorageType(jsonBodyContent) != MP_SUCCESS) {
        ERRLOG("Unable to obtain backend storage type, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    // 2. parse the actual backup level
    if (!jsonBodyContent.isObject() || !jsonBodyContent.isMember(PARAM_KEY_SNAPTYPE)) {
        COMMLOG(OS_LOG_WARN,
            "The request message has no key: '%s', task '%s', parent task '%s'.",
            PARAM_KEY_SNAPTYPE.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    } else {
        GET_JSON_UINT64(jsonBodyContent, PARAM_KEY_SNAPTYPE, m_vmProtectionParams.ulSnapType);
        if (VerifyParamsSize(m_vmProtectionParams.ulSnapType, SNAP_TYPE_RANGE, PARAM_KEY_SNAPTYPE) == MP_FAILED) {
            ERRLOG("Prepare backup SnapType=%I64d is out of Range", m_vmProtectionParams.ulSnapType);
            return MP_FAILED;
        }
    }

    // 3. parse the vm snapshot moref
    if (ParseVmSnapshotRef(jsonBodyContent) != MP_SUCCESS) {
        ERRLOG("Unable to obtain vm snapshot moRef, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    INFOLOG(
        "Perform vmware backup preparation successfully, storage type '%d', backup level '%u', \
        task '%s', parent task '%s'.",
        m_backendStorageProtocol,
        m_vmProtectionParams.ulSnapType,
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::TargetLunPrepare(Json::Value &bodyMsg)
{
    // parse all storage lun(s) mounted
    if (MP_SUCCESS != ParseStorageLunMounted(bodyMsg)) {
        ERRLOG("Unable to parse storage lun(s) mounted, task '%d', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }
    INFOLOG(
        "All storage lun mounted on host agent have been discovered, task '%s', parent task '%s'.",
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::UpdateSubTaskQos()
{
    if (m_volumeInfo.uLimitSpeed == 0) {
        m_isQosLimitSpeed = false;
        return MP_FAILED;
    }
    JobQosManager::GetJobQosManager(m_strTaskID)->SetJobQos(m_volumeInfo.uLimitSpeed);
    JobQosManager::GetJobQosManager(m_strTaskID)->StartQosLimit();
    m_isQosLimitSpeed = true;
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::VMwareVmBackupCancel(Json::Value &bodyMsg)
{
    INFOLOG(
        "Backup of disk '%s' has been canceled, will try to close the disk, task '%s', parent task '%s'.",
        m_volumeInfo.strDiskID.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    bodyMsg.clear();
    // reset var shared
    m_completedBlocks.store(0);
    m_zeroBlocks.store(0);
    m_totalCompletedBlocks.store(0);
    m_reducedBlocks.store(0);

    if (m_isDiskOpened) {
        if (MP_SUCCESS != CloseDisk()) {
            ERRLOG("Unable to close disk '%s', task '%s', parent task '%s'.",
                m_volumeInfo.strDiskID.c_str(),
                m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
            return MP_FAILED;
        }
        m_isDiskOpened = false;
    }

    if (m_volumeInfo.uLimitSpeed != 0) {
        JobQosManager::UnRegisterQos(m_strTaskID);
    }

    INFOLOG("Backup of vmware vm '%s' has been completed successfully, task '%s', parent task '%s'.",
        m_volumeInfo.strDiskPath.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    return MP_SUCCESS;
}

mp_uint64 VMwareNativeDataPathImpl::GetDiskSpeed(mp_int32 progress)
{
    if (m_progress == COMPLETED_PROGRESS) {
        INFOLOG("now progress is 100");
        return m_nowSpeed;
    }
    if (m_preTime == 0 || m_iDirtyRangeSize == 0) {
        WARNLOG("taskID %s pre time is %d and range size is %d", m_strTaskID.c_str(), m_preTime, m_iDirtyRangeSize);
        m_preTime = CMpTime::GetTimeUsec();
        return 0;
    }

    uint64_t finishedSize = m_completedBlocks.load() * BLOCK_SIZE_KB;
    uint64_t nowTime = CMpTime::GetTimeUsec();
    uint64_t usedTime = nowTime - m_preTime;
    m_nowSpeed = (finishedSize - m_preDataSizeFinished) / (usedTime / (ONE_USEC * ONE_USEC));
    INFOLOG("taskID %s finishedSize is %lld, prefinishedSize is %lld, usedTime is %lld, speed is(kB/s): %llu",
        m_strTaskID.c_str(),
        finishedSize,
        m_preDataSizeFinished,
        usedTime,
        m_nowSpeed);

    m_preTime = nowTime;
    m_preDataSizeFinished = finishedSize;
    m_progress = progress;
    return m_nowSpeed;
}

mp_int32 VMwareNativeDataPathImpl::BackupProgressQuery(Json::Value &bodyMsg)
{
    bodyMsg.clear();

    mp_int32 progress = 0;
    mp_int32 jobStatus = 0;
    mp_string jobDesc;
    CalcJobProgress(progress, jobStatus, jobDesc);
    bodyMsg[PARAM_KEY_TASKSTATUS] = jobStatus;
    bodyMsg[PARAM_KEY_TASKPROGRESS] = progress;
    bodyMsg[PARAM_KEY_TASKDESC] = jobDesc;

    DBGLOG("Storage san access disk path: %s", m_volumeInfo.vmfsIOFlag.c_str());
    mp_string transModeTmp = m_transportModeSelected;
    if (!m_volumeInfo.nasIOFlag.empty()) {
        transModeTmp = VMWARE_TRANSMODE_STORAGE_NAS;
    } else if (!m_volumeInfo.vmfsIOFlag.empty()) {
        /* m_transportModeSelected may not set to 'storage san access' yet,
         * so if mount path not empty, return VMWARE_TRANSMODE_STORAGE_LUN directly,
         * otherwise, it's probably report incorrect trans mode(was set when open for querying CBT)
         * from 'storage san access' mode to others. */
        transModeTmp = VMWARE_TRANSMODE_STORAGE_LUN;
    }
    bodyMsg[PARAM_KEY_DATA_TRANS_MODE] = transModeTmp;
    bodyMsg[PARAM_KEY_DATA_TRANS_SPEED] = static_cast<Json::UInt64>(GetDiskSpeed(progress));
    INFOLOG("Query backup progress for vmware vm disk '%s' successfully, \
            transport mode: %s, task '%s', parent task '%s'.",
            m_volumeInfo.strDiskPath.c_str(),
            transModeTmp.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());

    return (jobStatus == TASK_RUN_FAILURE) ? MP_FAILED : MP_SUCCESS;
}

void VMwareNativeDataPathImpl::CheckHotAddVmdkDevice(Json::Value &bodyMsg)
{
    if (!CheckDiskIsMappedByWwn()) {
        return;
    }
    if (CloseDisk() != MP_SUCCESS) {
        ERRLOG("Unable to close disk '%s' after check device mapped, task '%s', parent task '%s'.",
            m_volumeInfo.strDiskID.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    }
    sleep(1);
    if (CheckDiskIsMappedByWwn()) {
        bodyMsg["ResAttachedDiskPath"] = m_volumeInfo.strDiskPath;
    }
}

// last step of a volume level protection: close disk, cleanup vddk
mp_int32 VMwareNativeDataPathImpl::DataBlockBackupFinish(Json::Value &bodyMsg)
{
    INFOLOG(
        "Backup of disk '%s' has been completed, will try to close the disk, task '%s', parent task '%s'.",
        m_volumeInfo.strDiskID.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    bodyMsg.clear();
    // reset var shared
    bodyMsg[PARAM_KEY_ZEROBLOCK] =
            m_totalCompletedBlocks.load() - m_zeroBlocks.load() >= 0 ? m_totalCompletedBlocks.load() -
                                                                       m_zeroBlocks.load() : 0;
    if (m_reducedBlocks.load() > 0) {
        bodyMsg[PARAM_KEY_REDUCTION_BLOCKS] = m_reducedBlocks.load();
        INFOLOG("ReducedBlocks details , reducedBlocks: '%llu'",  m_reducedBlocks.load());
    }
    INFOLOG("Blocks process details , TotalCompletedBlocks size: '%llu', ZeroBlocks: '%llu', CompletedBlocks: '%llu'",
            m_totalCompletedBlocks.load(), m_zeroBlocks.load(), m_completedBlocks.load());
    m_completedBlocks.store(0);
    m_zeroBlocks.store(0);
    m_totalCompletedBlocks.store(0);
    m_reducedBlocks.store(0);
    // 执行备份后置处理动作
    if (m_writer != nullptr && m_writer->PostBackup() != MP_SUCCESS) {
        ERRLOG("Post backup for disk '%s' failed.", m_volumeInfo.strDiskID.c_str());
        if (m_volumeInfo.uLimitSpeed != 0) {
            JobQosManager::UnRegisterQos(m_strTaskID);
        }
        return MP_FAILED;
    }

    if (m_isDiskOpened) {
        if (MP_SUCCESS != CloseDisk()) {
            ERRLOG("Unable to close disk '%s', task '%s', parent task '%s'.",
                m_volumeInfo.strDiskID.c_str(),
                m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
            return MP_FAILED;
        }
        m_isDiskOpened = false;
    }

    CheckHotAddVmdkDevice(bodyMsg);

    if (m_volumeInfo.uLimitSpeed != 0) {
        JobQosManager::UnRegisterQos(m_strTaskID);
    }

    INFOLOG("Backup of vmware vm '%s' has been completed successfully, task '%s', parent task '%s'.",
        m_volumeInfo.strDiskID.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::VMwareVmBackupFinish(Json::Value &bodyMsg)
{
    INFOLOG("Backup of vmware vm has been completed, will try to release all resources, task '%s', parent task '%s'.",
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    bodyMsg.clear();
    std::string errDesc;
    VMWARE_DISK_RET_CODE rc = VMwareDiskLib::GetInstance()->EndAccess(m_vddkConnParams, errDesc);
    if (rc != VIX_OK) {
        WARNLOG("End access failure, errcode '%d', error '%s', task '%s', parent task '%s'!",
            rc,
            errDesc.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    }

    rc = VMwareDiskLib::GetInstance()->Cleanup(m_vddkConnParams, errDesc);
    if (rc != VIX_OK) {
        WARNLOG("Cleanup failure, errcode '%d', error '%s', task '%s', parent task '%s'!",
            rc,
            errDesc.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    }
    if (m_volumeInfo.uLimitSpeed != 0) {
        JobQosManager::UnRegisterQos(m_strTaskID);
    }
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 VMwareNativeDataPathImpl::VMwareVmRestoreCancel(Json::Value &bodyMsg)
{
    INFOLOG("Restore of disk '%s' has been canceled, will try to close it, task '%s', parent task '%s'.",
        m_volumeInfo.strDiskID.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    bodyMsg.clear();
    // reset var shared
    m_completedBlocks.store(0);

    if (m_isDiskOpened) {
        if (MP_SUCCESS != CloseDisk()) {
            ERRLOG("Unable to close disk '%s', task '%s', parent task '%s'.",
                m_volumeInfo.strDiskID.c_str(),
                m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
            return MP_FAILED;
        }
        m_isDiskOpened = false;
    }
    INFOLOG("Recovery of vmware vm '%s' has been canceled successfully, task '%s', parent task '%s'.",
        m_volumeInfo.strDiskPath.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::RestoreProgressQuery(Json::Value &bodyMsg)
{
    bodyMsg.clear();

    mp_int32 progress = 0;
    mp_int32 jobStatus = 0;
    mp_string jobDesc;
    CalcJobProgress(progress, jobStatus, jobDesc);
    bodyMsg[PARAM_KEY_TASKSTATUS] = jobStatus;
    bodyMsg[PARAM_KEY_TASKPROGRESS] = progress;
    bodyMsg[PARAM_KEY_TASKDESC] = jobDesc;
    bodyMsg[PARAM_KEY_DATA_TRANS_MODE] = m_transportModeSelected;
    bodyMsg[PARAM_KEY_DATA_TRANS_SPEED] = static_cast<Json::UInt64>(GetDiskSpeed(progress));
    if (jobStatus == TASK_RUNNING || jobStatus == TASK_RUN_SUCCESS) {
        INFOLOG("Query restore progress for vmware vm disk '%s' successfully, task '%s', parent task '%s'.",
            m_volumeInfo.strDiskPath.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    }
    INFOLOG("bodyMsg: %s", bodyMsg.toStyledString().c_str());
    return (jobStatus == TASK_RUN_FAILURE) ? MP_FAILED : MP_SUCCESS;
}

// last step of a volume level protection: close disk, cleanup vddk
mp_int32 VMwareNativeDataPathImpl::DataBlockRestoreFinish(Json::Value &bodyMsg)
{
    INFOLOG("Restore of disk '%s' has been completed, will try to close it, task '%s', parent task '%s'.",
        m_volumeInfo.strDiskID.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    bodyMsg.clear();
    // reset var shared
    m_completedBlocks.store(0);

    // 执行恢复后置处理动作
    if (m_reader != nullptr && m_reader->PostRecovery() != MP_SUCCESS) {
        ERRLOG("Post recovery for disk '%s' failed.", m_volumeInfo.strDiskID.c_str());
        return MP_FAILED;
    }

    if (m_isDiskOpened) {
        if (MP_SUCCESS != CloseDisk()) {
            ERRLOG("Unable to close disk '%s', task '%s', parent task '%s'.",
                m_volumeInfo.strDiskID.c_str(),
                m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
            return MP_FAILED;
        }
        m_isDiskOpened = false;
    }

    INFOLOG("The vCenter vm disk recovery has been completed successfully!, task '%s', parent task '%s'.",
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::VMwareVmRestoreFinish(Json::Value &bodyMsg)
{
    bodyMsg.clear();
    std::string errDesc;
    VMWARE_DISK_RET_CODE rc = VMwareDiskLib::GetInstance()->EndAccess(m_vddkConnParams, errDesc);
    if (rc != VIX_OK) {
        WARNLOG("End access failure, errcode '%d', error '%s', task '%s', parent task '%s'!",
            rc,
            errDesc.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    }

    rc = VMwareDiskLib::GetInstance()->Cleanup(m_vddkConnParams, errDesc);
    if (rc != VIX_OK) {
        WARNLOG("Cleanup failure, errcode '%d', error '%s', task '%s', parent task '%s'!",
            rc,
            errDesc.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    }
    INFOLOG(
        "The vCenter vm recovery vm has been completed successfully, task '%s', parent task '%s'.",
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 VMwareNativeDataPathImpl::ParseStorageLunMounted(Json::Value &msg)
{
    mp_int32 iRet = MP_FAILED;
    Json::Value jsonMsgBody = msg[MANAGECMD_KEY_BODY];
    static const mp_string strkeyStorageLunMounted = "StorageLunMounted";

    std::vector<Json::Value> jsonLunList;
    iRet = CJsonUtils::GetJsonArrayJson(jsonMsgBody, strkeyStorageLunMounted, jsonLunList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Unable to parse json array object of key '%s', ret: '%d', task '%s', parent task '%s'.",
            strkeyStorageLunMounted.c_str(),
            iRet,
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return iRet;
    }

    std::vector<storage_lun> lunList;
    iRet = JsonArray2StorageLunArr(jsonLunList, lunList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Unable to parse storage lun(s) mounted, ret: '%d', task '%s', parent task '%s'.",
            iRet,
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return iRet;
    }

    m_lunList.assign(lunList.begin(), lunList.end());

    return iRet;
}

mp_int32 VMwareNativeDataPathImpl::MatchSpecificTargetLun(mp_string &wwn)
{
    std::ostringstream oss;
    std::vector<storage_lun>::iterator lunIter;
    m_volumeInfo.strTargetLunPath = "";
    for (lunIter = m_lunList.begin(); lunIter != m_lunList.end(); ++lunIter) {
        // match string except substring "wwn"
        if (wwn == lunIter->wwn) {
            INFOLOG(
                "The target disk '%s' has been found, task '%s', parent task '%s'.",
                wwn.c_str(),
                m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
            oss << lunIter->deviceName << STR_COMMA;
            break;
        }
    }
    if (lunIter == m_lunList.end()) {
        COMMLOG(OS_LOG_ERROR,
            "The disk(%s) does not exist, task '%s', parent task '%s'.",
            wwn.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return ERROR_DISK_GET_DISK_INFO_FAILED;
    }

    m_volumeInfo.strTargetLunPath = oss.str();
    m_volumeInfo.strTargetLunPath = m_volumeInfo.strTargetLunPath.substr(0, m_volumeInfo.strTargetLunPath.length() - 1);
    INFOLOG(
        "The target disk mounted is: '%s', task '%s', parent task '%s'.",
        m_volumeInfo.strTargetLunPath.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::ParseDirtyRangesParams(Json::Value &msg)
{
    mp_int32 iRet = MP_FAILED;
    Json::Value jsonMsgBody = msg[MANAGECMD_KEY_BODY];
    static const mp_string strkeyDirtyRange = "DirtyRange";
    // parse json array object from json message body
    std::vector<Json::Value> jsonDirtyRanges;
    iRet = CJsonUtils::GetJsonArrayJson(jsonMsgBody, strkeyDirtyRange, jsonDirtyRanges);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to parse json array object of key '%s', ret: '%d', task '%s', parent task '%s'.",
            strkeyDirtyRange.c_str(),
            iRet,
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return iRet;
    }

    std::vector<dirty_range> dirtyRanges;
    iRet = JsonArray2DirtyRangeArr(jsonDirtyRanges, dirtyRanges);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to parse dirty range attrs, ret: '%d', task '%s', parent task '%s'.",
            iRet,
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return iRet;
    }

    m_volumeInfo.vecDirtyRange.assign(dirtyRanges.begin(), dirtyRanges.end());

    return iRet;
}

mp_int32 VMwareNativeDataPathImpl::ParseProtectEnvParams(Json::Value &bodyMsg, vmware_pe_info &peInfo)
{
    Json::Value jsonProductEnv = bodyMsg[PARAM_KEY_PROTECT_ENV];
    GET_JSON_UINT64(jsonProductEnv, PARAM_KEY_PRODUCTMANAGER_PROTOCOL, peInfo.productManager.ulProtocol);
    GET_JSON_STRING(jsonProductEnv, PARAM_KEY_PRODUCTMANAGER_IP, peInfo.productManager.strIP);
    GET_JSON_UINT64(jsonProductEnv, PARAM_KEY_PRODUCTMANAGER_PORT, peInfo.productManager.ulPort);
    GET_JSON_STRING(jsonProductEnv, PARAM_KEY_PRODUCTMANAGER_USERNAME, peInfo.productManager.strUserName);
    GET_JSON_STRING(jsonProductEnv, PARAM_KEY_PRODUCTMANAGER_PASSWORD, peInfo.productManager.strPassword);
    GET_JSON_STRING(jsonProductEnv, PARAM_KEY_PRODUCTMANAGER_VERSION, peInfo.productManager.strVersion);
    GET_JSON_STRING(jsonProductEnv, PARAM_KEY_VM_REF, peInfo.strVmRef);
    if (CheckVmwarePmInfo(peInfo.productManager) == MP_FAILED) {
        return MP_FAILED;
    }
    mp_int32 iRPort = VerifyParamsSize(peInfo.productManager.ulPort, PORT_RANGE, PARAM_KEY_PRODUCTMANAGER_PORT);
    mp_int32 iRPro = VerifyParamsSize(
        peInfo.productManager.ulProtocol, PROTOCOL_RANGE, PARAM_KEY_PRODUCTMANAGER_PROTOCOL);
    if (iRPort == MP_FAILED || iRPro == MP_FAILED) {
        ERRLOG("Prepare backup Port=%I64d is out of Range", peInfo.productManager.ulPort);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::JsonArray2DirtyRangeArr(
    const std::vector<Json::Value> &jsonArr, std::vector<dirty_range> &dirtyRanges)
{
    Json::Value dirtyRange;
    mp_size arrSize = jsonArr.size();
    for (mp_size i = 0; i < arrSize; i++) {
        dirtyRange = jsonArr[i];

        if (!dirtyRange.isObject()) {
            ERRLOG("JsonArray2DirtyRangeArr: The element of array is not a Json object.");
            dirtyRanges.clear();
            return ERROR_COMMON_INVALID_PARAM;
        } else {
            dirty_range dr;
            GET_JSON_UINT64(dirtyRange, PARAM_KEY_VOLUME_DIRTYRANGE_START, dr.start);
            GET_JSON_UINT64(dirtyRange, PARAM_KEY_VOLUME_DIRTYRANGE_LENGTH, dr.length);
            if (VerifyParamsSize(dr.start, MAX_DISK_SIZE, PARAM_KEY_VOLUME_DIRTYRANGE_START) == MP_FAILED ||
                VerifyParamsSize(dr.length, DT_DATA_BLOCK, PARAM_KEY_VOLUME_DIRTYRANGE_LENGTH) == MP_FAILED) {
                return ERROR_COMMON_INVALID_PARAM;
            }

            dirtyRanges.push_back(dr);
        }
    }

    return MP_SUCCESS;
}
mp_int32 VMwareNativeDataPathImpl::JsonArray2StorageLunArr(
    const std::vector<Json::Value> &jsonArr, std::vector<storage_lun> &lunList)
{
    const static mp_string strKeyLunID = "lunId";
    const static mp_string strKeyWWN = "wwn";
    const static mp_string strKeyDeviceName = "deviceName";
    const static mp_string strKeyDiskNumber = "diskNumber";

    Json::Value lunInfo;
    mp_size arrSize = jsonArr.size();
    for (mp_size i = 0; i < arrSize; i++) {
        lunInfo = jsonArr[i];

        if (!lunInfo.isObject()) {
            ERRLOG("JsonArray2StorageLunArr: The element of array is not a Json object.");
            lunList.clear();
            return ERROR_COMMON_INVALID_PARAM;
        } else {
            storage_lun lun;
            GET_JSON_STRING(lunInfo, strKeyLunID, lun.lunId);
            GET_JSON_STRING(lunInfo, strKeyWWN, lun.wwn);
            GET_JSON_STRING(lunInfo, strKeyDeviceName, lun.deviceName);
            GET_JSON_STRING(lunInfo, strKeyDiskNumber, lun.diskNumber);

            lunList.push_back(lun);
        }
    }

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::PkgJsonBody(Json::Value &jsonMsgBody)
{
    // package the task id and parent task id to volume params
    GET_JSON_STRING(jsonMsgBody, PARAM_KEY_TASKID, m_strTaskID);
    GET_JSON_STRING(jsonMsgBody, PARAM_KEY_PARENT_TASKID, m_strParentTaskID);
    GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_MEDIUMID, m_volumeInfo.strMediumID);
    GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISKID, m_volumeInfo.strDiskID);
    GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISKPATH, m_volumeInfo.strDiskPath);
    /* storage snapshot backup */
    if (m_protectType == VMWARE_VM_BACKUP) {
        GET_JSON_ARRAY_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISK_DATASTORE_WWN, m_volumeInfo.datastoreWwn);
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VMFSIO_FLAG, m_volumeInfo.vmfsIOFlag);
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISK_MOUNT_PATH, m_volumeInfo.nasIOFlag);
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISK_RELATIVE_PATH, m_volumeInfo.strDiskRelativePath);

        DBGLOG("Get paramters, strDiskPath: %s, strDiskRelativePath: %s, vmfsIOFlag: %s, nasIOFlag: %s, DatastoreWwn:, "
               "strDiskMountPath: %s",
            m_volumeInfo.strDiskPath.c_str(), m_volumeInfo.strDiskRelativePath.c_str(), m_volumeInfo.vmfsIOFlag.c_str(),
            m_volumeInfo.nasIOFlag.c_str());
        for (const auto &wwn : m_volumeInfo.datastoreWwn) {
            DBGLOG("wwn: %s", wwn.c_str());
        }
    }
    /* storage snapshot backup - END */
    GET_JSON_UINT64(jsonMsgBody, PARAM_KEY_VOLUME_DISKSIZE, m_volumeInfo.ulDiskSize);
    GET_JSON_INT32(jsonMsgBody, PARAM_KEY_HOSTAGENT_SYSTEM_VIRT, m_systemVirt);
    if (jsonMsgBody.isMember(PARAM_KEY_VOLUME_DISKTYPE)) {
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISKTYPE, m_volumeInfo.strDiskType);
    }
    if (jsonMsgBody.isMember(PARAM_KEY_VOLUME_WWN)) {
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_WWN, m_volumeInfo.strWwn);
    }
    DBGLOG("disktype %s, wwn %s, task '%s', parent task '%s' size %ld.", m_volumeInfo.strDiskType.c_str(),
        m_volumeInfo.strWwn.c_str(), m_strTaskID.c_str(), m_strParentTaskID.c_str(), m_volumeInfo.ulDiskSize);

    if (VerifyParamsSize(m_volumeInfo.ulDiskSize, MAX_DISK_SIZE, PARAM_KEY_VOLUME_DISKSIZE) == MP_FAILED) {
        return MP_FAILED;
    }
    GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_EAGERLY_CRUB, m_volumeInfo.strEagerlyCrub);
    return MP_SUCCESS;
}

// parse volume parameters
mp_int32 VMwareNativeDataPathImpl::ParseVolumeParams(Json::Value &msg)
{
    Json::Value jsonMsgBody = msg[MANAGECMD_KEY_BODY];
    if (MP_SUCCESS != PkgJsonBody(jsonMsgBody)) {
        ERRLOG("error in PkgJsonBody.");
        return MP_FAILED;
    }

    m_volumeInfo.strTaskID = m_strTaskID;
    m_volumeInfo.strParentTaskID = m_strParentTaskID;

    if (CheckVmwareVolumeInfo(m_volumeInfo) == MP_FAILED) {
        return MP_FAILED;
    }
    if (m_protectType == VMWARE_VM_RECOVERY) {
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_BACKUPED_DISKID, m_volumeInfo.strBackupedDiskID);
        if (jsonMsgBody.isMember(PARAM_KEY_VOLUME_SUPPORT_SAN_TRANSPORTMODE) &&
            jsonMsgBody[PARAM_KEY_VOLUME_SUPPORT_SAN_TRANSPORTMODE].isBool()) {
            m_volumeInfo.bSupportSAN = jsonMsgBody[PARAM_KEY_VOLUME_SUPPORT_SAN_TRANSPORTMODE].asBool();
            INFOLOG("Support SAN transport mode: %d, task '%s', parent task '%s'.",
                m_volumeInfo.bSupportSAN, m_strTaskID.c_str(), m_strParentTaskID.c_str());
        } else {
            ERRLOG("The request message has no key: '%s', task '%s', parent task '%s'.",
                PARAM_KEY_VOLUME_SUPPORT_SAN_TRANSPORTMODE.c_str(), m_strTaskID.c_str(), m_strParentTaskID.c_str());
            return MP_FAILED;
        }
    }

    if (m_protectType == VMWARE_VM_BACKUP) {
        // parse vmdk desc file attrs
        mp_int32 iTmp = 0;
        GET_JSON_INT32(jsonMsgBody[PARAM_KEY_DESCFILE_ATTRS], PARAM_KEY_DESCFILE_ATTRS_CYLINDER, iTmp);
        m_volumeInfo.descFileInfo.strCylinders = std::to_string(iTmp);
        GET_JSON_INT32(jsonMsgBody[PARAM_KEY_DESCFILE_ATTRS], PARAM_KEY_DESCFILE_ATTRS_HEAD, iTmp);
        m_volumeInfo.descFileInfo.strHeads = std::to_string(iTmp);
        GET_JSON_INT32(jsonMsgBody[PARAM_KEY_DESCFILE_ATTRS], PARAM_KEY_DESCFILE_ATTRS_SECTOR, iTmp);
        m_volumeInfo.descFileInfo.strSectors = std::to_string(iTmp);
        // parse backup speed limition
        GET_JSON_UINT32(jsonMsgBody, PARAM_KEY_VOLUME_LIMITSPEED, m_volumeInfo.uLimitSpeed);

        // parse actual backup level
        GET_JSON_UINT32(jsonMsgBody, PARAM_KEY_VOLUME_BACKUPLEVEL, m_volumeInfo.uBackupLevel);

        // parse expected transport mode
        if (jsonMsgBody.isMember(PARAM_KEY_VOLUME_EXPECTED_TRANSPORTMODE) &&
            jsonMsgBody[PARAM_KEY_VOLUME_EXPECTED_TRANSPORTMODE].isInt()) {
            GET_JSON_UINT32(jsonMsgBody, PARAM_KEY_VOLUME_EXPECTED_TRANSPORTMODE, m_volumeInfo.transportModeExpected);
            INFOLOG("Specified mode : '%d'.", m_volumeInfo.transportModeExpected);
        }
    }
    return MP_SUCCESS;
}

void VMwareNativeDataPathImpl::GetTransportMode(const bool &enableAdvanced, mp_string &transportMode)
{
    std::string supportTransModeList = VMwareDiskLib::GetInstance()->ListTransportModes();
    INFOLOG("Transport modes supported by current version of VDDK: '%s'.", supportTransModeList.c_str());
    size_t index = supportTransModeList.find("san:hotadd");
    bool isSupportAdvancedTrans = (std::string::npos != index);
    if (enableAdvanced && isSupportAdvancedTrans) {
        bool canNotUseSAN = ((m_systemVirt == VIRTUAL_MACHINE) || (m_protectType == VMWARE_VM_RECOVERY &&
            !m_volumeInfo.bSupportSAN && m_volumeInfo.strEagerlyCrub != "true"));
        if (canNotUseSAN) {
            INFOLOG("This task can not use SAN. EagerlyCrub: '%s', systemVirt: %d.",
                m_volumeInfo.strEagerlyCrub.c_str(), m_systemVirt);
            transportMode = "hotadd:" + transportMode;
        } else {
            transportMode = "san:hotadd:" + transportMode;
        }
    }
}

mp_int32 VMwareNativeDataPathImpl::TryToOpenDisk(
    const mp_uint32 &openMode, const mp_uint64 &chunkSize, mp_string &tryTransMode, mp_string &errDesc)
{
    INFOLOG("Begin to try to open disk with modes: '%s'.", tryTransMode.c_str());
    if (m_diskType == "rdm") {
        mp_uint32 rdmOpenMode = (openMode == VMWAREDISK_FLAG_OPEN_READ_ONLY)?O_RDONLY:O_RDWR;
        m_transportModeSelected = VMWARE_TRANSMODE_RDM;
        return m_spVMwareDiskApi->OpenDiskByLocalDevice(m_localDevicePath, rdmOpenMode,
            m_diskType, m_volumeInfo.ulDiskSize);
    }
    int iRet = m_spVMwareDiskApi->OpenDisk(m_volumeInfo.strDiskPath,
        m_vmProtectionParams.vmInfo.strSnapshotRef,
        openMode,
        chunkSize,
        tryTransMode,
        m_transportModeSelected,
        errDesc);
    if (iRet == VIX_E_FILE_ACCESS_ERROR) {
        tryTransMode = "hotadd:nbdssl";
        COMMLOG(OS_LOG_WARN, "Access rights error, try to use mode: '%s'.", tryTransMode.c_str());
        iRet = m_spVMwareDiskApi->OpenDisk(m_volumeInfo.strDiskPath,
            m_vmProtectionParams.vmInfo.strSnapshotRef,
            openMode,
            chunkSize,
            tryTransMode,
            m_transportModeSelected,
            errDesc);
    }
    return iRet;
}

mp_int32 VMwareNativeDataPathImpl::TryConnecetWithSpecifiedMode()
{
    m_vddkConnParams.transportMode = TRANSPORT_MODE_MAP.at(m_volumeInfo.transportModeExpected);
    m_spVMwareDiskApi = VMwareDiskLib::GetInstance()->GetVMwareDiskApiInstance(m_vddkConnParams);
    if (m_spVMwareDiskApi == nullptr) {
        WARNLOG("Connect with expected mode(%s) failed.", m_vddkConnParams.transportMode.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 VMwareNativeDataPathImpl::OpenDisk(mp_uint32 openMode)
{
    // obtain availabel transport mode supported by VDDK lib
    mp_string errDesc;
    mp_uint64 chunkSize = 0;           // VDDKNBDChunkSizeKB
    mp_bool isEnableSSL = false;       // VMwareNBDSSLEnable
    mp_bool isEnableAdvanced = false;  // EnableLanFreeForVMWareBackup
    if (!GetConfigDataFromDataProcessNode(chunkSize, isEnableSSL, isEnableAdvanced)) {
        ERRLOG("get dataProcess form agentConfig failed!");
        return DMVMWareErrorCodeInternalError;
    }
    std::string transportModes = isEnableSSL ? "nbdssl" : "nbd";
    GetTransportMode(isEnableAdvanced, transportModes);
    INFOLOG("Selected transport modes: '%s'.", transportModes.c_str());
    // obtain the VMwareDiskApi instance
    m_vddkConnParams.vmSnapshotRef = m_vmProtectionParams.vmInfo.strSnapshotRef;
    m_vddkConnParams.openMode = (openMode == VMWAREDISK_FLAG_OPEN_READ_ONLY) ? true : false;
    m_vddkConnParams.protectType = m_protectType;
    if (m_protectType == VMWARE_VM_RECOVERY) {
        m_vddkConnParams.bSupportSAN = m_volumeInfo.bSupportSAN;
    }
    m_vddkConnParams.hostagentSystemVirt = m_systemVirt;
    m_vddkConnParams.diskType = m_volumeInfo.strDiskType;
    if (m_volumeInfo.transportModeExpected != VMWAREDEF::TRANSPORT_MODE_NONE &&
        TryConnecetWithSpecifiedMode() == MP_SUCCESS) {
        transportModes = TRANSPORT_MODE_MAP.at(m_volumeInfo.transportModeExpected);
        INFOLOG("Use specified transport mode, try with idx: '%s'.", transportModes.c_str());
    } else {
        m_vddkConnParams.transportMode = transportModes;
        m_spVMwareDiskApi = VMwareDiskLib::GetInstance()->GetVMwareDiskApiInstance(m_vddkConnParams);
    }
    if (m_spVMwareDiskApi == nullptr) {
        ERRLOG("Create vmware disk api failed!");
        return DMVMWareErrorCodeInternalError;
    }
    // open disk and obtain the disk operation halder
    int iRet = TryToOpenDisk(openMode, chunkSize, transportModes, errDesc);
    if (VIX_OK != iRet) {
        COMMLOG(OS_LOG_ERROR,
            "Open disk failed '%s', vm '%s', vm moRef '%s', vm snapshot moRef '%s', seleted mode '%s', \
            task '%s', parent task '%s', ret: '%d'.",
            errDesc.c_str(),
            m_vmProtectionParams.vmInfo.strVmName.c_str(),
            m_vmProtectionParams.vmInfo.strVmRef.c_str(),
            m_vmProtectionParams.vmInfo.strSnapshotRef.c_str(),
            m_transportModeSelected.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str(),
            iRet);
        return DMVMWareErrorCodeInternalError;
    }
    OpenDiskPrintLog();
    return MP_SUCCESS;
}

void VMwareNativeDataPathImpl::OpenDiskPrintLog()
{
    INFOLOG(
        "Open disk '%s' of vm '%s', moRef '%s', snapshot moRef '%s' success, seleted mode '%s', \
        task '%s', parent task '%s'.",
        m_volumeInfo.strDiskPath.c_str(),
        m_vmProtectionParams.vmInfo.strVmName.c_str(),
        m_vmProtectionParams.vmInfo.strVmRef.c_str(),
        m_vmProtectionParams.vmInfo.strSnapshotRef.c_str(),
        m_transportModeSelected.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
}

mp_int32 VMwareNativeDataPathImpl::TryToOpenDiskForAfsBitmap(const std::shared_ptr<VMwareDiskApi>& diskApi,
    const mp_uint32 &openMode, const mp_uint64 &chunkSize, mp_string &tryTransMode, mp_string &errDesc)
{
    INFOLOG("Begin to try to open disk for afs bitmap with modes: '%s'.", tryTransMode.c_str());
    if (m_diskType == "rdm") {
        mp_uint32 rdmOpenMode = (openMode == VMWAREDISK_FLAG_OPEN_READ_ONLY)?O_RDONLY:O_RDWR;
        return diskApi->OpenDiskByLocalDevice(m_localDevicePath, rdmOpenMode,
            m_diskType, m_volumeInfo.ulDiskSize);
    }
    int iRet = diskApi->OpenDisk(m_volumeInfo.strDiskPath,
        m_vmProtectionParams.vmInfo.strSnapshotRef,
        openMode,
        chunkSize,
        tryTransMode,
        m_transportModeSelected,
        errDesc);
    if (iRet == VIX_E_FILE_ACCESS_ERROR) {
        tryTransMode = "hotadd:nbdssl";
        COMMLOG(OS_LOG_WARN, "Access rights error, try to use mode: '%s'.", tryTransMode.c_str());
        iRet = diskApi->OpenDisk(m_volumeInfo.strDiskPath,
            m_vmProtectionParams.vmInfo.strSnapshotRef,
            openMode,
            chunkSize,
            tryTransMode,
            m_transportModeSelected,
            errDesc);
    }
    INFOLOG("Exit TryToOpenDiskForAfsBitmap");
    return iRet;
}

void VMwareNativeDataPathImpl::BuildConnectParams(VddkConnectParams &connectParams)
{
    std::string tempIp = CIP::FormatFullUrl(m_vmProtectionParams.pmInfo.strIP);
    connectParams.vmSpec = "moref=" + m_vmProtectionParams.vmInfo.strVmRef;
    connectParams.serverName = std::move(tempIp);
    connectParams.port = m_vmProtectionParams.pmInfo.ulPort;
    connectParams.userName = m_vmProtectionParams.pmInfo.strUserName;
    connectParams.password = m_vmProtectionParams.pmInfo.strPassword;
    connectParams.thumbPrint = m_vmProtectionParams.pmInfo.strThumbPrint;
    connectParams.vmMoRef = m_vmProtectionParams.vmInfo.strVmRef;
    connectParams.vmSnapshotRef = m_vmProtectionParams.vmInfo.strSnapshotRef;
}

mp_int32 VMwareNativeDataPathImpl::OpenDiskForAfsBitmapInner(std::shared_ptr<VMwareDiskApi>& diskApi)
{
    INFOLOG("Enter OpenDiskForAfsBitmapInner");
    mp_int32 openMode = VMWAREDISK_FLAG_OPEN_READ_ONLY;
    // obtain availabel transport mode supported by VDDK lib
    mp_string errDesc;
    mp_uint64 chunkSize = 0;           // VDDKNBDChunkSizeKB
    mp_bool isEnableSSL = false;       // VMwareNBDSSLEnable
    mp_bool isEnableAdvanced = false;  // EnableLanFreeForVMWareBackup
    if (!GetConfigDataFromDataProcessNode(chunkSize, isEnableSSL, isEnableAdvanced)) {
        ERRLOG("get dataProcess form agentConfig failed!");
        return DMVMWareErrorCodeInternalError;
    }
    std::string transportModes = isEnableSSL ? "nbdssl" : "nbd";
    GetTransportMode(isEnableAdvanced, transportModes);
    INFOLOG("Selected transport modes: '%s'.", transportModes.c_str());
    // obtain the VMwareDiskApi instance
    VddkConnectParams vddkConnParams;
    BuildConnectParams(vddkConnParams);

    vddkConnParams.openMode = (openMode == VMWAREDISK_FLAG_OPEN_READ_ONLY) ? true : false;
    vddkConnParams.protectType = m_protectType;
    vddkConnParams.hostagentSystemVirt = m_systemVirt;

    vddkConnParams.transportMode = transportModes;
    vddkConnParams.diskType = m_volumeInfo.strDiskType;
    diskApi = VMwareDiskLib::GetInstance()->GetVMwareDiskApiInstance(vddkConnParams);
    if (diskApi == nullptr) {
        ERRLOG("Create vmware disk api failed!");
        return DMVMWareErrorCodeInternalError;
    }
    // open disk and obtain the disk operation halder
    int iRet = TryToOpenDiskForAfsBitmap(diskApi, openMode, chunkSize, transportModes, errDesc);
    if (VIX_OK != iRet) {
        COMMLOG(OS_LOG_ERROR,
            "Open disk for afs bitmap failed '%s', vm '%s', vm moRef '%s', vm snapshot moRef '%s', seleted mode '%s', \
            task '%s', parent task '%s', ret: '%d'.",
            errDesc.c_str(),
            m_vmProtectionParams.vmInfo.strVmName.c_str(),
            m_vmProtectionParams.vmInfo.strVmRef.c_str(),
            m_vmProtectionParams.vmInfo.strSnapshotRef.c_str(),
            m_transportModeSelected.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str(),
            iRet);
        return DMVMWareErrorCodeInternalError;
    }
    OpenDiskPrintLog();
    return MP_SUCCESS;
}

bool VMwareNativeDataPathImpl::GetConfigDataFromDataProcessNode(
    mp_uint64 &chunkSize, mp_bool &isEnableSSL, mp_bool &isEnableAdvanced)
{
    mp_int32 result = ERROR_COMMON_READ_CONFIG_FAILED;
    result = CConfigXmlParser::GetInstance().GetValueUint64(CFG_DATAPROCESS_SECTION, CFG_CHUNK_SIZE, chunkSize);
    if (result != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "parses config failed!, parent node '%s', child node '%s'.",
            mp_string(CFG_DATAPROCESS_SECTION).c_str(),
            mp_string(CFG_CHUNK_SIZE).c_str());
        return false;
    }
    result = CConfigXmlParser::GetInstance().GetValueBool(CFG_DATAPROCESS_SECTION, CFG_IS_ENABLE_SSL, isEnableSSL);
    if (result != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "parses config failed!, parent node '%s', child node '%s'.",
            mp_string(CFG_DATAPROCESS_SECTION).c_str(),
            mp_string(CFG_IS_ENABLE_SSL).c_str());
        return false;
    }
    result = CConfigXmlParser::GetInstance().GetValueBool(
        CFG_DATAPROCESS_SECTION, CFG_IS_ENABLE_ADVANCED, isEnableAdvanced);
    if (result != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "parses config failed!, parent node '%s', child node '%s'.",
            mp_string(CFG_DATAPROCESS_SECTION).c_str(),
            mp_string(CFG_IS_ENABLE_ADVANCED).c_str());
        return false;
    }
    return true;
}
// close disk
mp_int32 VMwareNativeDataPathImpl::CloseDisk()
{
    // set thread exit flag, exit all thread
    m_exitFlag = true;

    // before close disk, need to wait for write thread exit
    for (auto &t : m_joinableThrs) {
        INFOLOG("Terminate thread '%llu'", t.get_id());
        t.join();
    }
    m_joinableThrs.clear();

    if ((!m_volumeInfo.vmfsIOFlag.empty() || !m_volumeInfo.nasIOFlag.empty()) &&
        m_spVMwareDiskApi == nullptr) {
        INFOLOG("Vmware disk api not inited, no need to close in storage san access mode.");
    } else {
        if (m_spVMwareDiskApi == nullptr) {
            COMMLOG(OS_LOG_ERROR,
                "CloseDisk: The VMwareDiskApi is not inited!,task '%s', parent task '%s'.",
                m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
            return vSphereErrorCodeInternalError;
        }

        mp_string strErrDesc;
        if (m_spVMwareDiskApi->CloseDisk(strErrDesc) != VIX_OK) {
            ERRLOG("CloseDisk: Close disk failed, task '%s', parent task '%s'.",
                m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
            return DMVMWareErrorCodeInternalError;
        }
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::InitVmProtectionParams(Json::Value &msg)
{
    vmware_vm_info vmInfoInner;
    vmware_pm_info pmInfoInner;
    mp_uint64 snapType = VMWARE_VM_FULL_BACKUP;  // default full level backup

    // parse vminfo and productmanager info
    if (ParsePreparationRequsetParams(msg, snapType, vmInfoInner, pmInfoInner) != MP_SUCCESS) {
        ERRLOG("Parse params failed, task(%s), parent task(%s).", m_strTaskID.c_str(), m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    m_vmProtectionParams.vmInfo.strVmID = vmInfoInner.strVmID;
    m_vmProtectionParams.vmInfo.strVmName = vmInfoInner.strVmName;
    m_vmProtectionParams.vmInfo.strVmRef = vmInfoInner.strVmRef;
    m_vmProtectionParams.pmInfo.ulProtocol = pmInfoInner.ulProtocol;
    m_vmProtectionParams.pmInfo.ulPort = pmInfoInner.ulPort;
    m_vmProtectionParams.pmInfo.strIP = pmInfoInner.strIP;
    m_vmProtectionParams.pmInfo.strUserName = pmInfoInner.strUserName;
    m_vmProtectionParams.pmInfo.strPassword = pmInfoInner.strPassword;
    m_vmProtectionParams.pmInfo.strVersion = pmInfoInner.strVersion;
    m_vmProtectionParams.pmInfo.strThumbPrint = pmInfoInner.strThumbPrint;
    m_vmProtectionParams.ulSnapType = snapType;

    INFOLOG("Parse vm protection params successfully, task(%s), parent task(%s).",
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    ClearString(pmInfoInner.strPassword);
    return MP_SUCCESS;
}
mp_int32 VMwareNativeDataPathImpl::PrivateParsePreparationRequsetParams(
    const Json::Value &msg, vmware_pm_info &pmInfo)
{
    Json::Value jsonMsgBody = msg[MANAGECMD_KEY_BODY];
    GET_JSON_UINT64(
        jsonMsgBody[EXT_CMD_PROTECT_PRODUCTMANAGER_INFO], PARAM_KEY_PRODUCTMANAGER_PROTOCOL, pmInfo.ulProtocol);
    GET_JSON_STRING(jsonMsgBody[EXT_CMD_PROTECT_PRODUCTMANAGER_INFO], PARAM_KEY_PRODUCTMANAGER_IP, pmInfo.strIP);
    GET_JSON_UINT64(jsonMsgBody[EXT_CMD_PROTECT_PRODUCTMANAGER_INFO], PARAM_KEY_PRODUCTMANAGER_PORT, pmInfo.ulPort);
    GET_JSON_STRING(
        jsonMsgBody[EXT_CMD_PROTECT_PRODUCTMANAGER_INFO], PARAM_KEY_PRODUCTMANAGER_USERNAME, pmInfo.strUserName);
    GET_JSON_STRING(
        jsonMsgBody[EXT_CMD_PROTECT_PRODUCTMANAGER_INFO], PARAM_KEY_PRODUCTMANAGER_PASSWORD, pmInfo.strPassword);
    GET_JSON_STRING(
        jsonMsgBody[EXT_CMD_PROTECT_PRODUCTMANAGER_INFO], PARAM_KEY_PRODUCTMANAGER_VERSION, pmInfo.strVersion);
    GET_JSON_STRING_OPTION(
        jsonMsgBody[EXT_CMD_PROTECT_PRODUCTMANAGER_INFO], PARAM_KEY_PRODUCTMANAGER_THUMBPRINT, pmInfo.strThumbPrint);
    if (CheckVmwarePmInfo(pmInfo) == MP_FAILED) {
        return MP_FAILED;
    }
    if (VerifyParamsSize(pmInfo.ulPort, PORT_RANGE, PARAM_KEY_PRODUCTMANAGER_PORT) == MP_FAILED ||
        VerifyParamsSize(pmInfo.ulProtocol, PROTOCOL_RANGE, PARAM_KEY_PRODUCTMANAGER_PROTOCOL) == MP_FAILED) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
mp_int32 VMwareNativeDataPathImpl::ParsePreparationRequsetParams(
    const Json::Value &msg, mp_uint64 &snapType, vmware_vm_info &vmInfo, vmware_pm_info &pmInfo)
{
    // 1. parse task id
    Json::Value jsonMsgBody = msg[MANAGECMD_KEY_BODY];
    GET_JSON_STRING(jsonMsgBody, PARAM_KEY_TASKID, m_strTaskID);
    GET_JSON_STRING(jsonMsgBody, PARAM_KEY_PARENT_TASKID, m_strParentTaskID);
    // 2. parse backup level, this key only exist when doing backup
    if (jsonMsgBody.isMember(PARAM_KEY_SNAPTYPE)) {
        GET_JSON_UINT64(jsonMsgBody, PARAM_KEY_SNAPTYPE, snapType);
        if (VerifyParamsSize(snapType, SNAP_TYPE_RANGE, PARAM_KEY_SNAPTYPE) == MP_FAILED) {
            return MP_FAILED;
        }
    }
    // 3. parse vm info
    if (!jsonMsgBody.isMember(EXT_CMD_PROTECT_VM_INFO)) {
        ERRLOG("Input param does not have key: '%s', task '%s', parent task '%s'.",
            EXT_CMD_PROTECT_VM_INFO.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }
    GET_JSON_STRING(jsonMsgBody[EXT_CMD_PROTECT_VM_INFO], PARAM_KEY_VM_NAME, vmInfo.strVmName);
    GET_JSON_STRING(jsonMsgBody[EXT_CMD_PROTECT_VM_INFO], PARAM_KEY_VM_ID, vmInfo.strVmID);
    GET_JSON_STRING(jsonMsgBody[EXT_CMD_PROTECT_VM_INFO], PARAM_KEY_VM_REF, vmInfo.strVmRef);
    if (CheckVmwareVmInfo(vmInfo) == MP_FAILED) {
        return MP_FAILED;
    }
    // parse attr "snapshotRef" only in backup task
    if (jsonMsgBody[EXT_CMD_PROTECT_VM_INFO].isMember(PARAM_KEY_VM_SNAPSHOTREF)) {
        GET_JSON_STRING(jsonMsgBody[EXT_CMD_PROTECT_VM_INFO], PARAM_KEY_VM_SNAPSHOTREF, vmInfo.strSnapshotRef);
    }
    // 4. parse productmanager info
    if (!jsonMsgBody.isMember(EXT_CMD_PROTECT_PRODUCTMANAGER_INFO)) {
        ERRLOG("Input param does not have key: '%s', task '%s', parent task '%s'.",
            EXT_CMD_PROTECT_PRODUCTMANAGER_INFO.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    if (PrivateParsePreparationRequsetParams(msg, pmInfo) == MP_FAILED) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::InitVddkLibPath(Json::Value &msg, mp_string &vddkPath)
{
    GET_JSON_STRING(msg[MANAGECMD_KEY_BODY], VMWAREDEF::PARAM_VDDKLIB_PATH, vddkPath);
    if (vddkPath.empty()) {
        ERRLOG(
            "The vddk lib path is empty, task '%s', parent task '%s'.", m_strTaskID.c_str(), m_strParentTaskID.c_str());
        return MP_FAILED;
    }
    vddkPath = CMpString::StrReplace(vddkPath, "//", "/");
    CHECK_FAIL_EX(CheckPathTraversal(vddkPath));
    CHECK_FAIL_EX(CheckPathString(vddkPath, CPath::GetInstance().GetAgentVDDKPath()));
    INFOLOG("The VDDK lib's installation path is: '%s', task '%s', parent task '%s'.",
        vddkPath.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::ParseDiskType(Json::Value &msg, mp_string &diskType)
{
    INFOLOG("Enter ParseDiskType: task '%s', parent task '%s'.",
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    Json::Value body = msg[MANAGECMD_KEY_BODY];
    if (!body.isObject() || !body.isMember(VMWAREDEF::PARAM_DISKTYPE_STR)) {
        INFOLOG("The disk type is normal disk: task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }
    GET_JSON_STRING(msg[MANAGECMD_KEY_BODY], VMWAREDEF::PARAM_DISKTYPE_STR, diskType);
    INFOLOG("The disk type is: '%s', task '%s', parent task '%s'.",
        diskType.c_str(),
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::VMwareNativeDataBlockBackup(Json::Value &bodyMsg, mp_string &strError)
{
    INFOLOG("Enter VMwareNativeDataBlockBackup, task '%s', parent task '%s'.",
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());
    m_preTime = CMpTime::GetTimeUsec();
    // 设置任务类型
    m_protectType = VMWARE_VM_BACKUP;
    this->m_isClose = false;
    m_preDataSizeFinished = 0;
    m_progress = 0;

    // 预处理，解析参数、打开VMDK磁盘等
    if (PrepareProcess(bodyMsg, strError) != MP_SUCCESS) {
        return MP_FAILED;
    }

    // 设置备份任务的读写端，读为VMware，写为备份存储
    if (m_iDirtyRangeSize != 0 && m_spVMwareDiskApi == nullptr && m_volumeInfo.vmfsIOFlag.empty()) {
        strError = "VMware native datablock backup : product has not been initialized";
        ERRLOG("Product has not been initialized, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    if (m_volumeInfo.vmfsIOFlag == "vmfsio") {
        m_reader = CreateVmfsIOEngine();
        if (m_reader == nullptr) {
            ERRLOG("Create vmfsio failed.");
            return MP_FAILED;
        }
    } else if (!m_volumeInfo.nasIOFlag.empty()) {
        // VMWARE_VM_RECOVERY for here need to read from NFS.
        m_reader = std::make_shared<FileIOEngine>(m_volumeInfo, VMWARE_VM_NFS_BACKUP, m_snapType);
    } else {
        m_reader = std::make_shared<VMwareIOEngine>(m_spVMwareDiskApi);
    }

    m_writer = std::make_shared<FileIOEngine>(m_volumeInfo, m_protectType, m_snapType);
    m_completedBlocks.store(0);
    // 另起线程处理数据块
    m_joinableThrs.push_back(std::thread(&VMwareNativeDataPathImpl::ProcessDiskBlocks, this));

    return MP_SUCCESS;
}

std::shared_ptr<IOEngine> VMwareNativeDataPathImpl::CreateVmfsIOEngine()
{
    m_vmfsio = std::make_shared<VmfsIO>(m_volumeInfo.datastoreWwn);
    if (m_vmfsio->Init() != MP_SUCCESS) {
        ERRLOG("Initialize vmfsio engine failed, disk: %s, task '%s', parent task '%s'.",
            m_volumeInfo.strDiskRelativePath.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return nullptr;
    }

    VmfsVerT vmfsVer = m_vmfsio->GetVmfsVersion();
    if (vmfsVer == VmfsVerT::VMFS5) {
        auto reader = std::make_shared<Vmfs5IOEngine>(m_vmfsio->GetVmfsHandler(), m_volumeInfo, m_protectType);
        return reader;
    } else {
        auto reader = std::make_shared<Vmfs6IOEngine>(m_vmfsio->GetVmfs6Handler(), m_volumeInfo, m_protectType);
        return reader;
    }

    return nullptr;
}

mp_int32 VMwareNativeDataPathImpl::VMwareNativeBackupCloseDisk()
{
    m_protectType = VMWARE_VM_BACKUP;
    // close disk
    INFOLOG("Entering");
    if (m_isDiskOpened) {
        INFOLOG("Close disk.");
        CloseDisk();
    }
    return MP_SUCCESS;
}


bool VMwareNativeDataPathImpl::IsRDMDisk()
{
    INFOLOG("disk type is '%s', task '%s', parent task '%s'.",
            m_volumeInfo.strDiskType.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    if (m_volumeInfo.strDiskType == "rdm") {
        m_diskType = m_volumeInfo.strDiskType;
        return true;
    }
    return false;
}

mp_int32 VMwareNativeDataPathImpl::GetLocalDevicePathByWwn(const mp_string &wwn, mp_string &localDevicePath)
{
    mp_int32 iRet;
    mp_string cmdQueryDeviceByWwn = "ls /dev/disk/by-id | grep " + wwn;
    std::vector<mp_string> vecDeviceInfo;
    int retryCount = 0;
    while (retryCount < RETRY_COUNT) {
        iRet = CSystemExec::ExecSystemWithEcho(cmdQueryDeviceByWwn, vecDeviceInfo);
        DBGLOG("\"%s\" Exec system cmd. result size %d", cmdQueryDeviceByWwn.c_str(), vecDeviceInfo.size());
        if (iRet != MP_SUCCESS) {
            ERRLOG("Exec system cmd failed, cmd is %s, iRet is %d. task '%s', parent task '%s'.",
                cmdQueryDeviceByWwn.c_str(), iRet, m_strTaskID.c_str(), m_strParentTaskID.c_str());
            return iRet;
        }
        if (!vecDeviceInfo.empty()) {
            break;
        }
        ScanDiskByIscsiAndFc();
        DoSleep(SLEEP_TIME_INTERVAL);
        retryCount++;
    }

    // 查找指定wwn对应的磁盘设备
    for (std::vector<mp_string>::iterator it = vecDeviceInfo.begin(); it != vecDeviceInfo.end(); it++) {
        DBGLOG("\"%s\" wwn and device info. size %d.", it->c_str(), vecDeviceInfo.size());
        if (it->find("wwn") != std::string::npos) {
            mp_string cmdGetRealPath = "realpath /dev/disk/by-id/" + (*it);
            std::vector<mp_string> vecRealPath;
            iRet = CSystemExec::ExecSystemWithEcho(cmdGetRealPath, vecRealPath);
            DBGLOG("\"%s\" Exec system cmd. result size %d", cmdGetRealPath.c_str(), vecRealPath.size());
            if (iRet != MP_SUCCESS) {
                ERRLOG("Exec system cmd failed, cmd is %s, iRet is %d. task '%s', parent task '%s'.",
                    cmdGetRealPath.c_str(),
                    iRet,
                    m_strTaskID.c_str(),
                    m_strParentTaskID.c_str());
                return iRet;
            }
            localDevicePath = *vecRealPath.begin();
            INFOLOG("\"%s\" device info, wwn is %s.", localDevicePath.c_str(), wwn.c_str());
            return MP_SUCCESS;
        }
    }
    INFOLOG("GetLocalDevicePathByWwn failed, wwn is %s, task '%s', parent task '%s'.",
            wwn.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    return MP_FAILED;
}

void VMwareNativeDataPathImpl::ScanDiskByIscsiAndFc()
{
    mp_string iscsiScanDiskCmd("rescan-scsi-bus.sh -r");
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(iscsiScanDiskCmd);
    if (iRet == MP_SUCCESS) {
        INFOLOG("Exec system cmd success, cmd is %s, iRet is %d. task '%s', parent task '%s'.",
            iscsiScanDiskCmd.c_str(), iRet, m_strTaskID.c_str(), m_strParentTaskID.c_str());
        return;
    }
    ERRLOG("Exec system cmd failed, cmd is %s, iRet is %d. task '%s', parent task '%s'.",
        iscsiScanDiskCmd.c_str(), iRet, m_strTaskID.c_str(), m_strParentTaskID.c_str());
    mp_string getFcHostCmd("ls /sys/class/fc_host/");
    std::vector<mp_string> vecHostInfo;
    iRet = CSystemExec::ExecSystemWithEcho(getFcHostCmd, vecHostInfo);
    DBGLOG("\"%s\" Exec system cmd. result size %d", getFcHostCmd.c_str(), vecHostInfo.size());
    if (iRet != MP_SUCCESS) {
        ERRLOG("Exec system cmd failed, cmd is %s, iRet is %d. task '%s', parent task '%s'.",
            getFcHostCmd.c_str(), iRet, m_strTaskID.c_str(), m_strParentTaskID.c_str());
        return;
    }
    for (std::vector<mp_string>::iterator it = vecHostInfo.begin(); it != vecHostInfo.end(); it++) {
        DBGLOG("\"%s\" host info. size %d.", it->c_str(), vecHostInfo.size());
        mp_string fcScanDiskCmd = "echo \"1\" > /sys/class/fc_host/" + (*it) + "/issue_lip";
        DBGLOG("\"%s\" fcScanDiskCmd .", fcScanDiskCmd.c_str());
        iRet = CSystemExec::ExecSystemWithoutEcho(fcScanDiskCmd);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Exec system cmd failed, cmd is %s, iRet is %d. task '%s', parent task '%s'.",
                fcScanDiskCmd.c_str(), iRet, m_strTaskID.c_str(), m_strParentTaskID.c_str());
            return;
        }
    }
}

bool VMwareNativeDataPathImpl::CheckDiskIsMappedByWwn()
{
    if (m_transportModeSelected != "hotadd") {
        return false;
    }
    std::string diskUuid = m_volumeInfo.strDiskID;
    INFOLOG("The device transport mode is hotadd, diskId:%s.", diskUuid.c_str());
    diskUuid.erase(std::remove(diskUuid.begin(), diskUuid.end(), '-'), diskUuid.end());
    std::transform(diskUuid.begin(), diskUuid.end(), diskUuid.begin(), ::tolower);
    std::string wwn = diskUuid;

    std::string cmd = "ls -al /dev/disk/by-id/scsi-3" + wwn;
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(cmd);
    if (iRet == MP_SUCCESS) {
        INFOLOG("The device is mapped, cmd is %s, strDiskPath is %s. task '%s', parent task '%s'.",
            cmd.c_str(),
            m_volumeInfo.strDiskPath.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return true;
    }
    return false;
}

mp_int32 VMwareNativeDataPathImpl::VMwareNativeBackupOpenDisk(Json::Value &bodyMsg)
{
    m_protectType = VMWARE_VM_BACKUP;
    INFOLOG("Enter VMwareNativeBackupOpenDisk, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    // prepare volume params
    if (ParseVolumeParams(bodyMsg) != MP_SUCCESS) {
        ERRLOG("Unable to parse volume detail, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    // open disk
    if (!m_isDiskOpened) {
        mp_int32 openMode = (m_protectType == VMWARE_VM_BACKUP) ? VMWAREDISK_FLAG_OPEN_READ_ONLY
                                                                : VMWAREDISK_FLAG_OPEN_READ_WRITE;
        if (OpenDisk(openMode) != MP_SUCCESS) {
            ERRLOG("Unable to open disk '%s', task '%s', parent task '%s'.",
                m_volumeInfo.strDiskID.c_str(),
                m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
            return MP_FAILED;
        }
        m_isDiskOpened = true;
        // RDM disk
        if (IsRDMDisk()) {
            INFOLOG("disk is rdm disk, task '%s', parent task '%s'.",
                    m_strTaskID.c_str(),
                    m_strParentTaskID.c_str());
            if (GetLocalDevicePathByWwn(m_volumeInfo.strWwn, m_localDevicePath) != MP_SUCCESS) {
                ERRLOG("Unable to Get local device by wwn, diskId '%s', task '%s', parent task '%s'.",
                       m_volumeInfo.strDiskID.c_str(),
                       m_strTaskID.c_str(),
                       m_strParentTaskID.c_str());
                return MP_FAILED;
            }
        }
    }

    // Query Allocated Blocks
    if (m_isDiskOpened) {
        if (m_spVMwareDiskApi->QueryAllocatedBlocks(bodyMsg) != MP_SUCCESS) {
            ERRLOG("Unable to query allocated blocks disk '%s', task '%s', parent task '%s'.",
                m_volumeInfo.strDiskID.c_str(),
                m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
            return MP_FAILED;
        }
    } else {
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

void VMwareNativeDataPathImpl::JsonArray2SpecifiedArr(
    const Json::Value &jsonVal, std::vector<std::string> &specifiedLst)
{
    if (!jsonVal.isMember(PARAM_KEY_EXCLUDE_SPECIFIED_LIST)) {
        return;
    }
    size_t size = jsonVal[PARAM_KEY_EXCLUDE_SPECIFIED_LIST].size();
    for (Json::Value::UInt idx = 0; idx < size; idx++) {
        specifiedLst.push_back(jsonVal[PARAM_KEY_EXCLUDE_SPECIFIED_LIST][idx].asString());
    }
}

mp_int32 VMwareNativeDataPathImpl::JsonArray2ExcludeDataArr(const Json::Value &jsonVal, DataExclusion &exclInfos)
{
    if (!jsonVal.isObject()) {
        ERRLOG("The element of array is not a Json object.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_int32 ret = CJsonUtils::GetJsonBool(jsonVal, PARAM_KEY_EXCLUDE_DELETED_FILES, exclInfos.m_deletedFiles);
    if (ret != MP_SUCCESS) {
        ERRLOG("Get the deletedFiles option fialed.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    ret = CJsonUtils::GetJsonBool(jsonVal, PARAM_KEY_EXCLUDE_TMP_FILES, exclInfos.m_tmpFiles);
    if (ret != MP_SUCCESS) {
        ERRLOG("Get the tmpFiles option fialed.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    ret = CJsonUtils::GetJsonBool(jsonVal, PARAM_KEY_EXCLUDE_SPECIFIED_OPTION, exclInfos.m_specified);
    if (ret != MP_SUCCESS) {
        ERRLOG("Get the specified option fialed.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    if (exclInfos.m_specified) {
        (void)JsonArray2SpecifiedArr(jsonVal, exclInfos.m_specifiedList);
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::ParseExcludeDataParams(const Json::Value &msg, DataExclusion &info)
{
    int32_t iRet = 0;
    Json::Value jsonMsgBody = msg[MANAGECMD_KEY_BODY];
    try {
        if (!jsonMsgBody.isObject() || !jsonMsgBody.isMember(PARAM_KEY_EXCLUDE_LIST)) {
            INFOLOG("The InvalidDataReduction key is not found.");
            return MP_SUCCESS;
        }
        Json::Value jsonExcludes = jsonMsgBody[PARAM_KEY_EXCLUDE_LIST];
        iRet = JsonArray2ExcludeDataArr(jsonExcludes, info);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Unable to parse exclude data attrs");
            return iRet;
        }
    } catch (...) {
        ERRLOG("Catch a exception");
        return MP_FAILED;
    }
    INFOLOG("Parse exclude params ok.");
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::JsonArray2AfsVolumeArr(
    const std::vector<Json::Value> &jsonArr, std::vector<FilterDIiskInfo> &volumes)
{
    volumes.clear();
    Json::Value jsonMsgBody;
    mp_size arrSize = jsonArr.size();
    for (mp_size i = 0; i < arrSize; i++) {
        jsonMsgBody = jsonArr[i];
        if (!jsonMsgBody.isObject()) {
            ERRLOG("The element of array is not a Json object.");
            return ERROR_COMMON_INVALID_PARAM;
        }

        FilterDIiskInfo info;
        GET_JSON_STRING(jsonMsgBody, MANAGECMD_KEY_TASKID, info.m_taskID);
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISKID, info.m_diskID);
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISKTYPE, info.m_diskType);
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_WWN, info.m_strWwn);
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISKPATH, info.m_strDiskPath);
        GET_JSON_UINT64(jsonMsgBody, PARAM_KEY_VOLUME_DISKSIZE, info.m_diskSizeInBytes);
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VMFSIO_FLAG, info.m_vmfsIOFlag);
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISK_MOUNT_PATH, info.m_nasIOFlag);
        GET_JSON_STRING(jsonMsgBody, PARAM_KEY_VOLUME_DISK_RELATIVE_PATH, info.m_strDiskRelativePath);
        (void)CJsonUtils::GetJsonArrayString(jsonMsgBody, PARAM_KEY_VOLUME_DISK_DATASTORE_WWN, info.datastoreWwn);
        volumes.push_back(info);
    }

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::ParseSnapshotRef(const Json::Value &jsonMsgBody)
{
    std::string snapshotRef;
    if (jsonMsgBody.isObject() && jsonMsgBody.isMember(EXT_CMD_PROTECT_VM_INFO) &&
        jsonMsgBody[EXT_CMD_PROTECT_VM_INFO].isMember(PARAM_KEY_VM_SNAPSHOTREF)) {
        GET_JSON_STRING(jsonMsgBody[EXT_CMD_PROTECT_VM_INFO], PARAM_KEY_VM_SNAPSHOTREF, snapshotRef);
        INFOLOG("The strSnapshotRef:%s", snapshotRef.c_str());
        m_vmProtectionParams.vmInfo.strSnapshotRef = snapshotRef;
        return MP_SUCCESS;
    }
    ERRLOG("The strSnapshotRef is not found");
    return MP_FAILED;
}

mp_int32 VMwareNativeDataPathImpl::ParseAfsVolumesInfo(const Json::Value &msg, std::vector<FilterDIiskInfo> &volumes)
{
    Json::Value jsonMsgBody = msg[MANAGECMD_KEY_BODY];
    if (ParseSnapshotRef(jsonMsgBody) != MP_SUCCESS) {
        return MP_FAILED;
    }

    static const mp_string strKey = "Volumes";
    std::vector<Json::Value> jsonVolumes;
    mp_int32 iRet = 0;
    try {
        iRet = CJsonUtils::GetJsonArrayJson(jsonMsgBody, strKey, jsonVolumes);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Unable to parse json of key '%s', ret: '%d',.", strKey.c_str(), iRet);
            return iRet;
        }
        iRet = JsonArray2AfsVolumeArr(jsonVolumes, volumes);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Unable to parse afs volumes attrs");
            return iRet;
        }
    } catch (...) {
        ERRLOG("Parse json has an exception");
        return MP_FAILED;
    }
    INFOLOG("Exit ParseAfsVolumesInfo");
    return MP_SUCCESS;
}

void VMwareNativeDataPathImpl::FillVmwareTmpVolumeInfo(const FilterDIiskInfo &afsVolParams)
{
    m_volumeInfo.strDiskType = afsVolParams.m_diskType;
    m_volumeInfo.strDiskID = afsVolParams.m_diskID;
    m_volumeInfo.strDiskRelativePath = afsVolParams.m_strDiskRelativePath;
    m_volumeInfo.vmfsIOFlag = afsVolParams.m_vmfsIOFlag;
    m_volumeInfo.nasIOFlag = afsVolParams.m_nasIOFlag;
    m_volumeInfo.strWwn = afsVolParams.m_strWwn;
    m_volumeInfo.datastoreWwn = afsVolParams.datastoreWwn;
    m_volumeInfo.strDiskPath = afsVolParams.m_strDiskPath;
    m_volumeInfo.ulDiskSize = afsVolParams.m_diskSizeInBytes;
}

bool VMwareNativeDataPathImpl::FillDiskHandleParams(
    const FilterDIiskInfo &info, const std::shared_ptr<VMwareDiskApi> &api, DiskHandleInfo &handle)
{
    handle.m_diskUuid = info.m_diskID;
    handle.m_taskId = info.m_taskID;
    handle.m_parentTaskID = info.m_taskID;
    handle.m_diskPath = info.m_strDiskPath;
    handle.m_diskSizeInBytes = info.m_diskSizeInBytes;

    mp_uint64 chunkSize = 512;
    (void)CConfigXmlParser::GetInstance().GetValueUint64(CFG_DATAPROCESS_SECTION, CFG_CHUNK_SIZE, chunkSize);
    handle.m_trunkSize = chunkSize;

    // 磁盘的存储仓库路径
    const mp_string nfsMountPoint = "/opt/advbackup/vmware/data/";
    handle.m_diskBitmapPath = nfsMountPoint + info.m_taskID + PATH_SEPARATOR + info.m_diskID;

    std::shared_ptr<IOEngine> reader;
    if ((m_volumeInfo.vmfsIOFlag == "vmfsio")) {
        INFOLOG("The vmfsio volume backup.");
        reader = CreateVmfsIOEngine();
    } else if (!m_volumeInfo.nasIOFlag.empty()) {
        INFOLOG("The nasIOFlag volume backup.");
        reader = std::make_shared<FileIOEngine>(m_volumeInfo, VMWARE_VM_NFS_BACKUP, m_snapType);
    } else {
        INFOLOG("The vddk volume backup.");
        reader = std::make_shared<VMwareIOEngine>(api);
    }

    if (reader == nullptr || reader->Open() != MP_SUCCESS) {
        ERRLOG("IOEngine open failed.");
        return false;
    }
    handle.m_reader = reader;
    handle.m_diskApi = api;
    return true;
}

void VMwareNativeDataPathImpl::CloseAllDisksForAfsBitmap(const std::vector<DiskHandleInfo> &vecDiskHandle)
{
    mp_string strErrDesc;
    for (const auto &it : vecDiskHandle) {
        if (it.m_diskApi != nullptr && it.m_diskApi.get() != nullptr) {
            it.m_diskApi->CloseDisk(strErrDesc);
        }
        if (it.m_reader != nullptr && it.m_reader.get() != nullptr) {
            it.m_reader->Close();
        }
    }
}

mp_int32 VMwareNativeDataPathImpl::GetDevicePathForRdm()
{
    if (!IsRDMDisk()) {
        return MP_SUCCESS;
    }
    INFOLOG("disk is rdm disk, task '%s', parent task '%s'.", m_strTaskID.c_str(), m_strParentTaskID.c_str());
    if (GetLocalDevicePathByWwn(m_volumeInfo.strWwn, m_localDevicePath) != MP_SUCCESS) {
        ERRLOG("Unable to Get local device by wwn, diskId '%s', task '%s', parent task '%s'.",
            m_volumeInfo.strDiskID.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::OpenDisksForAfsBitmap(
    const std::vector<FilterDIiskInfo> &infos, std::vector<DiskHandleInfo> &vecDiskHandle)
{
    m_protectType = VMWARE_VM_BACKUP;
    for (const auto &it : infos) {
        FillVmwareTmpVolumeInfo(it); // 设置m_volumeInfo信息
        m_diskType = it.m_diskType;
        if (GetDevicePathForRdm() != MP_SUCCESS) {
            return MP_FAILED;
        }
        std::shared_ptr<VMwareDiskApi> diskApi;
        if (!m_volumeInfo.nasIOFlag.empty() || !m_volumeInfo.vmfsIOFlag.empty()) {
            INFOLOG("Storage nas access");
        } else {
            if (OpenDiskForAfsBitmapInner(diskApi) != MP_SUCCESS) {
                return MP_FAILED;
            }
        }

        DiskHandleInfo handle;
        if (!FillDiskHandleParams(it, diskApi, handle)) {
            return MP_FAILED;
        }
        vecDiskHandle.push_back(handle);
    }
    INFOLOG("Exit OpenDisksForAfsBitmap");
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::VMwareNativeBackupAfsBitmap(Json::Value &bodyMsg, mp_string &strError)
{
    std::string jsonRepStr;
    (void)WipeSensitiveForJsonData(bodyMsg.toStyledString(), jsonRepStr);
    INFOLOG("Enter VMwareNativeBackupAfsBitmap...bodyMsg: %s", jsonRepStr.c_str());

    DataExclusion excelInfo;
    if (ParseExcludeDataParams(bodyMsg, excelInfo) != MP_SUCCESS) {
        return MP_FAILED;
    }

    if (!excelInfo.IsEnableDeleted()) {
        INFOLOG("The deleted option is not enable.");
        return MP_SUCCESS;
    }

    std::vector<FilterDIiskInfo> vecAfsVolumes;
    if (ParseAfsVolumesInfo(bodyMsg, vecAfsVolumes) != MP_SUCCESS) {
        return MP_FAILED;
    }

    if (vecAfsVolumes.size() == 0) {
        WARNLOG("The volume list is empty.");
        return MP_SUCCESS;
    }

    m_strTaskID = vecAfsVolumes[0].m_taskID;
    m_strParentTaskID = vecAfsVolumes[0].m_taskID;

    std::vector<DiskHandleInfo> vecDiskHandle;
    if (OpenDisksForAfsBitmap(vecAfsVolumes, vecDiskHandle) != MP_SUCCESS) {
        strError = "Open disk failed.";
        CloseAllDisksForAfsBitmap(vecDiskHandle);
        return MP_FAILED;
    }

    // 获取多磁盘的无效数据差量位图
    AfsProcess afs;
    uint64_t startTime = CMpTime::GetTimeUsec();
    int32_t ret = afs.GetAndSaveAfsDirtyRanges(vecDiskHandle, excelInfo, strError);
    uint64_t finishTime = CMpTime::GetTimeUsec();
    uint64_t afsTime = finishTime - startTime;
    bodyMsg.clear();
    bodyMsg["afsTime"] = afsTime;
    
    CloseAllDisksForAfsBitmap(vecDiskHandle);
    INFOLOG("Exit VMwareNativeBackupAfsBitmap, time taken : %d", afsTime);
    return ret;
}

mp_int32 VMwareNativeDataPathImpl::VMwareNativeDataBlockRestore(Json::Value &bodyMsg, mp_string &strError)
{
    m_preTime = CMpTime::GetTimeUsec();
    // 设置任务类型
    m_protectType = VMWARE_VM_RECOVERY;
    this->m_isClose = false;
    m_preDataSizeFinished = 0;
    m_progress = 0;

    // 预处理，解析参数、打开VMDK磁盘等
    if (PrepareProcess(bodyMsg, strError) != MP_SUCCESS) {
        return MP_FAILED;
    }

    if (m_iDirtyRangeSize == 0) {
        this->m_isClose = true;
        return MP_SUCCESS;
    }

    // 设置恢复任务的读写端，读为备份存储，写为VMware
    if (m_spVMwareDiskApi == nullptr) {
        strError = "VMware native datablock restore : product has not been initialized";
        ERRLOG("Product has not been initialized, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }
    m_reader = std::make_shared<FileIOEngine>(m_volumeInfo, m_protectType, m_snapType);
    m_writer = std::make_shared<VMwareIOEngine>(m_spVMwareDiskApi);
    m_completedBlocks.store(0);

    // 另起线程处理数据块
    m_joinableThrs.push_back(std::thread(&VMwareNativeDataPathImpl::ProcessDiskBlocks, this));
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::PrepareProcessPreJob(Json::Value &bodyMsg, mp_string &errDesc)
{
    INFOLOG("Enter to PrepareProcess , task '%s', parent task '%s'.", m_strTaskID.c_str(), m_strParentTaskID.c_str());
    // parse dirty range info
    if (ParseDirtyRangesParams(bodyMsg) != MP_SUCCESS) {
        errDesc = "Unable to parse volume dirty range detail.";
        ERRLOG("Unable to parse volume dirty range detail, task id '%s'.", m_strTaskID.c_str());
        return MP_FAILED;
    }

    // prepare volume params
    if (!m_isDiskOpened && ParseVolumeParams(bodyMsg) != MP_SUCCESS) {
        errDesc = "Unable to parse volume detail.";
        ERRLOG("Unable to parse volume detail, task '%s', parent task '%s'.",
            m_strTaskID.c_str(), m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    // 增加解析无效数据识别选项
    if (m_protectType == VMWARE_VM_BACKUP) {
        DataExclusion info;
        if (ParseExcludeDataParams(bodyMsg, info) == MP_SUCCESS && info.IsEnableDeleted()) {
            m_volumeInfo.dataExclude = info;
        }
    }

    // RDM disk
    if (IsRDMDisk()) {
        INFOLOG("disk is rdm disk, task '%s', parent task '%s'.", m_strTaskID.c_str(), m_strParentTaskID.c_str());
        if (GetLocalDevicePathByWwn(m_volumeInfo.strWwn, m_localDevicePath) != MP_SUCCESS) {
            ERRLOG("Unable to Get local device by wwn, diskId '%s', task '%s', parent task '%s'.",
                m_volumeInfo.strDiskID.c_str(), m_strTaskID.c_str(), m_strParentTaskID.c_str());
            return MP_FAILED;
        }
    }
    // parse target lun
    if (m_backendStorageProtocol == VMWAREDEF::VMWARE_STORAGE_PROTOCOL_ISCSI &&
        MatchSpecificTargetLun(m_volumeInfo.strMediumID) != MP_SUCCESS) {
        errDesc = "Unable to match a target lun using volume WWN.";
        ERRLOG("Unable to match a target lun using volume WWN '%s', task '%s'.",
            m_volumeInfo.strMediumID.c_str(), m_strTaskID.c_str());
        return MP_FAILED;
    }

    // if dirty range is zero, return directly
    m_iDirtyRangeSize = m_volumeInfo.vecDirtyRange.size();
    if (m_iDirtyRangeSize == 0) {
        INFOLOG("No data needs to be protected, task '%s', parent task '%s'.",
            m_strTaskID.c_str(), m_strParentTaskID.c_str());
    }

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::PrepareProcess(Json::Value &bodyMsg, mp_string &errDesc)
{
    if (PrepareProcessPreJob(bodyMsg, errDesc) != MP_SUCCESS) {
        ERRLOG("PrepareProcessPreJob failed.");
        return MP_FAILED;
    }

    // open disk
    DBGLOG("Storage san access check, disk vmfs path: %s", m_volumeInfo.strDiskRelativePath.c_str());
    if (!m_volumeInfo.nasIOFlag.empty()) {
        m_isDiskOpened = true;
        m_transportModeSelected = VMWARE_TRANSMODE_STORAGE_NAS;
        INFOLOG("Storage nas access, set transport mode to '%s', taskId: %s, parent task id: %s",
            m_transportModeSelected.c_str(), m_strTaskID.c_str(), m_strParentTaskID.c_str());
    } else if (!m_volumeInfo.vmfsIOFlag.empty()) {
        m_isDiskOpened = true;
        m_transportModeSelected = VMWARE_TRANSMODE_STORAGE_LUN;
        INFOLOG("Storage lun access, set transport mode to '%s', taskId: %s, parent task id: %s",
            m_transportModeSelected.c_str(), m_strTaskID.c_str(), m_strParentTaskID.c_str());
    } else {
        INFOLOG(
            "Open disk via vddk, \
            m_isDiskOpened: %d, dirtyRangeSize: %lld, taskId: %s, parent task id: %s",
            m_isDiskOpened, m_iDirtyRangeSize, m_strTaskID.c_str(), m_strParentTaskID.c_str());
        if (!m_isDiskOpened && m_iDirtyRangeSize != 0) {
            /*
            *  Note that the virtual disk should be opened by specific mode
            *  1. backup- VMWAREDISK_FLAG_OPEN_READ_ONLY
            *  2. recovery - VMWAREDISK_FLAG_OPEN_READ_WRITE
            */
            mp_int32 openMode = (m_protectType == VMWARE_VM_BACKUP) ? VMWAREDISK_FLAG_OPEN_READ_ONLY
                                                                    : VMWAREDISK_FLAG_OPEN_READ_WRITE;
            if (OpenDisk(openMode) != MP_SUCCESS) {
                errDesc = "Unable to open disk.";
                ERRLOG("Unable to open disk '%s', task '%s', parent task '%s'.",
                    m_volumeInfo.strDiskID.c_str(), m_strTaskID.c_str(), m_strParentTaskID.c_str());
                return MP_FAILED;
            } else {
                m_isDiskOpened = true;
            }
        }
    }

    INFOLOG("Transport mode selected '%s', taskId: %s, parent task id: %s",
        m_transportModeSelected.c_str(), m_strTaskID.c_str(), m_strParentTaskID.c_str());

    m_snapType = m_vmProtectionParams.ulSnapType;
    UpdateSubTaskQos();

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::GetJobThreadNum(int jobType)
{
    mp_int32 jobThrsNum = 2;
    mp_string cfgItem;
    switch (jobType) {
        case VMWARE_VM_BACKUP:
            cfgItem = "max_backup_threads";
            break;
        case VMWARE_VM_RECOVERY:
            cfgItem = "max_restore_threads";
            break;
        default:
            COMMLOG(OS_LOG_WARN, "Job type: '%d' not be supported, use default value: '%d'", jobType, jobThrsNum);
            return jobThrsNum;
    }

    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_DATAPROCESS_SECTION, cfgItem, jobThrsNum);
    if (iRet != MP_SUCCESS || jobThrsNum > MAX_THREAD_NUM) {
        COMMLOG(OS_LOG_WARN, "Get thread number for job type: '%d' from configuration file failed, "
            "use default value: '%d'", jobType, jobThrsNum);
        jobThrsNum = MAX_THREAD_NUM;
    }
    return jobThrsNum;
}

mp_int32 VMwareNativeDataPathImpl::ProcessDiskBlocksStart(
    TaskScheduler &ts, mp_int32 &nTasks, bool &failure, mp_int32 &jobThrsNum, mp_uint64 &totalSize)
{
    auto it = m_volumeInfo.vecDirtyRange.begin();
    for (; (!failure) && (it != m_volumeInfo.vecDirtyRange.end());) {
        // check exit flag
        if (m_exitFlag == true) {
            break;
        }

        mp_uint64 bufSize = VMWARE_DATABLOCK_SIZE;
        if (m_volumeInfo.ulDiskSize <= it->start) {
            ERRLOG("Invalid dirty range, disk size: '%llu', but IO offset: '%llu'", m_volumeInfo.ulDiskSize, it->start);
            break;
        }
        if (m_volumeInfo.ulDiskSize < (it->start + VMWARE_DATABLOCK_SIZE)) {
            bufSize = m_volumeInfo.ulDiskSize - it->start;
            INFOLOG("The block that start '%llu' is small than '%d'", it->start, VMWARE_DATABLOCK_SIZE);
        }

        /* goNext用于标记能否继续往调度器中添加任务，如果不能则重新加入进行重试 */
        std::shared_ptr<IOTask> task = std::make_shared<IOTask>(m_protectType, it->start, bufSize);
        task->Reader(m_reader);
        task->Writer(m_writer);
        bool goNext = ExecIOTask(ts, task, nTasks, failure, jobThrsNum);
        if (goNext) {
            if (m_isQosLimitSpeed) {
                JobQosManager::GetJobQosManager(m_volumeInfo.strTaskID)->ConsumeQos((bufSize / UNIT_M), -1);
            }
            ++it;
        }

        if (failure) {
            ERRLOG("IO task failed.");
            this->m_isTaskSuccess = false;
            break;
        }
    }
    if (this->m_isTaskSuccess) {
        INFOLOG("Obtain data block completed, task '%s', parent task '%s'.",
                m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
    }
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 VMwareNativeDataPathImpl::ProcessDiskBlocksStartForAioDatamoverForBackup(
    TaskScheduler &ts, mp_int32 &nTasks, bool &failure, mp_int32 &jobThrsNum, mp_uint64 &totalSize)
{
    // 开始异步读，同步写
    INFOLOG("open file name: %s", m_writer->GetFileNameForWrite().c_str());
    int fh = open(m_writer->GetFileNameForWrite().c_str(), O_RDWR);
    if (fh < 0) {
        int errnoCopy = errno;
        std::stringstream err;
        err << "Failed to open " << m_writer->GetFileNameForWrite() << " for read-only, errno=" << errnoCopy;
        ERRLOG("err: %ls", err.str().c_str());
        return MP_FAILED;
    }
    std::shared_ptr<int> tempHandlePtr = std::shared_ptr<int>(new (std::nothrow) int(fh), [](int* fd) {
        if (fd) {
            close(*fd);
            delete (fd);
        }
    });
    for (auto aa : m_volumeInfo.vecDirtyRange) {
        DBGLOG("DERTYRANGE---start: %llu, length: %llu", aa.start, aa.length);
    }
    if (m_volumeInfo.vecDirtyRange.size() > 0) {
        AsioDataMoverForBackup temp(*tempHandlePtr, m_reader, 0, m_volumeInfo.vecDirtyRange, true);
        while (temp.GetNumThreads() != 0) {
            if (m_exitFlag == true) {
                COMMLOG(OS_LOG_INFO, "begin to abort");
                temp.isAbortRequested.store(true);
            }
            m_completedBlocks = temp.GetCompleteBlocksCount();
            COMMLOG(OS_LOG_DEBUG, "m_completedBlocks %llu, m_volumeInfo.vecDirtyRange: %llu", m_completedBlocks.load(),
                m_volumeInfo.vecDirtyRange.size());
            sleep(1);
        }
        m_completedBlocks.store(temp.GetCompleteBlocksCount());
        COMMLOG(OS_LOG_INFO, "m_completedBlocks %llu, total size: %llu", m_completedBlocks.load(),
            m_volumeInfo.vecDirtyRange.size());
        if (m_completedBlocks.load() != m_volumeInfo.vecDirtyRange.size()) {
            COMMLOG(OS_LOG_WARN, "fix m_completedBlocks");
            m_completedBlocks.store(m_volumeInfo.vecDirtyRange.size());
        }
        m_isTaskSuccess = true;
        if (temp.IsAborted()) {
            INFOLOG("datamover execute failed");
            m_isTaskSuccess = false;
        }
    }
    if (this->m_isTaskSuccess) {
        INFOLOG("Obtain data block completed, task '%s', parent task '%s'.", m_strTaskID.c_str(),
                m_strParentTaskID.c_str());
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::ProcessDiskBlocksStartForAioDatamoverForRestore(
    TaskScheduler &ts, mp_int32 &nTasks, bool &failure, mp_int32 &jobThrsNum, mp_uint64 &totalSize)
{
    std::string filename = m_reader->GetFileName();
    // 开始异步读，同步写
    int fh = open(filename.c_str(), O_RDONLY, 0644);
    if (fh < 0) {
        int errnoCopy = errno;
        std::stringstream err;
        err << "Failed to open " << filename << " for read-only, errno=" << errnoCopy;
        ERRLOG("err: %ls", err.str().c_str());
        return MP_FAILED;
    }
    std::shared_ptr<int> tempHandlePtr = std::shared_ptr<int>(new (std::nothrow) int(fh), [](int* fd) {
        if (fd) {
            INFOLOG("begin to close");
            close(*fd);
            delete (fd);
            INFOLOG("end to close");
        }
    });
    for (auto aa : m_volumeInfo.vecDirtyRange) {
        DBGLOG("DERTYRANGE---start: %llu, length: %llu", aa.start, aa.length);
    }
    if (m_volumeInfo.vecDirtyRange.size() > 0) {
        AsioDataMoverForRestore temp(*tempHandlePtr, m_writer, 0, m_volumeInfo.vecDirtyRange, false);
        while (temp.GetNumThreads() != 0) {
            if (m_exitFlag == true) {
                COMMLOG(OS_LOG_INFO, "begin to abort");
                temp.isAbortRequested.store(true);
            }
            m_completedBlocks = temp.GetCompleteBlocksCount();
            COMMLOG(OS_LOG_DEBUG, "m_completedBlocks %llu, m_volumeInfo.vecDirtyRange: %llu", m_completedBlocks.load(),
                m_volumeInfo.vecDirtyRange.size());
            sleep(1);
        }
        m_completedBlocks.store(temp.GetCompleteBlocksCount());
        COMMLOG(OS_LOG_WARN, "m_completedBlocks %llu, total size: %llu", m_completedBlocks.load(),
            m_volumeInfo.vecDirtyRange.size());
        if (m_completedBlocks.load() != m_volumeInfo.vecDirtyRange.size()) {
            m_completedBlocks.store(m_volumeInfo.vecDirtyRange.size());
        }
        m_isTaskSuccess = true;
        if (temp.IsAborted()) {
            INFOLOG("datamover execute failed");
            m_isTaskSuccess = false;
        }
    }
    if (this->m_isTaskSuccess) {
        INFOLOG("Obtain data block completed, id '%s', parent id '%s'", m_strTaskID.c_str(), m_strParentTaskID.c_str());
    }
    return MP_SUCCESS;
}

void VMwareNativeDataPathImpl::PreFillDiskHandleParams(DiskHandleInfo &handle)
{
    handle.m_diskUuid = m_volumeInfo.strDiskID;
    handle.m_taskId = m_volumeInfo.strTaskID;
    handle.m_parentTaskID = m_volumeInfo.strParentTaskID;
    handle.m_diskPath = m_volumeInfo.strDiskPath;
    handle.m_diskSizeInBytes = m_volumeInfo.ulDiskSize;
    mp_uint64 chunkSize = 512;
    (void)CConfigXmlParser::GetInstance().GetValueUint64(CFG_DATAPROCESS_SECTION, CFG_CHUNK_SIZE, chunkSize);
    handle.m_trunkSize = chunkSize;

    const mp_string nfsMountPoint = "/opt/advbackup/vmware/data/";
    handle.m_diskBitmapPath = nfsMountPoint + m_volumeInfo.strTaskID + PATH_SEPARATOR + m_volumeInfo.strDiskID;
    handle.m_reader = m_reader;
    handle.m_diskApi = m_spVMwareDiskApi;
}

void VMwareNativeDataPathImpl::FilterAfsBitmapForBackup(AfsProcess &afs, DiskHandleInfo& info)
{
    INFOLOG("Enter FilterAfsBitmapForBackup, blocks: %d", m_volumeInfo.vecDirtyRange.size());
    size_t ogrBlocks = m_volumeInfo.vecDirtyRange.size();
    PreFillDiskHandleParams(info);
    std::vector<tag_dirty_range_info> newDr;
    int32_t ret = afs.GetDiskFilterDirtyRanges(m_volumeInfo.vecDirtyRange, info, m_volumeInfo.dataExclude, newDr);
    if (ret != MP_SUCCESS) {
        WARNLOG("Get disk free dirty ranges failed, diskId: %s, task: %s",
            m_volumeInfo.strDiskID.c_str(),
            m_strTaskID.c_str());
        this->m_errDesc = "Get disk free dirty ranges failed.";
        return;
    }

    uint32_t reduceBlocks = ogrBlocks - newDr.size();
    m_reducedBlocks += reduceBlocks;

    m_volumeInfo.vecDirtyRange = newDr;
    // 更新实际的备份数据块数量
    m_iDirtyRangeSize = m_volumeInfo.vecDirtyRange.size();
    INFOLOG("Exit filter dirty range ok, blocks: %d, redction blocks: %d", ogrBlocks, reduceBlocks);
}

void VMwareNativeDataPathImpl::FilterDiskBitmap()
{
    AfsProcess afs;
    DiskHandleInfo info;
    bool enAbleAfs = (m_protectType == VMWARE_VM_BACKUP) && (m_volumeInfo.dataExclude.IsEnableDeleted());
    if (enAbleAfs) {
        INFOLOG("The InvalidDataReduction is anable.");
        FilterAfsBitmapForBackup(afs, info);
    }
}

mp_bool VMwareNativeDataPathImpl::GetUseAioRunTask()
{
    mp_bool useAio = false;
    std::string aioKey = m_protectType == VMWARE_VM_RECOVERY ?
                         CFG_DATAPROCESS_USE_AIO_RESTORE : CFG_DATAPROCESS_USE_AIO_BACKUP;
    int iRet = CConfigXmlParser::GetInstance().GetValueBool(CFG_DATAPROCESS_SECTION, aioKey, useAio);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Read use_aio failed");
        useAio = false;
    }
    return useAio;
}

mp_int32 VMwareNativeDataPathImpl::ProcessDiskBlocksStartForAioDatamover(TaskScheduler &ts, mp_int32 &nTasks,
    bool &failure, mp_int32 &jobThrsNum, mp_uint64 &totalSize)
{
    if (m_protectType == VMWARE_VM_BACKUP) {
        return ProcessDiskBlocksStartForAioDatamoverForBackup(ts, nTasks, failure, jobThrsNum, totalSize);
    } else if (m_protectType == VMWARE_VM_RECOVERY) {
        return ProcessDiskBlocksStartForAioDatamoverForRestore(ts, nTasks, failure, jobThrsNum, totalSize);
    }
    ERRLOG("Abnormal branch");
    return MP_FAILED;
}

void VMwareNativeDataPathImpl::ProcessDiskBlocks()
{
    INFOLOG("Create thread '%llu' to process disk blocks, task '%s', parent task '%s'", std::this_thread::get_id(),
        m_strTaskID.c_str(), m_strParentTaskID.c_str());

    // 设置本线程对线程取消信号的反应：收到信号后立即执行取消
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    m_mutex.lock();
    TaskPool *tp = TaskPool::GetInstance();
    m_mutex.unlock();
    if (tp == nullptr) {
        ERRLOG("Task pool init failed.");
        this->m_isTaskSuccess = false;
        return;
    }

    // 任意分支退出时关闭读写端
    SCOPE_EXIT
    {
        INFOLOG("Close read and write operation handlers will be triggered, thread '%llu', task '%s', parent task '%s'",
            std::this_thread::get_id(), m_strTaskID.c_str(), m_strParentTaskID.c_str());
        CloseRW();
    };
    if (OpenRW() != MP_SUCCESS) {  // 打开任务的读写端
        ERRLOG("Open reader or writer failed, thread '%llu', task '%s', parent task '%s'",
            std::this_thread::get_id(), m_strTaskID.c_str(), m_strParentTaskID.c_str());
        this->m_isTaskSuccess = false;
        return;
    }

    INFOLOG("Open read and write operation handlers successfully, thread '%llu', task '%s', parent task '%s',"
        "dirty range count %llu", std::this_thread::get_id(), m_strTaskID.c_str(), m_strParentTaskID.c_str(),
        m_volumeInfo.vecDirtyRange.size());

    // 过滤磁盘无效数据差量位图
    FilterDiskBitmap();

    mp_int32 jobThrsNum = GetJobThreadNum(m_protectType);
    TaskScheduler ts(*tp);
    mp_int32 nTasks = 0;  // 当前尚在执行的任务数
    bool failure = false;
    mp_uint64 totalSize = 0;  // MB
    if (GetUseAioRunTask()) {
        if (ProcessDiskBlocksStartForAioDatamover(ts, nTasks, failure, jobThrsNum, totalSize) != MP_SUCCESS) {
            return;
        }
    } else {
        if (ProcessDiskBlocksStart(ts, nTasks, failure, jobThrsNum, totalSize) != MP_SUCCESS) {
            return;
        }
    }
    ProcessDiskBlocksEnd(ts, nTasks, failure);
}

mp_void VMwareNativeDataPathImpl::ProcessDiskBlocksEnd(TaskScheduler &ts, mp_int32 &nTasks, bool &failure)
{
    /* 处理尚未结束的任务的执行结果 */
    std::shared_ptr<Task> res;
    while (nTasks) {
        if (ts.Get(res, true) == false) {
            ERRLOG("Get io task result failed");
            failure = true;
            this->m_isTaskSuccess = false;
            break;
        }
        if (res->Result() != 0) {
            ERRLOG("Task failed, &task='%llu', res='%d'", (mp_uint64)res.get(), res->Result());
            failure = true;
            this->m_isTaskSuccess = false;
            this->m_errDesc = res->GetErrDesc();
        }
        if (res->IsZero()) {
            ++m_zeroBlocks;
        }
        --nTasks;
        ++m_completedBlocks;  // 已处理完的Block数量加一
    }

    m_totalCompletedBlocks += m_completedBlocks.load();
    INFOLOG("Blocks process failure: '%d', DirtyRange size: '%llu', TotalCompletedBlocks: '%llu',"
            "actual completed blocks: '%llu', zero blocks: '%llu, 'task '%s',parent task '%s'.", failure,
            m_volumeInfo.vecDirtyRange.size(), m_totalCompletedBlocks.load(),
            m_completedBlocks.load(), m_zeroBlocks.load(), m_strTaskID.c_str(), m_strParentTaskID.c_str());
}

bool VMwareNativeDataPathImpl::ExecIOTask(
    TaskScheduler &ts, std::shared_ptr<IOTask> &task, mp_int32 &nTasks, bool &failure, mp_int32 taskThrsNum)
{
    bool goNext = false;
    failure = false;
    if (nTasks < taskThrsNum) {
        if (!ts.Put(task)) {
            ERRLOG("Put io task to pool failed");
            return goNext;
        }

        ++nTasks;
        goNext = true;
        return goNext;
    }

    std::shared_ptr<Task> res;
    const int timeOut = 5;  // MS
    while (ts.Get(res, true, timeOut)) {
        --nTasks;
        if (res->Result() != 0) {
            ERRLOG("Task failed, &task='%llu', res='%d'", (uint64_t)res.get(), res->Result());
            failure = true;
            this->m_errDesc = res->GetErrDesc();
        }
        if (res->IsZero()) {
            ++m_zeroBlocks;
        }
        ++m_completedBlocks;  // 已处理完的Block数量加一
    }

    return goNext;
}

void VMwareNativeDataPathImpl::CalcJobProgress(mp_int32 &progress, mp_int32 &jobStatus, mp_string &jobDesc)
{
    if (m_iDirtyRangeSize < 0) {
        progress = 0;
    } else if (m_iDirtyRangeSize == 0) {
        progress = COMPLETED_PROGRESS;
    } else {
        progress = m_completedBlocks.load() * COMPLETED_PROGRESS / m_iDirtyRangeSize;
    }

    if (progress == COMPLETED_PROGRESS) {
        // 必须等待CloseRW完成后才设置完成标记
        jobStatus = (m_isClose == true) ? TASK_RUN_SUCCESS : TASK_RUNNING;
        jobDesc = (m_isClose == true) ? "success" : "running";
    } else if (progress != COMPLETED_PROGRESS && !m_isTaskSuccess) {
        jobStatus = TASK_RUN_FAILURE;
        jobDesc = m_errDesc;
    } else {
        jobStatus = TASK_RUNNING;
        jobDesc = "running";
    }

    INFOLOG("Progress of task '%s' is '%d', state is: '%d'", m_volumeInfo.strTaskID.c_str(), progress, jobStatus);
}

void VMwareNativeDataPathImpl::StopThreads()
{
    // set thread exit flag, exit all thread
    m_exitFlag = true;

    // 等待所有使用该对象资源的线程退出
    for (auto &t : m_joinableThrs) {
        INFOLOG("Terminate thread '%llu'", t.get_id());
        t.join();
    }
    m_joinableThrs.clear();
    INFOLOG("Stop thread, task=%s, parent task=%s.", m_strTaskID.c_str(), m_strParentTaskID.c_str());
}

mp_int32 VMwareNativeDataPathImpl::OpenRW()
{
    if (m_reader != nullptr && m_reader->Open() != MP_SUCCESS) {
        return MP_FAILED;
    }
    if (m_writer != nullptr && m_writer->Open() != MP_SUCCESS) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::CloseRW()
{
    if (m_reader != nullptr && m_reader.get() != nullptr) {
        m_reader->Close();
    }
    if (m_writer != nullptr && m_writer.get() != nullptr) {
        m_writer->Close();
    }

    mp_int32 ret = MP_SUCCESS;
    this->m_isClose = true;
    INFOLOG("Close reader and writer successfully.");
    return ret;
}

mp_int32 VMwareNativeDataPathImpl::ParseStorageType(const Json::Value &msgBody)
{
    if (!msgBody.isObject() || !msgBody.isMember(VMWAREDEF::PARAM_STORAGE_STR)) {
        ERRLOG("The request message has no key: '%s', task '%s', parent task '%s'.",
            VMWAREDEF::PARAM_STORAGE_STR.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    // set backend storage type, the default type is nas
    Json::Value storageInfo = msgBody[VMWAREDEF::PARAM_STORAGE_STR];
    if (!storageInfo.isObject() || !storageInfo.isMember(VMWAREDEF::PARAM_STORAGE_PROTOCOL_STR)) {
        COMMLOG(OS_LOG_WARN,
            "The request message has no key: '%s', task '%s', parent task '%s'.",
            VMWAREDEF::PARAM_STORAGE_PROTOCOL_STR.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        m_backendStorageProtocol = VMWAREDEF::VMWARE_STORAGE_PROTOCOL_NAS;
    } else {
        GET_JSON_UINT32(storageInfo, VMWAREDEF::PARAM_STORAGE_PROTOCOL_STR, m_backendStorageProtocol);
        if (m_backendStorageProtocol > SNAP_TYPE_RANGE) {
            ERRLOG("Prepare backup storage Protocol=%d is out of Range", m_backendStorageProtocol);
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::ParseVmSnapshotRef(const Json::Value &msgBody)
{
    if (!msgBody.isObject() || !msgBody.isMember(PARAM_KEY_VM_INFO)) {
        ERRLOG("The request message has no key: '%s', task '%s', parent task '%s'.",
            PARAM_KEY_VM_INFO.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    GET_JSON_STRING(msgBody[PARAM_KEY_VM_INFO], PARAM_KEY_VM_SNAPSHOTREF, m_vmProtectionParams.vmInfo.strSnapshotRef);

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::CheckPathParm(std::string &path)
{
    CHECK_FAIL_EX(CheckPathTraversal(path));
}

void VMwareNativeDataPathImpl::DoRmVmUuidFile(Json::Value &msgBody)
{
    Json::Value body = msgBody[MANAGECMD_KEY_BODY];
    if (body.isObject() && body.isMember(EXT_CMD_PROTECT_VM_UUID) && body[EXT_CMD_PROTECT_VM_UUID].isString()) {
        std::string uuid = "/tmp/vmware-root/";
        uuid += body[EXT_CMD_PROTECT_VM_UUID].asString();
        uuid = uuid + "-" + m_vmProtectionParams.vmInfo.strVmRef;
        if (CheckPathParm(uuid) != MP_SUCCESS) {
            WARNLOG("Invalid file path %s, skip remove", uuid.c_str());
            return;
        }
        INFOLOG("remove file name is %s", uuid.c_str());
        CMpFile::DelDirAndSubFile(uuid.c_str());
    } else {
        COMMLOG(OS_LOG_WARN, "remove uuid file uuid is empty!");
    }
}

mp_int32 VMwareNativeDataPathImpl::InitVddkLib(Json::Value &msgBody, mp_bool &isInited)
{
    DoRmVmUuidFile(msgBody);
    if (isInited) {
        INFOLOG(
            "The VDDK lib has alreadly been initialized in parent task, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_SUCCESS;
    }
    // 1. parse vddk lib path
    mp_string strVddkLibPath;
    if (InitVddkLibPath(msgBody, strVddkLibPath) != MP_SUCCESS) {
        ERRLOG("Unable to init vddk lib path, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    // stoi转换前要判断对应字段是否符合要求
    if (m_vmProtectionParams.pmInfo.strVersion.empty() || m_vmProtectionParams.pmInfo.strVersion[0] < '0' ||
        m_vmProtectionParams.pmInfo.strVersion[0] > '9') {
        ERRLOG("The version is invalid, task '%s', parent task '%s', version: '%s'.",
            m_strTaskID.c_str(), m_strParentTaskID.c_str(), m_vmProtectionParams.pmInfo.strVersion.c_str());
        return MP_FAILED;
    }
    // 2. connect to vcenter using vddk
    VMwareDiskLib::GetInstance()->SetVddkLibPathAndVersion(strVddkLibPath,
        std::stoi(m_vmProtectionParams.pmInfo.strVersion.substr(0, 1)),
        VMWAREDEF::VMWARE_VDDK_MINJOR_VERSION,
        m_vmProtectionParams.pmInfo.strVersion);
    if (!(VMwareDiskLib::GetInstance()->Init())) {
        ERRLOG("Unable to init the VDDK library, task '%s', parent task '%s'.",
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    // 3. perform preparation operation for using vddk
    // cleanup all resource before a backup/recovery
    InnerCleanup();

    // prepare for access in about 3 mins
    std::string errDesc;

    mp_int32 iRet = VMwareDiskLib::GetInstance()->PrepareForAccess(m_vddkConnParams, errDesc);
    if ((iRet != VIX_OK) && (iRet != VIX_E_OPERATION_DISABLED) && (iRet != VIX_E_NOT_SUPPORTED_ON_REMOTE_OBJECT)) {
        ERRLOG("Prepare for access failed: '%s', task '%s', parent task '%s'.",
            errDesc.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
        return MP_FAILED;
    }

    INFOLOG(
        "Prepare for access success, task '%s', parent task '%s'.",
        m_strTaskID.c_str(),
        m_strParentTaskID.c_str());

    return MP_SUCCESS;
}

void VMwareNativeDataPathImpl::InnerCleanup()
{
    std::string errDesc;
    VMWARE_DISK_RET_CODE rc = VMwareDiskLib::GetInstance()->EndAccess(m_vddkConnParams, errDesc);
    if (rc != VIX_OK) {
        WARNLOG("End access failure, errcode '%d', error '%s', task '%s', parent task '%s'!",
            rc,
            errDesc.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    }

    // disconnect with remote product env(specific vm)
    rc = VMwareDiskLib::GetInstance()->Uninit(m_vddkConnParams, errDesc);
    if (rc != VIX_OK) {
        WARNLOG("Uninit failure, errcode '%d', error '%s', task '%s', parent task '%s'!",
            rc,
            errDesc.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    }

    rc = VMwareDiskLib::GetInstance()->Cleanup(m_vddkConnParams, errDesc);
    if (rc != VIX_OK) {
        WARNLOG("Cleanup failure, errcode '%d', error '%s', task '%s', parent task '%s'!",
            rc,
            errDesc.c_str(),
            m_strTaskID.c_str(),
            m_strParentTaskID.c_str());
    }
}

mp_int32 VMwareNativeDataPathImpl::VerifyParamsSize(
    const mp_uint64 &param, const mp_uint64 &size, const mp_string &name)
{
    if (param > size) {
        ERRLOG("%s size=%I64d is out of range %I64d", name.c_str(), param, size);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::CheckVmwareVmInfo(vmware_vm_info& params)
{
    CHECK_FAIL_EX(CheckParamStringEnd(params.strVmName, 0, UTILS_NUM_FOTMAT_STRING_MAX_LEN));
    CHECK_FAIL_EX(CheckParamStringEnd(params.strVmID, 0, UTILS_NUM_FOTMAT_STRING_MAX_LEN));
    CHECK_FAIL_EX(CheckParamStringEnd(params.strVmRef, 0, UTILS_NUM_VMNAME_MAX_LEN));
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::CheckVmwarePmInfo(vmware_pm_info& params)
{
    if (CheckIpAddressValid(params.strIP) == MP_FAILED) {
        return MP_FAILED;
    }
    CHECK_FAIL_EX(CheckParamStringEnd(params.strUserName, 0, UTILS_NUM_FOTMAT_STRING_MAX_LEN));
    CHECK_FAIL_EX(CheckParamStringEnd(params.strPassword, 0, UTILS_NUM_FOTMAT_STRING_MAX_LEN));
    CHECK_FAIL_EX(CheckParamStringEnd(params.strVersion, 0, UTILS_NUM_FOTMAT_STRING_MAX_LEN));
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl::CheckVmwareVolumeInfo(vmware_volume_info& params)
{
    CHECK_FAIL_EX(CheckParamStringEnd(params.strMediumID, 0, UTILS_NUM_FOTMAT_STRING_MAX_LEN));
    CHECK_FAIL_EX(CheckParamStringEnd(params.strDiskID, 0, UTILS_NUM_FOTMAT_STRING_MAX_LEN));
    CHECK_FAIL_EX(CheckParamStringEnd(params.strDiskPath, 0, UTILS_NUM_FOTMAT_STRING_MAX_LEN));
    CHECK_FAIL_EX(CheckParamStringEnd(params.strEagerlyCrub, 0, UTILS_NUM_FOTMAT_STRING_MAX_LEN));

    return MP_SUCCESS;
}