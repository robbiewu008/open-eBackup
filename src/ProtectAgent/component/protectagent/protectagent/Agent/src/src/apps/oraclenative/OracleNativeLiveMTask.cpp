#include "apps/oraclenative/OracleNativeLiveMTask.h"

#include "common/Utils.h"
#include "taskmanager/TaskContext.h"
#include "apps/oracle/OracleDefines.h"
#include "taskmanager/TaskStepPreSufScript.h"
#include "apps/oraclenative/TaskStepCheckDBClose.h"
#include "apps/oraclenative/TaskStepClearDataBase.h"
#include "apps/oraclenative/TaskStepOracleNativeLiveMount.h"

OracleNativeLiveMTask::OracleNativeLiveMTask(const mp_string& taskID) : OracleNativeTask(taskID)
{
    m_taskName = "OracleNativeLiveMTask";
    statusFlag = MP_FALSE;
    CreateTaskStep();
}

OracleNativeLiveMTask::~OracleNativeLiveMTask()
{}

mp_int32 OracleNativeLiveMTask::InitTaskStep(const Json::Value& param)
{
    // check database valid
    mp_int32 iRet = InitTaskStepParam(param, "", STEPNAME_CHECK_DBCLOSE);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init close db failed, iRet %d.", iRet);
        return iRet;
    }

    // prefix script
    iRet = InitTaskStepParam(param, KEY_SCRIPTS, STEPNAME_PERSCRIPT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init presript failed, iRet %d.", iRet);
        return iRet;
    }

    // start database with livemount
    iRet = InitTaskStepParam(param, "", STEPNAME_NATIVE_LIVEMOUNT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init livemount failed, iRet %d.", iRet);
        return iRet;
    }

    // suffix script
    iRet = InitTaskStepParam(param, KEY_SCRIPTS, STEPNAME_SUFSCRIPT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init suff script failed, iRet %d.", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

/*
step
1.check database status
2.exec pre script
3.exec livemount
4.exec suf script
*/
mp_void OracleNativeLiveMTask::CreateTaskStep()
{
    LOGGUARD("");
    mp_int32 iStepFive = 5;
    mp_int32 iStepEightyFive = 85;
    ADD_TASKSTEP(TaskStepCheckDBClose, STEPNAME_CHECK_DBCLOSE, iStepFive, m_steps);
    ADD_TASKSTEP(TaskStepPreScript, STEPNAME_PERSCRIPT, iStepFive, m_steps);
    ADD_TASKSTEP(TaskStepOracleNativeLiveMount, STEPNAME_NATIVE_LIVEMOUNT, iStepEightyFive, m_steps);
    ADD_TASKSTEP(TaskStepPostScript, STEPNAME_SUFSCRIPT, iStepFive, m_steps);

    ADD_CLEARSTEP(STEPNAME_NATIVE_LIVEMOUNT, TaskStepClearDataBase, STEPNAME_CLEAR_DATABASE, "");
    ADD_CLEARSTEP(STEPNAME_NATIVE_LIVEMOUNT, TaskStepFailPostScript, STEPNAME_FAILEDSCRIPT, "scripts");
}
