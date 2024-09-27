/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareNativeDataPathProcess.cpp
 * @brief  The implemention about VMwareNativeDataPathProcess
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "dataprocess/datapath/VMwareNativeDataPathProcess.h"
#include <iostream>
#include <algorithm>
#include "common/Types.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/ScopeExit.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "dataprocess/jobqosmanager/JobQosManager.h"
namespace {
const mp_int32 MAX_DATAPATHPROCESSIMPL_INSTANCE_NUM = 40;
constexpr mp_uint32 QOS_FREQUENCY = 1;
}  // namespace

VMwareNativeDataPathProcess::VMwareNativeDataPathProcess(const mp_string &vddkVer) : DataPath(SERVICE_VMWARE, vddkVer)
{
    GenerateCmdMsgHandlerFunMap();
    CMpThread::InitLock(&m_datapathMapLock);
    CMpThread::InitLock(&m_taskMapLock);
    CMpThread::InitLock(&m_handerLock);
    JobQosManager::Init();
    JobQosManager::SetProduceFrequency(QOS_FREQUENCY);
}

VMwareNativeDataPathProcess::~VMwareNativeDataPathProcess()
{
    CMpThread::DestroyLock(&m_datapathMapLock);
    CMpThread::DestroyLock(&m_taskMapLock);
    CMpThread::DestroyLock(&m_handerLock);
    VMwareDiskLib::DestroyInstance();
}

