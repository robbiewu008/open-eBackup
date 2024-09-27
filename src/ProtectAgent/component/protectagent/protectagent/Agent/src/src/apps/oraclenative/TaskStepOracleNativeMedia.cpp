#include "apps/oraclenative/TaskStepOracleNativeMedia.h"

#include "apps/oracle/OracleDefines.h"
#include "array/array.h"
#include "array/disk.h"
#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "message/tcp/CDppMessage.h"
#include "taskmanager/TaskContext.h"

using namespace std;

TaskStepOracleNativeMedia::TaskStepOracleNativeMedia(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNative(id, taskId, name, ratio, order)
{}

TaskStepOracleNativeMedia::~TaskStepOracleNativeMedia()
{}

mp_int32 TaskStepOracleNativeMedia::Init(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeMedia::Run()
{
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to prepare backup media...", m_taskId.c_str());

    mp_string diskList;
    // 1 wwn exists
    mp_int32 iRet = InitialVolumes(diskList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "InitialVolumes failed, ret=%d", iRet);
        return iRet;
    }
    // init backup media only backup need
    mp_int32 taskType;
    iRet = TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_TASKTYPE, taskType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Task(%s) get taskType failed.", m_taskId.c_str());
        return iRet;
    }
    mp_bool bFlag = (taskType == ORACLENATIVE_TASK_DATA_BACKUP) || (taskType == ORACLENATIVE_TASK_LOG_BACKUP);
    if (bFlag && asmInstance.empty()) {
        backupMedium.InitFsType("xfs");
    }
    
    // 3 create Filesystem:vg\lv\fs? ASM:DG
    iRet = PrepareBackupMedium(diskList);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // 4 prepare information for some scenario, .eg asm meta file path in prepare data medium
    iRet = PrepareMediumInfo();
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Task(%s) prepare backup media succ.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeMedia::PrepareMediumInfo()
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeMedia::InitialVolumes(mp_string& diskList)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeMedia::PrepareBackupMedium(const mp_string& diskList)
{
    LOGGUARD("");
    mp_int32 taskType;
    mp_int32 iRet = TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_TASKTYPE, taskType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Task(%s) get taskType failed.", m_taskId.c_str());
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "Task(%s) get taskType is %d.", m_taskId.c_str(), taskType);
    // backup medium need to be created when cmd is backup data or backup log and backup medium isn't existed
    mp_bool bFlag = (taskType == ORACLENATIVE_TASK_DATA_BACKUP) || (taskType == ORACLENATIVE_TASK_LOG_BACKUP);
    if (bFlag) {
        backupMedium.SetCreateMediumWhenNoExist();
    }

    if (dbType == ORA_DBTYPE_ASM) {
        iRet = backupMedium.CreateAsmMedium(diskList);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Task(%s) create asm backup media failed, iRet %d.", m_taskId.c_str(), iRet);
            return iRet;
        }
    } else {
        iRet = backupMedium.CreateFsMedium(diskList);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Task(%s) create file backup media failed, iRet %d.", m_taskId.c_str(), iRet);
            return iRet;
        }
    }

    COMMLOG(OS_LOG_INFO, "create media success.");
    return MP_SUCCESS;
}
