#include "apps/oraclenative/OracleNativeInstRestoreTask.h"

#include "common/Utils.h"
#include "taskmanager/TaskContext.h"
#include "apps/oracle/OracleDefines.h"
#include "taskmanager/TaskStepPreSufScript.h"
#include "apps/oraclenative/TaskStepCheckDBClose.h"
#include "apps/oraclenative/TaskStepClearDataBase.h"
#include "apps/oraclenative/TaskStepOracleNativeInstRestore.h"
#include "apps/oraclenative/TaskStepOracleNativeMoveDBF.h"

OracleNativeInstRestoreTask::OracleNativeInstRestoreTask(const mp_string& taskID) : OracleNativeTask(taskID)
{
    m_taskName = "OracleNativeInstRestoreTask";
    statusFlag = MP_FALSE;
    CreateTaskStep();
}

OracleNativeInstRestoreTask::~OracleNativeInstRestoreTask()
{}

mp_int32 OracleNativeInstRestoreTask::InitTaskStep(const Json::Value& param)
{
    // check database valid
    mp_int32 iRet = InitTaskStepParam(param, "", STEPNAME_CHECK_DBCLOSE);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // prefix script
    iRet = InitTaskStepParam(param, KEY_SCRIPTS, STEPNAME_PERSCRIPT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init pre script failed, iRet %d.", iRet);
        return iRet;
    }

    // start database with livemount
    iRet = InitTaskStepParam(param, "", STEPNAME_NATIVE_INST_RESTORE);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init inststore failed, iRet %d.", iRet);
        return iRet;
    }

    // suffix script
    iRet = InitTaskStepParam(param, KEY_SCRIPTS, STEPNAME_SUFSCRIPT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask suff pre script failed, iRet %d.", iRet);
        return iRet;
    }

    // move dbf online
    iRet = InitTaskStepParam(param, "", STEPNAME_NATIVE_MOVE_DBF);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init move dbf failed, iRet %d.", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

/*
step
1.check database exists
2.exec pre script
3.exec livemount
4.exec suf script
5.movedbf
*/
mp_void OracleNativeInstRestoreTask::CreateTaskStep()
{
    LOGGUARD("");
    mp_int32 iStepFive = 5;
    mp_int32 iStepEightyFive = 85;
    ADD_TASKSTEP(TaskStepCheckDBClose, STEPNAME_CHECK_DBCLOSE, iStepFive, m_steps);
    ADD_TASKSTEP(TaskStepPreScript, STEPNAME_PERSCRIPT, iStepFive, m_steps);
    ADD_TASKSTEP(TaskStepOracleNativeInstRestore, STEPNAME_NATIVE_INST_RESTORE, iStepEightyFive, m_steps);
    ADD_TASKSTEP(TaskStepPostScript, STEPNAME_SUFSCRIPT, iStepFive, m_steps);
    ADD_TASKSTEP(TaskStepOracleNativeMoveDBF, STEPNAME_NATIVE_MOVE_DBF, 0, m_steps);

    ADD_CLEARSTEP(STEPNAME_NATIVE_INST_RESTORE, TaskStepClearDataBase, STEPNAME_CLEAR_DATABASE, "");
    ADD_CLEARSTEP(STEPNAME_NATIVE_INST_RESTORE, TaskStepFailPostScript, STEPNAME_FAILEDSCRIPT, KEY_SCRIPTS);
}