mp_void VMwareNativeDataPathProcess::GenerateCmdMsgHandlerFunMap()
{
    // 初始化命令字与函数的映射关系
    // 注意处理函数如果处理失败也需要返回body体，返回失败会导致dp关闭与agent的网络连接
    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_INIT_VDDKLIB,
        std::bind(&VMwareNativeDataPathProcess::HandleVddkLibInit, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_CLEANUP_VDDKLIB,
        std::bind(&VMwareNativeDataPathProcess::HandleVddkLibCleanup, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_RECOVERY_PREPARATION,
        std::bind(&VMwareNativeDataPathProcess::HandleRecoveryPreparation, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_RESTORE_DATABLOCK,
        std::bind(&VMwareNativeDataPathProcess::HandleDataBlockRestoreRun, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS,
        std::bind(&VMwareNativeDataPathProcess::HandleRestoreProgressQuery, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_FINISH_DATABLOCKRECOVERY,
        std::bind(&VMwareNativeDataPathProcess::HandleDataBlockRestoreFinish, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_FINISH_VMRECOVERY,
        std::bind(&VMwareNativeDataPathProcess::HandleVmRestoreFinish, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_CANCEL_VMRECOVERY,
        std::bind(&VMwareNativeDataPathProcess::HandleVmRestoreCancel, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    GenerateCmdMsgHandlerFunMapBackup();
}

mp_void VMwareNativeDataPathProcess::GenerateCmdMsgHandlerFunMapBackup()
{
    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_BACKUP_PREPARATION,
        std::bind(&VMwareNativeDataPathProcess::HandleBackupPreparation, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(
        std::make_pair(CMD_VMWARENATIVE_BACKUP_OPENDISK, std::bind(&VMwareNativeDataPathProcess::HandleBackupOpenDisk,
        this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(
        std::make_pair(CMD_VMWARENATIVE_BACKUP_CLOSEDISK, std::bind(&VMwareNativeDataPathProcess::HandleBackupCloseDisk,
        this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_PREPARE_TARGETLUN,
        std::bind(&VMwareNativeDataPathProcess::HandleTargetLunPrepare, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_BACKUP_DATABLOCK,
        std::bind(&VMwareNativeDataPathProcess::HandleDataBlockBackupRun, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS,
        std::bind(&VMwareNativeDataPathProcess::HandleBackupProgressQuery, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_FINISH_DATABLOCKBACKUP,
        std::bind(&VMwareNativeDataPathProcess::HandleDataBlockBackupFinish, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_FINISH_VMBACKUP,
        std::bind(&VMwareNativeDataPathProcess::HandleVmBackupFinish, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_CANCEL_VMBACKUP,
        std::bind(&VMwareNativeDataPathProcess::HandleVmBackupCancel, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3)));

    m_mapCmdMsgHandlerFun.insert(std::make_pair(CMD_VMWARENATIVE_BACKUP_AFS,
        std::bind(&VMwareNativeDataPathProcess::HandleDisksAfsBitmap,
        this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}


mp_int32 VMwareNativeDataPathProcess::VMwareNativeDpCmdParse(CDppMessage &message,
    mp_string &currentTaskID, mp_string &parentTaskID)
{
    mp_uint32 cmd = message.GetManageCmd();
    mp_uint64 seqNo = message.GetOrgSeqNo();
    INFOLOG("Invoke dp function, cmd=0x%x, seq=%llu.", cmd, seqNo);
    auto iter = m_mapCmdMsgHandlerFun.find(cmd);
    if (iter == m_mapCmdMsgHandlerFun.end()) {
        ERRLOG("Unable to find dp function, cmd=0x%x, taskid=%s, seq=%llu.", cmd, currentTaskID.c_str(), seqNo);
        return MP_FAILED;
    } else {
        std::shared_ptr<CDppMessage> messagePtr = std::make_shared<CDppMessage>(message);
        if (cmd == CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS || cmd == CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS) {
            mp_int32 iRet = iter->second(messagePtr, currentTaskID, parentTaskID);
            INFOLOG("Invoke progress function, cmd=0x%x, taskid=%s, seq=%llu, iRet=%d.",
                cmd, currentTaskID.c_str(), seqNo, iRet);
            return iRet;
        }

        CThreadAutoLock threadLock(&m_handerLock);
        auto iterThread = m_mapVMwareThreadHander.find(parentTaskID);
        if (iterThread == m_mapVMwareThreadHander.end()) {
            ERRLOG("Fatal error Unable to find hander thread, cmd=0x%x, taskid=%s, seq=%llu.",
                cmd, currentTaskID.c_str(), seqNo);
            return MP_FAILED;
        } else {
            auto handerThread = iterThread->second;
            if (handerThread == nullptr || messagePtr.get() == nullptr) {
                ERRLOG("Fatal error current handerThread is nullptr");
                return MP_FAILED;
            }
            handerThread->InsertTask(std::bind(iter->second, messagePtr, currentTaskID, parentTaskID));
        }
    }

    INFOLOG("Invoke dp function succ, cmd=0x%x, taskid=%s, seq=%llu.", cmd, currentTaskID.c_str(), seqNo);
    return MP_SUCCESS;
}

// logic to deal with all DPP format requests from Agent
mp_int32 VMwareNativeDataPathProcess::ExtCmdProcess(CDppMessage &message)
{
    Json::Value bodyMsg;
    mp_int32 iRet = message.GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get manage body failed, ret=%d, seq=%llu.", iRet, message.GetOrgSeqNo());
        return iRet;
    }
    // parse task/parent task id and create taskid <=> current class map
    if (!bodyMsg.isObject() || !bodyMsg.isMember(MANAGECMD_KEY_BODY)) {
        ERRLOG("Reveived message has no body attr.");
        return MP_FAILED;
    }

    INFOLOG("Handle cmd=%u, seq=%llu.", message.GetManageCmd(), message.GetOrgSeqNo());
    mp_string currentTaskID;
    GET_JSON_STRING(bodyMsg[MANAGECMD_KEY_BODY], PARAM_KEY_TASKID, currentTaskID);
    if (currentTaskID.empty()) {
        ERRLOG("Task id must be provided!");
        return MP_FAILED;
    }

    mp_string parentTaskID;
    GET_JSON_STRING(bodyMsg[MANAGECMD_KEY_BODY], PARAM_KEY_PARENT_TASKID, parentTaskID);
    if (parentTaskID.empty()) {
        ERRLOG("Parent task id must be provided!");
        return MP_FAILED;
    }
    DBGLOG("currentTaskID: %s and strParentTaskID: %s", currentTaskID.c_str(), parentTaskID.c_str());

    iRet = GenerateMap(message, currentTaskID, parentTaskID);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    iRet = VMwareNativeDpCmdParse(message, currentTaskID, parentTaskID);
    if (iRet != MP_SUCCESS) {
        ERRLOG("VMwareNativeDpCmdParse exec fail, task id '%s', parent taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(),
            parentTaskID.c_str(),
            iRet,
            message.GetManageCmd(),
            message.GetOrgSeqNo());
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathProcess::GenerateMap(CDppMessage &message,
    mp_string &currentTaskID, mp_string &parentTaskID)
{
    {   // clean 之前任务的线程
        CThreadAutoLock threadLock(&m_handerLock);
        while (!m_prepareCleanThread.empty()) {
            auto parentId = m_prepareCleanThread.back();
            m_prepareCleanThread.pop_back();
            RemoveVMwareThreadHander(parentId);
        }
    }
    mp_int32 iRet = GenerateVMwareDpImplInstanceMap(currentTaskID, parentTaskID);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to init a VMwareNativeDataPathImpl, taskid=%s, parent taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(),
            parentTaskID.c_str(),
            iRet,
            message.GetManageCmd(),
            message.GetOrgSeqNo());
        return iRet;
    }

    iRet = GenerateVMwareThreadHanderMap(currentTaskID, parentTaskID);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to init a VMwareThreadHandermpl, taskid=%s, parent taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(),
            parentTaskID.c_str(),
            iRet,
            message.GetManageCmd(),
            message.GetOrgSeqNo());
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathProcess::GenerateVMwareDpImplInstanceMap(mp_string &currentTaskID, mp_string &parentTaskID)
{
    CThreadAutoLock threadLock(&m_datapathMapLock);
    mp_int32 ret = MP_SUCCESS;
    std::map<mp_string, std::shared_ptr<VMwareNativeDataPathImpl>>::iterator iter = m_mapVMwareDpImplInstance.find(
        currentTaskID);
    if (iter == m_mapVMwareDpImplInstance.end()) {
        INFOLOG("VMwareNativeDataPathImpl instance of task=%s[parent=%s] does not exist, will create new.",
            currentTaskID.c_str(),
            parentTaskID.c_str());

        std::shared_ptr<VMwareNativeDataPathImpl> spVmwareInstance = std::make_shared<VMwareNativeDataPathImpl>();
        m_mapVMwareDpImplInstance.insert(std::make_pair(currentTaskID, spVmwareInstance));
    } else {
        INFOLOG("VMwareNativeDataPathImpl intance of task=%s[parent=%s] is running!",
            currentTaskID.c_str(),
            parentTaskID.c_str());
    }
    return ret;
}

mp_int32 VMwareNativeDataPathProcess::GenerateVMwareThreadHanderMap(mp_string &currentTaskID, mp_string &parentTaskID)
{
    // parse task/parent task id and create parent_taskid <=> handerthread map
    CThreadAutoLock threadLock(&m_handerLock);
    mp_int32 ret = MP_SUCCESS;
    std::map<mp_string, std::shared_ptr<HanderThread>>::iterator iter = m_mapVMwareThreadHander.find(parentTaskID);
    if (iter == m_mapVMwareThreadHander.end()) {
        INFOLOG("VMwareThreadHander instance of task=%s[parent=%s] does not exist, will create new.",
            currentTaskID.c_str(),
            parentTaskID.c_str());

        std::shared_ptr<HanderThread> handerThread = std::make_shared<HanderThread>();
        handerThread->StartThread();
        m_mapVMwareThreadHander.insert(std::make_pair(parentTaskID, handerThread));
    } else {
        INFOLOG("VMwareThreadHander intance of task=%s[parent=%s] is running!",
            currentTaskID.c_str(),
            parentTaskID.c_str());
    }
    return ret;
}

mp_void VMwareNativeDataPathProcess::RemoveDataPathImplInstance(const mp_string &taskId)
{
    CThreadAutoLock threadLock(&m_datapathMapLock);
    std::map<mp_string, std::shared_ptr<VMwareNativeDataPathImpl>>::iterator iter = m_mapVMwareDpImplInstance.find(
        taskId);
    if (iter == m_mapVMwareDpImplInstance.end()) {
        COMMLOG(OS_LOG_WARN,
            "Instance of class 'VMwareNativeDataPathImpl' for task '%s' does not exist, \
            errors occurs somewhere, please check !",
            taskId.c_str());
    } else {
        COMMLOG(OS_LOG_INFO,
            "Task '%s' has been finished, will remove instance of class 'VMwareNativeDataPathImpl'.",
            taskId.c_str());
        m_mapVMwareDpImplInstance.erase(taskId);
    }
}

mp_void VMwareNativeDataPathProcess::ReleaseTaskPoolResource(const mp_string &taskId)
{
    CThreadAutoLock threadLock(&m_datapathMapLock);
    std::map<mp_string, std::shared_ptr<VMwareNativeDataPathImpl>>::iterator iter = m_mapVMwareDpImplInstance.find(
        taskId);
    if (iter == m_mapVMwareDpImplInstance.end()) {
        WARNLOG("Instance of class 'VMwareNativeDataPathImpl' for task '%s' does not exist, \
            errors occurs somewhere, please check !",
            taskId.c_str());
    } else {
        INFOLOG("Task '%s' will be finished, will release taskpool resource \
            for instance of class 'VMwareNativeDataPathImpl'.",
            taskId.c_str());
        if (iter->second == NULL) {
            return;
        }
        iter->second->StopThreads();  // 销毁VMwareNativeDataPathImpl对象前先终止使用该对象资源的线程
    }
}

mp_void VMwareNativeDataPathProcess::RemoveTaskMapItem(const mp_string &taskId)
{
    CThreadAutoLock threadLock(&m_taskMapLock);
    std::unordered_map<mp_string, std::vector<mp_string>>::iterator iter = m_mapTaskInfo.find(taskId);
    if (iter != m_mapTaskInfo.end()) {
        INFOLOG("The vm level task '%s' has been finished, will remove it from task map.", taskId.c_str());
        m_mapTaskInfo.erase(taskId);
    }
    ReleaseVMwareThreadHander(taskId);
}

mp_void VMwareNativeDataPathProcess::RemoveVMwareThreadHander(const mp_string &parentTaskID)
{
    std::map<mp_string, std::shared_ptr<HanderThread>>::iterator iter = m_mapVMwareThreadHander.find(
        parentTaskID);
    if (iter == m_mapVMwareThreadHander.end()) {
        COMMLOG(OS_LOG_WARN,
            "Instance of class 'VMwareThreadHander' for vm level task '%s' does not exist, \
            errors occurs somewhere, please check!",
            parentTaskID.c_str());
    } else {
        COMMLOG(OS_LOG_INFO,
            "The vm level task '%s' has been finished, will remove instance of class 'VMwareThreadHander'.",
            parentTaskID.c_str());
        if (iter->second == nullptr) {
            return;
        }
        m_mapVMwareThreadHander.erase(iter);
    }
}

mp_void VMwareNativeDataPathProcess::ReleaseVMwareThreadHander(const mp_string &parentTaskID)
{
    CThreadAutoLock threadLock(&m_handerLock);
    std::map<mp_string, std::shared_ptr<HanderThread>>::iterator iter = m_mapVMwareThreadHander.find(
        parentTaskID);
    if (iter == m_mapVMwareThreadHander.end()) {
        COMMLOG(OS_LOG_WARN,
            "Instance of class 'VMwareThreadHander' for vm level task '%s' does not exist, \
            errors occurs somewhere, please check!",
            parentTaskID.c_str());
    } else {
        COMMLOG(OS_LOG_INFO,
            "The vm levlel task '%s' has been finished, will release instance of class 'VMwareThreadHander'.",
            parentTaskID.c_str());
        if (iter->second == nullptr) {
            return;
        }
        m_prepareCleanThread.push_back(parentTaskID);
    }
}

// check whether the parent task exist or not
mp_bool VMwareNativeDataPathProcess::QueryTaskMapItem(const mp_string &taskId)
{
    CThreadAutoLock threadLock(&m_taskMapLock);
    std::unordered_map<mp_string, std::vector<mp_string>>::iterator iter = m_mapTaskInfo.find(taskId);
    if (iter == m_mapTaskInfo.end()) {
        ERRLOG("The vm level task '%s' does not exist, maybe error occurs, please check!", taskId.c_str());
        return false;
    }
    return true;
}

mp_void VMwareNativeDataPathProcess::AddTaskMapItem(const mp_string &pTaskId, const mp_string &sTaskId)
{
    CThreadAutoLock threadLock(&m_taskMapLock);
    std::vector<mp_string> sTaskList;
    std::unordered_map<mp_string, std::vector<mp_string>>::iterator iter = m_mapTaskInfo.find(pTaskId);
    if (iter == m_mapTaskInfo.end()) {
        sTaskList.push_back(sTaskId);
        m_mapTaskInfo.insert(std::pair<mp_string, std::vector<mp_string>>(pTaskId, sTaskList));
    } else {
        m_mapTaskInfo[pTaskId].push_back(sTaskId);
    }
}

mp_int32 VMwareNativeDataPathProcess::HandleDataBlockVerify(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message.get() == nullptr) {
        COMMLOG(OS_LOG_ERROR, "message is nullptr");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathProcess::DoVMwareNativeFunc(std::shared_ptr<VMwareNativeDataPathImpl> &mapVMwareDpImpl,
    const mp_string &currentTaskID)
{
    CThreadAutoLock threadLock(&m_datapathMapLock);
    std::map<mp_string, std::shared_ptr<VMwareNativeDataPathImpl>>::iterator iter = m_mapVMwareDpImplInstance.find(
        currentTaskID);
    if (iter == m_mapVMwareDpImplInstance.end()) {
        ERRLOG("can't find the instace of VMwareNativeDataPathImpl taskId=%s", currentTaskID.c_str());
        return MP_FAILED;
    }
    mapVMwareDpImpl = m_mapVMwareDpImplInstance[currentTaskID];
    if (mapVMwareDpImpl.get() == nullptr) {
        ERRLOG("the instace of VMwareNativeDataPathImpl is nullptr taskId=%s", currentTaskID.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

// init VDDK lib - vm level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleVddkLibInit(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    mp_bool isVddkInited = false;
    // if map key exists, it means the vm level vddk init has been performed
    if (QueryTaskMapItem(parentTaskID)) {
        isVddkInited = true;
    }
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleVddkLibInit, cmd=0x%x, seq=%llu, taskID=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_string errDetail;
    mp_int32 iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manage body failed, taskid=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()), EXT_CMD_VMWARENATIVE_INIT_VDDKLIB_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    // do vddk init
    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareNativeVddkInit(bodyMsg, isVddkInited);
    }
    if (iRet != MP_SUCCESS) {
        errDetail = "Unable to init vmware native VDDK library, taskid=" + currentTaskID;
        ERRLOG("Unable to init vmware native VDDK library, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()), EXT_CMD_VMWARENATIVE_INIT_VDDKLIB_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    // add map key
    if (!isVddkInited) {
        AddTaskMapItem(parentTaskID, currentTaskID);
    }

    INFOLOG("Init VDDK lib successfully, taskid=%s, cmd=0x%x, seq=%llu.",
        currentTaskID.c_str(),
        message->GetManageCmd(),
        message->GetOrgSeqNo());
    // DPP response with no body to host agent
    errDetail.clear();
    SendNoBodyResponse(*(message.get()), EXT_CMD_VMWARENATIVE_INIT_VDDKLIB_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// uinit VDDK lib - vm level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleVddkLibCleanup(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleVddkLibCleanup, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());

    Json::Value bodyMsg;
    mp_string errDetail = "Unable to cleanup vmware native VDDK library, taskid=" + currentTaskID;
    mp_int32 iRet = MP_FAILED;

    SCOPE_EXIT
    {
        // remove instance of class 'VMwareNativeDataPathImpl'
        RemoveDataPathImplInstance(currentTaskID);
        // clear vm level task info and collect vm level thread to be released
        RemoveTaskMapItem(parentTaskID);
    };

    // release all disk tasks' thread resource
    ReleaseDiskThreadResource(parentTaskID);
    // parse manage body
    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(),
            message->GetManageCmd(),
            message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_CLEANUP_VDDKLIB_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    // invoke method of class 'VMwareNativeDataPathImpl'
    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareNativeVddkCleanup(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to cleanup vmware native VDDK library, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(),
            iRet,
            message->GetManageCmd(),
            message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_CLEANUP_VDDKLIB_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    } else {
        errDetail.clear();
        INFOLOG("Cleanup VDDK lib successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(),
            message->GetManageCmd(),
            message->GetOrgSeqNo());
    }

    SendNoBodyResponse(*(message.get()), EXT_CMD_VMWARENATIVE_CLEANUP_VDDKLIB_ACK, iRet, errDetail, currentTaskID);

    return MP_SUCCESS;
}

// backup preparation - disk level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleBackupPreparation(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleBackupPreparation, cmd=0x%x, seq=%llu taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to prepare vmware backup, taskid=" + currentTaskID;

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_BACKUP_PREPARATION_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manage body failed, taskid=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(),
            message->GetManageCmd(),
            message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_BACKUP_PREPARATION_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    // parse vm info
    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareNativePreparation(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        errDetail = "Unable to prepare vmware backup, taskid=" + currentTaskID;
        ERRLOG("Unable to prepare vmware backup, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(),
            iRet,
            message->GetManageCmd(),
            message->GetOrgSeqNo());
    } else {
        errDetail.clear();
        INFOLOG("Prepare vmware native backup successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(),
            message->GetManageCmd(),
            message->GetOrgSeqNo());
    }

    // DPP response with no body to host agent
    SendNoBodyResponse(*(message.get()), EXT_CMD_VMWARENATIVE_BACKUP_PREPARATION_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleBackupOpenDisk(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleBackupOpenDisk, cmd=0x%x, seq=%llu taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to open disk vmware backup, taskid=" + currentTaskID;

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_BACKUP_OPENDISK_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manage body failed, taskid=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(),
            message->GetManageCmd(),
            message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_BACKUP_OPENDISK_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    // parse vm info
    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareNativeBackupOpenDisk(bodyMsg);
    }

    if (iRet != MP_SUCCESS) {
        errDetail = "Unable to opendisk vmware backup, taskid=" + currentTaskID;
        ERRLOG("Unable to opendisk vmware backup, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
    } else {
        errDetail.clear();
        INFOLOG("opendisk vmware native backup successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendCommonDppResponse(*(message.get()), bodyMsg["body"],
        EXT_CMD_VMWARENATIVE_BACKUP_OPENDISK_ACK, iRet, errDetail, currentTaskID);
    return iRet;
}

EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleDisksAfsBitmap(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleDisksAfsBitmap, cmd=0x%x, seq=%llu taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to get disks afs bitmap vmware backup, taskid=" + currentTaskID;

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_BACKUP_AFS_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manage body failed, taskid=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_BACKUP_AFS_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareNativeBackupAfsBitmap(bodyMsg, errDetail);
    }

    if (iRet != MP_SUCCESS) {
        if (errDetail.empty()) {
            errDetail = "Unable to get disks afs bitmap vmware backup";
        }
        ERRLOG("Unable to get disks afs bitmap vmware backup, taskid=%s, ret=%d, cmd=0x%x, seq=%llu, err:%s.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo(),
            errDetail.c_str());
    } else {
        errDetail.clear();
        INFOLOG("Get disks afs bitmap vmware native backup successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendCommonDppResponse(*(message.get()), bodyMsg,
        EXT_CMD_VMWARENATIVE_BACKUP_AFS_ACK, iRet, errDetail, currentTaskID);

    RemoveDataPathImplInstance(currentTaskID);
    return iRet;
}

EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleBackupCloseDisk(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleBackupCloseDisk, cmd=0x%x, seq=%llu taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to close disk vmware backup, taskId=" + currentTaskID;

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()), EXT_CMD_VMWARENATIVE_BACKUP_CLOSEDISK_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manage body failed, taskId=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskId=%s, cmd=0x%x, seq=%llu.", currentTaskID.c_str(), message->GetManageCmd(),
            message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()), EXT_CMD_VMWARENATIVE_BACKUP_CLOSEDISK_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }
    // parse vm info
    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareNativeBackupCloseDisk();
    }
    if (iRet != MP_SUCCESS) {
        errDetail = "Unable to close disk vmware backup, taskId=" + currentTaskID;
        ERRLOG("Unable to close disk vmware backup, taskId=%s, ret=%d, cmd=0x%x, seq=%llu.", currentTaskID.c_str(),
            iRet, message->GetManageCmd(), message->GetOrgSeqNo());
    }
    errDetail.clear();
    INFOLOG("Close disk vmware native backup successfully, taskId=%s, cmd=0x%x, seq=%llu.", currentTaskID.c_str(),
        message->GetManageCmd(), message->GetOrgSeqNo());
    SendCommonDppResponse(*(message.get()), bodyMsg["body"],
        EXT_CMD_VMWARENATIVE_BACKUP_CLOSEDISK_ACK, iRet, errDetail, currentTaskID);
    return iRet;
}

// recovery preparation - disk level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleRecoveryPreparation(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleRecoveryPreparation, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to prepare vmware native recovery, taskid=" + currentTaskID;

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_RECOVERY_PREPARATION_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manage body failed, taskid=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_RECOVERY_PREPARATION_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    // parse vm info
    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareNativePreparation(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        errDetail = "Unable to prepare vmware native recovery, taskid=" + currentTaskID;
        ERRLOG("Unable to prepare vmware native recovery, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
    } else {
        errDetail.clear();
        INFOLOG("Prepare vmware native recovery successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }
    // DPP response with no body to host agent
    SendNoBodyResponse(*(message.get()),
        EXT_CMD_VMWARENATIVE_RECOVERY_PREPARATION_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// lun map - disk level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleTargetLunPrepare(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleTargetLunPrepare, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to obtain target data lun detail, taskid=" + currentTaskID;

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_PREPARE_BACKEND_STORAGE_RESOURCE_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manage body failed, taskid=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_PREPARE_BACKEND_STORAGE_RESOURCE_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->TargetLunPrepare(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        errDetail = "Unable to obtain target data lun detail, taskid=" + currentTaskID;
        ERRLOG("Unable to obtain target data lun detail, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
    } else {
        errDetail.clear();
        INFOLOG("Obtain vmware vm disk's transport mode successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendNoBodyResponse(*(message.get()),
        EXT_CMD_VMWARENATIVE_PREPARE_BACKEND_STORAGE_RESOURCE_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// backup data block - disk level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleDataBlockBackupRun(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleDataBlockBackupRun, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Errors occur when backuping up vmware vm disk data block, taskid=" + currentTaskID;

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_RUN_BACKUP_DATABLOCK_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    AddTaskMapItem(parentTaskID, currentTaskID);

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manage body failed, taskid=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_RUN_BACKUP_DATABLOCK_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareNativeDataBlockBackup(bodyMsg, errDetail);
    }
    if (iRet != MP_SUCCESS) {
        if (errDetail.empty()) {
            errDetail = "Errors occur when backuping up vmware vm disk data block, taskid=" + currentTaskID;
        }
        COMMLOG(OS_LOG_ERROR,
            "Errors occur when backuping up vmware vm disk data block, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
    } else {
        errDetail.clear();
        COMMLOG(OS_LOG_INFO,
            "Start threads to backup vmware vm disk data blocks successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendNoBodyResponse(*(message.get()),
        EXT_CMD_VMWARENATIVE_RUN_BACKUP_DATABLOCK_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// cancel backup - vm level task(not use currently)
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleVmBackupCancel(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleVmBackupCancel, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to cancel vmware vm backup task, taskid=" + currentTaskID;

    SCOPE_EXIT
    {
        // remove instance of class 'VMwareNativeDataPathImpl'
        RemoveDataPathImplInstance(currentTaskID);
        // clear vm level task info and collect vm level thread to be released
        RemoveTaskMapItem(parentTaskID);
        SendNoBodyResponse(
            *(message.get()), EXT_CMD_VMWARENATIVE_CANCEL_VMBACKUP_TASK_ACK, iRet, errDetail, currentTaskID);
    };

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        iRet = MP_FAILED;
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Manager body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareVmBackupCancel(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Unable to cancel vmware vm backup task, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    } else {
        errDetail.clear();
        COMMLOG(OS_LOG_INFO,
            "Cancel vmware vm backup successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendCommonDppResponse(*(message.get()), bodyMsg,
        EXT_CMD_VMWARENATIVE_CANCEL_VMBACKUP_TASK_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// query backup progress - disk level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleBackupProgressQuery(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleBackupProgressQuery, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to obtain vmware vm disk data block backup progress, taskid=" + currentTaskID;

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manage body failed, taskid=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->BackupProgressQuery(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        errDetail = "Unable to obtain vmware vm disk data block backup progress, taskid=" + currentTaskID;
        ERRLOG("Unable to obtain vmware vm disk data block backup progress, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
    } else {
        errDetail.clear();
        INFOLOG("Obtain vmware vm disk data block backup progress successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendCommonDppResponse(*(message.get()), bodyMsg,
        EXT_CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// finish data block backup - disk level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleDataBlockBackupFinish(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleDataBlockBackupFinish, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to finish disk data block backup, taskid=" + currentTaskID;

    SCOPE_EXIT
    {
        // remove one disk task's thread resource
        ReleaseTaskPoolResource(currentTaskID);
        // remove class instance
        RemoveDataPathImplInstance(currentTaskID);

        SendNoBodyResponse(
            *(message.get()), EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKBACKUP_ACK, iRet, errDetail, currentTaskID);
    };

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        iRet = MP_FAILED;
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->DataBlockBackupFinish(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Unable to finish disk data block backup, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    } else {
        errDetail.clear();
        COMMLOG(OS_LOG_INFO,
            "Finish vmware vm disk data block backup successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendCommonDppResponse(*(message.get()), bodyMsg,
        EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKBACKUP_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// finish backup - vm level task(not use currently)
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleVmBackupFinish(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleVmBackupFinish, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to finish vmware vm backup, taskid=" + currentTaskID;

    SCOPE_EXIT
    {
        // remove instance of class 'VMwareNativeDataPathImpl'
        RemoveDataPathImplInstance(currentTaskID);
        // clear vm level task info and collect vm level thread to be released
        RemoveTaskMapItem(parentTaskID);
        SendNoBodyResponse(
            *(message.get()), EXT_CMD_VMWARENATIVE_FINISH_VMBACKUP_TASK_ACK, iRet, errDetail, currentTaskID);
    };

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        iRet = MP_FAILED;
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareVmBackupFinish(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Unable to finish vmware vm backup, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    } else {
        errDetail.clear();
        COMMLOG(OS_LOG_INFO,
            "Finish vmware vm backup successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendCommonDppResponse(*(message.get()),
        bodyMsg, EXT_CMD_VMWARENATIVE_FINISH_VMBACKUP_TASK_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// run data block recovery - disk level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleDataBlockRestoreRun(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleDataBlockRestoreRun, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to cancel vmware vm restore task: taskid=" + currentTaskID;

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_RUN_RECOVERY_DATABLOCK_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    AddTaskMapItem(parentTaskID, currentTaskID);

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manager body failed: taskid=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_RUN_RECOVERY_DATABLOCK_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareNativeDataBlockRestore(bodyMsg, errDetail);
    }
    if (iRet != MP_SUCCESS) {
        if (errDetail.empty()) {
            errDetail = "Errors occur when performing vm disk data block recovery, taskid=" + currentTaskID;
        }
        COMMLOG(OS_LOG_ERROR,
            "Errors occur when performing vm disk data block recovery, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
    } else {
        errDetail.clear();
        COMMLOG(OS_LOG_INFO,
            "Start threads to restore vmware vm disk data blocks successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendNoBodyResponse(*(message.get()),
        EXT_CMD_VMWARENATIVE_RUN_RECOVERY_DATABLOCK_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// cancel vm recovery - vm level task(not use currently)
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleVmRestoreCancel(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleVmRestoreCancel, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to cancel vmware vm restore task: taskid=" + currentTaskID;

    SCOPE_EXIT
    {
        // remove instance of class 'VMwareNativeDataPathImpl'
        RemoveDataPathImplInstance(currentTaskID);
        // clear vm level task info and collect vm level thread to be released
        RemoveTaskMapItem(parentTaskID);
        SendNoBodyResponse(
            *(message.get()), EXT_CMD_VMWARENATIVE_CANCEL_VMRECOVERY_TASK_ACK, iRet, errDetail, currentTaskID);
    };

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        iRet = MP_FAILED;
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Manager body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareVmRestoreCancel(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to cancel vmware vm restore task, taskid=%s, ret=%d.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    } else {
        errDetail.clear();
        INFOLOG("Cancel vmware vm recovery successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendCommonDppResponse(*(message.get()), bodyMsg,
        EXT_CMD_VMWARENATIVE_CANCEL_VMRECOVERY_TASK_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// query recovery progress - disk level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleRestoreProgressQuery(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleRestoreProgressQuery, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to obtain disk data block restore progress: taskid=" + currentTaskID;

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        errDetail = "Get Manage body failure: taskid=" + currentTaskID;
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        SendNoBodyResponse(*(message.get()),
            EXT_CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS_ACK, iRet, errDetail, currentTaskID);
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->RestoreProgressQuery(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        errDetail = "Unable to obtain disk data block restore progress: taskid=" + currentTaskID;
        ERRLOG("Unable to obtain disk data block restore progress, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
    } else {
        errDetail.clear();
        INFOLOG("Obtain vmware vm disk data block restore progress successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendCommonDppResponse(*(message.get()), bodyMsg,
        EXT_CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// finish data block recovery - disk level task
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleDataBlockRestoreFinish(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleDataBlockRestoreFinish, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to finish disk data block restore: taskid=" + currentTaskID;

    SCOPE_EXIT
    {
        // remove one disk task's thread resource
        ReleaseTaskPoolResource(currentTaskID);
        // remove class instance
        RemoveDataPathImplInstance(currentTaskID);
        SendNoBodyResponse(
            *(message.get()), EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKRECOVERY_ACK, iRet, errDetail, currentTaskID);
    };

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        iRet = MP_FAILED;
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->DataBlockRestoreFinish(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to finish disk data block restore, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    } else {
        errDetail.clear();
        INFOLOG("Finish vmware vm disk data block recovery successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendCommonDppResponse(*(message.get()), bodyMsg,
        EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKRECOVERY_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

// finish vm recovery - vm level task(not use currently)
EXTER_ATTACK mp_int32 VMwareNativeDataPathProcess::HandleVmRestoreFinish(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID)
{
    if (message == nullptr) {
        ERRLOG("DPP message is nullptr.");
        return MP_FAILED;
    }
    INFOLOG("HandleVmRestoreFinish, cmd=0x%x, seq=%llu, taskId=%s",
        message->GetManageCmd(), message->GetOrgSeqNo(), currentTaskID.c_str());
    Json::Value bodyMsg;
    mp_int32 iRet = MP_FAILED;
    mp_string errDetail = "Unable to finish vmware vm recovery, taskid=" + currentTaskID;

    SCOPE_EXIT
    {
        // remove instance of class 'VMwareNativeDataPathImpl'
        RemoveDataPathImplInstance(currentTaskID);
        // clear vm level task info and collect vm level thread to be released
        RemoveTaskMapItem(parentTaskID);
        SendNoBodyResponse(
            *(message.get()), EXT_CMD_VMWARENATIVE_FINISH_VMRECOVERY_TASK_ACK, iRet, errDetail, currentTaskID);
    };

    // check parent task exist or not
    if (!QueryTaskMapItem(parentTaskID)) {
        iRet = MP_FAILED;
        ERRLOG("Due to parent task '%s' does not exist, will stop current task '%s', cmd=0x%x, seq=%llu.",
            parentTaskID.c_str(), currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    iRet = message->GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Manage body failed, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    }

    std::shared_ptr<VMwareNativeDataPathImpl> mapVMwareDpImpl = nullptr;
    iRet = DoVMwareNativeFunc(mapVMwareDpImpl, currentTaskID);
    if (iRet == MP_SUCCESS) {
        iRet = mapVMwareDpImpl->VMwareVmRestoreFinish(bodyMsg);
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Unable to finish vmware vm recovery, taskid=%s, ret=%d, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), iRet, message->GetManageCmd(), message->GetOrgSeqNo());
        return iRet;
    } else {
        errDetail.clear();
        COMMLOG(OS_LOG_INFO,
            "Finish vmware vm recovery successfully, taskid=%s, cmd=0x%x, seq=%llu.",
            currentTaskID.c_str(), message->GetManageCmd(), message->GetOrgSeqNo());
    }

    SendCommonDppResponse(*(message.get()),
        bodyMsg, EXT_CMD_VMWARENATIVE_FINISH_VMRECOVERY_TASK_ACK, iRet, errDetail, currentTaskID);
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathProcess::HandlePause(Json::Value &bodyMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    INFOLOG("Received Message: EXT_CMD_PAUSE\n");
    return iRet;
}

mp_int32 VMwareNativeDataPathProcess::HandleResume(Json::Value &bodyMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    INFOLOG("Received Message: EXT_CMD_RESUME\n");
    return iRet;
}

mp_void VMwareNativeDataPathProcess::HandleClose()
{
    mp_int32 cur_pid;
    INFOLOG("Received Message: EXT_CMD_CLOSE\n");
    cur_pid = getpid();
    INFOLOG("We are going to close current dataprocess with pid %d\n", cur_pid);

    sendExitFlag = MP_TRUE;
    exit(0);
}

mp_void VMwareNativeDataPathProcess::SendProductEnvConnDppResponse(
    CDppMessage &reqmsg, std::map<mp_string, vmware_pe_info> &status)
{
    std::unique_ptr<CDppMessage> msg(std::make_unique<CDppMessage>());
    msg.get()->CloneMsg(reqmsg);

    Json::Value vecProductEnv;
    Json::Value productEnv;
    std::map<mp_string, vmware_pe_info>::iterator iter;
    for (iter = status.begin(); iter != status.end(); ++iter) {
        productEnv[PARAM_KEY_PRODUCTMANAGER_PROTOCOL] =
            static_cast<Json::UInt64>(iter->second.productManager.ulProtocol);
        productEnv[PARAM_KEY_PRODUCTMANAGER_IP] = iter->second.productManager.strIP;
        productEnv[PARAM_KEY_PRODUCTMANAGER_PORT] = static_cast<Json::UInt64>(iter->second.productManager.ulPort);
        productEnv[PARAM_KEY_PRODUCTMANAGER_STATUS] = static_cast<Json::UInt64>(iter->second.productManager.ulStatus);
        productEnv[PARAM_KEY_AGENT_HOST_IP] = iter->second.strHostAgentIP;
        vecProductEnv.append(std::move(productEnv));
    }

    Json::Value jsRespBody;
    jsRespBody[MANAGECMD_KEY_BODY] = std::move(vecProductEnv);
    jsRespBody[MANAGECMD_KEY_CMDNO] = EXT_CMD_VMWARENATIVE_CHECK_PRODUCTENV_CONNECTION_ACK;
    msg.get()->SetMsgBody(jsRespBody);

    PushMsg2Queue(msg);
}

mp_void VMwareNativeDataPathProcess::SendNoBodyResponse(
    CDppMessage &reqmsg, mp_int32 cmd, mp_int32 errorcode, mp_string &errorDetail, const mp_string &currentTaskID)
{
    std::unique_ptr<CDppMessage> msg(std::make_unique<CDppMessage>());
    msg.get()->CloneMsg(reqmsg);

    Json::Value jsRespBody;
    // pass inner error info to rdagent process
    Json::Value rspBody;
    rspBody[MANAGECMD_KEY_TASKID] = currentTaskID;
    rspBody[MANAGECMD_KEY_ERRORCODE] = errorcode;
    rspBody[MANAGECMD_KEY_ERRORDETAIL] = errorDetail;
    jsRespBody[MANAGECMD_KEY_BODY] = std::move(rspBody);
    jsRespBody[MANAGECMD_KEY_CMDNO] = cmd;
    jsRespBody[MANAGECMD_KEY_ERRORCODE] = errorcode;
    jsRespBody[MANAGECMD_KEY_ERRORDETAIL] = errorDetail;
    msg.get()->SetMsgBody(jsRespBody);

    PushMsg2Queue(msg);
}

mp_void VMwareNativeDataPathProcess::SendCommonDppResponse(
    CDppMessage &reqmsg, Json::Value &paramBody, mp_int32 cmd,
    mp_int32 &errorcode, mp_string &errorDetail, const mp_string &currentTaskID)
{
    std::unique_ptr<CDppMessage> msg(std::make_unique<CDppMessage>());
    msg.get()->CloneMsg(reqmsg);

    Json::Value jsRespBody;
    if (!paramBody.isObject() || !paramBody.isMember(MANAGECMD_KEY_TASKID)) {
        paramBody[MANAGECMD_KEY_TASKID] = currentTaskID;
        paramBody[MANAGECMD_KEY_ERRORCODE] = errorcode;
        paramBody[MANAGECMD_KEY_ERRORDETAIL] = errorDetail;
    }
    jsRespBody[MANAGECMD_KEY_BODY] = paramBody;
    jsRespBody[MANAGECMD_KEY_CMDNO] = cmd;
    jsRespBody[MANAGECMD_KEY_ERRORCODE] = errorcode;
    jsRespBody[MANAGECMD_KEY_ERRORDETAIL] = errorDetail;
    msg.get()->SetMsgBody(jsRespBody);

    PushMsg2Queue(msg);
}

mp_void VMwareNativeDataPathProcess::ReleaseDiskThreadResource(const mp_string &parentTaskID)
{
    CThreadAutoLock threadLock(&m_taskMapLock);
    // find disk task that need do thread resource cleanup
    std::unordered_map<mp_string, std::vector<mp_string>>::iterator iter = m_mapTaskInfo.find(parentTaskID);
    if (iter == m_mapTaskInfo.end()) {
        WARNLOG("The vm level task '%s' does not exist, maybe error occurs, please check!", parentTaskID.c_str());
    } else {
        // clear thread resource for each disk task
        for (int i = 0; i < iter->second.size(); i++) {
            ReleaseTaskPoolResource(iter->second[i]);
        }
    }
}