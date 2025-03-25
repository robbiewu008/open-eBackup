/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TVMwareNativeDataPathProcess.h
 * @brief  Contains function declarations about TVMwareNativeDataPathProcess
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef DATA_PATH_VMWARENATIVE_H
#define DATA_PATH_VMWARENATIVE_H

#include <map>
#include <unordered_map>
#include "common/JsonUtils.h"
#include "dataprocess/datapath/DataPath.h"
#include "dataprocess/datamessage/DataMessage.h"
#include "dataprocess/datapath/VMwareNativeDataPathImpl.h"
#include "dataprocess/datapath/HanderThread.h"

// 定义命令枚举变量 -- 可以取代CDppMessage.h中相关命令字的定义
typedef enum tag_vmwarenative_cmd {
    CMD_VMWARENATIVE_CHECK_PRODUCTENV_CONNECTION = 0x0430,
    CMD_VMWARENATIVE_CHECK_PRODUCTENV_CONNECTION_ACK = 0x0431,
    CMD_VMWARENATIVE_QUERY_DISK_TRANSPORTMODE = 0x0432,
    CMD_VMWARENATIVE_QUERY_DISK_TRANSPORTMODE_ACK = 0x0433,
    CMD_VMWARENATIVE_PREPARE_TARGETLUN = 0x0434,
    CMD_VMWARENATIVE_PREPARE_TARGETLUN_ACK = 0x0435,
    CMD_VMWARENATIVE_BACKUP_PREPARATION = 0x0436,
    CMD_VMWARENATIVE_BACKUP_PREPARATION_ACK = 0x0437,
    CMD_VMWARENATIVE_BACKUP_DATABLOCK = 0x0438,
    CMD_VMWARENATIVE_BACKUP_DATABLOCK_ACK = 0x0439,
    CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS = 0x043A,
    CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS_ACK = 0x043B,
    CMD_VMWARENATIVE_FINISH_DATABLOCKBACKUP = 0x043C,
    CMD_VMWARENATIVE_FINISH_DATABLOCKBACKUP_ACK = 0x043D,
    CMD_VMWARENATIVE_FINISH_VMBACKUP = 0x043E,
    CMD_VMWARENATIVE_FINISH_VMBACKUP_ACK = 0x043F,
    CMD_VMWARENATIVE_CANCEL_VMBACKUP = 0x0440,
    CMD_VMWARENATIVE_CANCEL_VMBACKUP_ACK = 0x0441,
    CMD_VMWARENATIVE_RECOVERY_PREPARATION = 0x0442,
    CMD_VMWARENATIVE_RECOVERY_PREPARATION_ACK = 0x0443,
    CMD_VMWARENATIVE_RESTORE_DATABLOCK = 0x0444,
    CMD_VMWARENATIVE_RESTORE_DATABLOCK_ACK = 0x0445,
    CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS = 0x0446,
    CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS_ACK = 0x0447,
    CMD_VMWARENATIVE_FINISH_DATABLOCKRECOVERY = 0x0448,
    CMD_VMWARENATIVE_FINISH_DATABLOCKRECOVERY_ACK = 0x0449,
    CMD_VMWARENATIVE_FINISH_VMRECOVERY = 0x044A,
    CMD_VMWARENATIVE_FINISH_VMRECOVERY_ACK = 0x044B,
    CMD_VMWARENATIVE_CANCEL_VMRECOVERY = 0x044C,
    CMD_VMWARENATIVE_CANCEL_VMRECOVERY_ACK = 0x044D,
    CMD_VMWARENATIVE_CLEANUP_RESOURCES = 0x044E,
    CMD_VMWARENATIVE_CLEANUP_RESOURCES_ACK = 0x044F,
    CMD_VMWARENATIVE_INIT_VDDKLIB = 0x0450,
    CMD_VMWARENATIVE_INIT_VDDKLIB_ACK = 0x0451,
    CMD_VMWARENATIVE_CLEANUP_VDDKLIB = 0x0454,
    CMD_VMWARENATIVE_CLEANUP_VDDKLIB_ACK = 0x0455,
    CMD_VMWARENATIVE_BACKUP_OPENDISK = 0x046A,
    CMD_VMWARENATIVE_BACKUP_OPENDISK_ACK = 0x046B,
    CMD_VMWARENATIVE_BACKUP_CLOSEDISK = 0x047A,
    CMD_VMWARENATIVE_BACKUP_CLOSEDISK_ACK = 0x047B,

    CMD_VMWARENATIVE_BACKUP_AFS = 0x0476,
    CMD_VMWARENATIVE_BACKUP_AFS_ACK = 0x0477
} vmwarenative_cmd;

// 定义VMware native DataPath命令处理函数
using vmwareMsgHandlerFunPtr =
    std::function<mp_int32(std::shared_ptr<CDppMessage>, const mp_string&, const mp_string&)>;
