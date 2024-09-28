#include "apps/oraclenative/OracleNativeExpireCopyTask.h"

#include <vector>
#include "common/Utils.h"
#include "common/JsonUtils.h"
#include "taskmanager/TaskContext.h"
#include "taskmanager/TaskManager.h"
#include "apps/oraclenative/TaskStepCheckDBOpen.h"
#include "apps/oraclenative/TaskStepOracleNativeExpireCopy.h"

OracleNativeExpireCopyTask::OracleNativeExpireCopyTask(const mp_string& taskID) : OracleNativeTask(taskID)
{
    m_taskName = "OracleNativeExpireCopyTask";
    statusFlag = MP_FALSE;
    CreateTaskStep();
}

OracleNativeExpireCopyTask::~OracleNativeExpireCopyTask()
{}

mp_int32 OracleNativeExpireCopyTask::InitTaskStep(const Json::Value& param)
{
    LOGGUARD("");
    // check database valid
    mp_int32 iRet = InitTaskStepParam(param, "", STEPNAME_CHECK_ORACLE_OPEN);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    iRet = InitTaskStepParam(param, "", STEPNAME_NATIVE_EXPIRECOPY);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init expire copy failed, ret=%d.", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

mp_void OracleNativeExpireCopyTask::CreateTaskStep()
{
    LOGGUARD("");
    mp_int32 iStepFive = 5;
    mp_int32 iStepNinetyFive = 95;
    ADD_TASKSTEP(TaskStepCheckDBOpen, STEPNAME_CHECK_ORACLE_OPEN, iStepFive, m_steps);
    ADD_TASKSTEP(TaskStepOracleNativeExpireCopy, STEPNAME_NATIVE_EXPIRECOPY, iStepNinetyFive, m_steps);
}

Task *OracleNativeExpireCopyTask::CreateRedoTask(const mp_string& taskId)
{
    // get param from database
    taskinfo_t taskInfo;
    mp_int32 iRet = TaskManager::GetInstance()->QueryTaskInfo(taskId, "OracleNativeExpireCopyTask", taskInfo);
    if (iRet != MP_SUCCESS) {
        return NULL;
    }

    // set backup input params
    TaskContext::GetInstance()->SetJsonValue(taskId, KEY_INPUTPARAM, taskInfo.msgBody);
    // connection ip address: reporting data
    TaskContext::GetInstance()->SetValueString(taskId, KEY_CONNECTION_IP, taskInfo.connIp);
    // connection ip address: reporting data
    TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_CONNECTION_PORT, taskInfo.connPort);
    TaskContext::GetInstance()->SetValueString(taskId, g_CUR_RUNNING_STEP, taskInfo.subStep);
    TaskContext::GetInstance()->SetValueString(taskId, g_CUR_INNERPID, taskInfo.innerPID);

    OracleNativeExpireCopyTask *task = new (std::nothrow) OracleNativeExpireCopyTask(taskId);
    if (!task) {
        COMMLOG(OS_LOG_ERROR, "New task failed.");
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        return NULL;
    }

    Json::Value params;
    iRet = CJsonUtils::ConvertStringtoJson(taskInfo.msgBody, params);
    if (iRet != MP_SUCCESS) {
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        delete task;
        return NULL;
    }

    if (!params.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "CreateRedoTask: dpp message string have no body, %s.", taskId.c_str());
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        return NULL;
    }
    
    Json::Value bodyParam = params[MANAGECMD_KEY_BODY];
    task->InitTaskStep(bodyParam);
    if (iRet != MP_SUCCESS) {
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        delete task;
        return NULL;
    }
    
    COMMLOG(OS_LOG_INFO, "Create redo task %s succ.", taskId.c_str());
    return task;
}