using VMwareMsgHandlerFunMap = struct SVMwareMsgHandlerFunMap {
    mp_int32 cmd;
    vmwareMsgHandlerFunPtr funPtr;
};

class VMwareNativeDataPathProcess : public DataPath {
public:
    VMwareNativeDataPathProcess(const mp_string &vddkVer);
    ~VMwareNativeDataPathProcess();

    // Handle Message received in the base class
    mp_int32 ExtCmdProcess(CDppMessage &message);
    mp_int32 HandleProtect(Json::Value msgBody);
    mp_int32 HandlePause(Json::Value &msgBody);
    mp_int32 HandleResume(Json::Value &msgBody);
    mp_void HandleClose();
private:

    // functions interact with remote product ENV
    EXTER_ATTACK mp_int32 HandleVddkLibInit(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleVddkLibCleanup(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleTargetLunPrepare(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    mp_int32 HandleDataBlockVerify(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);

    // backup
    EXTER_ATTACK mp_int32 HandleBackupPreparation(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleBackupOpenDisk(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleBackupCloseDisk(std::shared_ptr<CDppMessage> message,
    const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleDataBlockBackupRun(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleBackupProgressQuery(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleVmBackupCancel(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleDataBlockBackupFinish(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleVmBackupFinish(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    // invalid data identify
    EXTER_ATTACK mp_int32 HandleDisksAfsBitmap(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);

    // restore
    EXTER_ATTACK mp_int32 HandleRecoveryPreparation(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleDataBlockRestoreRun(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleRestoreProgressQuery(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleVmRestoreCancel(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleDataBlockRestoreFinish(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);
    EXTER_ATTACK mp_int32 HandleVmRestoreFinish(std::shared_ptr<CDppMessage> message,
        const mp_string &currentTaskID, const mp_string &parentTaskID);

    // GenertateMap
    void CleanPreviousThread();
    mp_int32 GenerateMap(CDppMessage &message, mp_string &currentTaskID, mp_string &parentTaskID);
    mp_int32 GenerateVMwareDpImplInstanceMap(mp_string &currentTaskID, mp_string &parentTaskID);
    mp_int32 GenerateVMwareThreadHanderMap(mp_string &currentTaskID, mp_string &parentTaskID);

    // functions that send response to local host agent
    mp_void SendNoBodyResponse(CDppMessage &reqmsg,
        mp_int32 cmd, mp_int32 errorcode, mp_string &errDetail, const mp_string &currentTaskID);
    mp_void SendCommonDppResponse(CDppMessage &reqmsg, Json::Value &paramBody, mp_int32 cmd, mp_int32 &errorcode,
        mp_string &errDetail, const mp_string &currentTaskID);
    mp_void SendProductEnvConnDppResponse(CDppMessage &reqmsg, std::map<mp_string, vmware_pe_info> &status);

    mp_int32 VMwareNativeDpCmdParse(CDppMessage &message, mp_string &currentTaskID, mp_string &parentTaskID);
    mp_int32 DoVMwareNativeFunc(std::shared_ptr<VMwareNativeDataPathImpl> &mapVMwareDpImpl,
        const mp_string &currentTaskID);

    mp_void GenerateCmdMsgHandlerFunMap();
    mp_void GenerateCmdMsgHandlerFunMapBackup();
    mp_void RemoveVMwareThreadHander(const mp_string &parentTaskID,
        std::vector<std::shared_ptr<HanderThread>> &removeThreadList);
    mp_void ReleaseVMwareThreadHander(const mp_string &parentTaskID);
    mp_void ReleaseTaskPoolResource(const mp_string& taskId);
    mp_void RemoveDataPathImplInstance(const mp_string& taskId);
    mp_void RemoveTaskMapItem(const mp_string& taskId);
    mp_bool QueryTaskMapItem(const mp_string &taskId);
    mp_void AddTaskMapItem(const mp_string &pTaskId, const mp_string &sTaskId);
    mp_void ReleaseDiskThreadResource(const mp_string &parentTaskID);

private:
    thread_lock_t m_datapathMapLock;
    thread_lock_t m_taskMapLock;
    thread_lock_t m_handerLock;
    // mapping of taskid and VMwareNativeDataPathImpl instance
    std::map<mp_string, std::shared_ptr<VMwareNativeDataPathImpl>> m_mapVMwareDpImplInstance;
    // mapping of parent task id  and  thread
    std::map<mp_string, std::shared_ptr<HanderThread>> m_mapVMwareThreadHander;
    // mapping of cmd and function
    std::unordered_map<mp_uint32, std::function<mp_int32(std::shared_ptr<CDppMessage> message,
        const mp_string currentTaskID, const mp_string parentTaskID)>> m_mapCmdMsgHandlerFun;
    // save mapping of parent task and disk task
    std::unordered_map<mp_string, std::vector<mp_string>> m_mapTaskInfo;
    std::vector<mp_string> m_prepareCleanThread;
};

#endif
