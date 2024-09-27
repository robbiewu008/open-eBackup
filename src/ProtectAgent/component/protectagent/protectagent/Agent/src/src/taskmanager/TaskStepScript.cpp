#include "taskmanager/TaskStepScript.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/CSystemExec.h"
#include "securecom/RootCaller.h"
#include "securecom/SecureUtils.h"

using namespace std;

TaskStepScript::TaskStepScript(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStep(id, taskId, name, ratio, order)
{
    (mp_void) CMpThread::InitLock(&m_statusFileLocker);
}

TaskStepScript::~TaskStepScript()
{
    (mp_void) CMpThread::DestroyLock(&m_statusFileLocker);
}

mp_int32 TaskStepScript::Init(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_void TaskStepScript::SetRunMode(ScriptRunMode runMode)
{
    m_runMode = runMode;
}

mp_void TaskStepScript::InitBuiltInWithRdAdminExec(const mp_string& startScript, const mp_string& startScriptParam,
    const mp_string& stopScript, const mp_string& stopScriptParam)
{
    m_startScript = startScript;
    m_startScriptParam = startScriptParam;
    m_stopScript = stopScript;
    m_stopScriptParam = stopScriptParam;
    m_runMode = SCRIPT_RUN_WITH_RDADMIN;
}

mp_void TaskStepScript::InitBuiltInWithRootExec(
    mp_int32 startScriptId, const mp_string& startScriptParam, mp_int32 stopScriptId, const mp_string& stopScriptParam)
{
    m_startScriptId = startScriptId;
    m_startScriptParam = startScriptParam;
    m_stopScriptId = stopScriptId;
    m_stopScriptParam = stopScriptParam;
    m_runMode = SCRIPT_RUN_WITH_ROOT;
}

mp_void TaskStepScript::Init3rdScript(const mp_string& startScript, const mp_string& startScriptParam,
    const mp_string& stopScript, const mp_string& stopScriptParam, mp_bool affectResult)
{
    m_startScript = startScript;
    m_startScriptParam = startScriptParam;
    m_stopScript = stopScript;
    m_stopScriptParam = stopScriptParam;
    m_runMode = SCRIPT_RUN_WITH_ROOT;
    is3rdScript = MP_TRUE;
    m_affectResult = affectResult;
}

mp_int32 TaskStepScript::Run()
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to run thirdparty script.", m_taskId.c_str());
    if (m_startScript.empty()) {
        COMMLOG(OS_LOG_WARN, "Start script is null, user maybe dont have the script");
        return MP_SUCCESS;
    }

    m_stepStatus = STATUS_INPROGRESS;
    mp_int32 iRet = MP_SUCCESS;
    if (is3rdScript) {
        iRet = RunThirdPartyScriptAsRoot(m_startScript, m_startScriptParam);
    } else if (m_runMode == SCRIPT_RUN_WITH_ROOT) {
        iRet = RunScriptByRoot(m_startScriptId, m_startScriptParam);
    } else if (m_runMode == SCRIPT_RUN_WITH_RDADMIN) {
        iRet = RunScriptByRdAdmin(m_startScript, m_startScriptParam);
    } else {
        COMMLOG(OS_LOG_ERROR, "Run Task Step: run mode exec failed.");
        return MP_FAILED;
    }

    m_stepStatus = (iRet != MP_SUCCESS) ? STATUS_FAILED : STATUS_COMPLETED;
    if (iRet == MP_SUCCESS) {
        return iRet;
    }

    if (!m_affectResult) {
        // the script does not affect final result
        std::pair<mp_string, mp_uint32> tempPair(ORACLE_POST_SCRIPT_FAIL_LABEL, iRet);
        m_vecWarnInfo.emplace_back(tempPair);
        return MP_SUCCESS;
    }
    return iRet;
}

mp_int32 TaskStepScript::Cancel()
{
    LOGGUARD("");
    if (m_stopScript.empty()) {
        COMMLOG(OS_LOG_ERROR, "Stop script is null.");
        return MP_FAILED;
    }

    m_stepStatus = STATUS_DELETING;
    mp_int32 iRet = MP_SUCCESS;
    if (is3rdScript) {
        iRet = RunThirdPartyScriptAsRoot(m_stopScript, m_stopScriptParam);
    } else if (m_runMode == SCRIPT_RUN_WITH_ROOT) {
        iRet = RunScriptByRoot(m_stopScriptId, m_stopScriptParam);
    } else if (m_runMode == SCRIPT_RUN_WITH_RDADMIN) {
        iRet = RunScriptByRdAdmin(m_stopScript, m_stopScriptParam);
    } else {
        COMMLOG(OS_LOG_ERROR, "Stop Task Step: run mode exec failed.");
        m_stepStatus = STATUS_FAILED;
        return MP_FAILED;
    }

    m_stepStatus = (iRet != MP_SUCCESS) ? STATUS_ABORTED : STATUS_DELETED;
    return iRet;
}

mp_int32 TaskStepScript::Cancel(Json::Value &respParam)
{
    LOGGUARD("");
    if (m_stopScript.empty()) {
        COMMLOG(OS_LOG_ERROR, "Stop script is null.");
        return MP_FAILED;
    }

    m_stepStatus = STATUS_DELETING;
    mp_int32 iRet = MP_SUCCESS;
    if (is3rdScript) {
        iRet = RunThirdPartyScriptAsRoot(m_stopScript, m_stopScriptParam);
    } else if (m_runMode == SCRIPT_RUN_WITH_ROOT) {
        iRet = RunScriptByRoot(m_stopScriptId, m_stopScriptParam);
    } else if (m_runMode == SCRIPT_RUN_WITH_RDADMIN) {
        iRet = RunScriptByRdAdmin(m_stopScript, m_stopScriptParam);
    } else {
        COMMLOG(OS_LOG_ERROR, "Stop Task Step: run mode exec failed.");
        m_stepStatus = STATUS_FAILED;
        return MP_FAILED;
    }

    m_stepStatus = (iRet != MP_SUCCESS) ? STATUS_ABORTED : STATUS_DELETED;
    return iRet;
}

mp_int32 TaskStepScript::Stop(const Json::Value& param)
{
    COMMLOG(OS_LOG_ERROR, "Stop script isn't supported.");
    return MP_FAILED;
}

mp_int32 TaskStepScript::Redo(mp_string& innerPID)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepScript::Update(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepScript::Update(Json::Value& param, Json::Value& respParam)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepScript::Finish(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepScript::Finish(Json::Value& param, Json::Value& respParam)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepScript::RefreshStepInfo()
{
    LOGGUARD("");
    CThreadAutoLock tlock(&m_statusFileLocker);
    if (m_stepProgress == STATUS_ABORTED) {
        return MP_SUCCESS;
    }

    mp_string strFileName = PROGRESS_TMP_FILE + m_stepId;
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    vector<mp_string> vecOutput;
    mp_int32 iRet = CMpFile::ReadFile(strFilePath, vecOutput);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Read task status file failed, iRet=%d", iRet);
        return MP_FAILED;
    }
    mp_string memTaskStatusAndProgress = vecOutput.front();
    vecOutput.clear();

    // parse
    mp_int32 status;
    std::list<mp_string> listRes;
    mp_string seporator = ":";
    CMpString::StrToken(memTaskStatusAndProgress, seporator, listRes);
    listRes.pop_front();
    std::stringstream ss;
    ss << listRes.front();
    ss >> status;
    ss.clear();
    m_stepStatus = static_cast<TaskStatus>(status);

    listRes.pop_front();
    ss << listRes.front();
    ss >> m_stepProgress;
    ss.clear();
    
    COMMLOG(OS_LOG_DEBUG, "Read task status file success, status=%d, progress=%d.", m_stepStatus, m_stepProgress);
    return MP_SUCCESS;
}

mp_int32 TaskStepScript::RunScriptByRdAdmin(const mp_string& scriptName, const mp_string& param)
{
    LOGGUARD("");
    if (scriptName.empty()) {
        COMMLOG(OS_LOG_ERROR, "normal script name is empty, can not execute");
        return MP_FAILED;
    }

    mp_string strParam = m_stepId.append("").append(param);
    vector<mp_string> vecResult;
    mp_int32 iRet = SecureCom::SysExecScript(scriptName, strParam, &vecResult);
    if (MP_FAILED == iRet) {
        COMMLOG(OS_LOG_ERROR,
            "normal script %s execute failed, possiblly being terminated, return code is  %d.",
            BaseFileName(scriptName).c_str(),
            iRet);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 TaskStepScript::RunScriptByRoot(mp_int32 scriptId, const mp_string& param)
{
    LOGGUARD("");
    if (scriptId == 0) {
        COMMLOG(OS_LOG_ERROR, "root script id is zero, can not execute");
        return MP_FAILED;
    }

    // check windows or linux , invoke orcale_backup.bat or orcale_backup.sh
#ifdef WIN32
    // windows is not implement
    COMMLOG(OS_LOG_ERROR, "windows is not support to execute as administrator");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    // linux invoke orcale_backup.sh as root
    COMMLOG(OS_LOG_INFO, "before CRootCaller::Exec");
    vector<mp_string> vecResult;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec(scriptId, param, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet) {
        COMMLOG(
            OS_LOG_ERROR, "linux root script execute failed, possiblly being terminated, return code is  %d.", iRet);
        return MP_FAILED;
    } else {
        COMMLOG(OS_LOG_INFO, "linux root script execute success");
        return MP_SUCCESS;
    }
#endif
}

mp_int32 TaskStepScript::RunThirdPartyScriptAsRoot(const mp_string& scriptName, const mp_string& param)
{
    LOGGUARD("");
    if (scriptName.empty()) {
        COMMLOG(OS_LOG_ERROR, "Input parameter file name is null.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_string fileName = CMpString::Trim(scriptName);
    if (fileName.empty()) {
        return MP_SUCCESS;
    }
    mp_int32 iRet;
    vector<mp_string> vecResult;
#ifndef WIN32
    CRootCaller rootCaller;
    mp_string strInput = fileName + NODE_COLON + "1" + NODE_COLON;
    COMMLOG(OS_LOG_INFO, "The full script is \"%s\".", strInput.c_str());
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_THIRDPARTY, strInput, &vecResult);
#else
    mp_int32 iRettmp = MP_SUCCESS;
    // SecureCom::SysExecScript只获取到bin目录下，需组装至thridparty目录
    mp_string strInput = mp_string(AGENT_THIRDPARTY_DIR) + PATH_SEPARATOR + fileName;

    // 调用时，默认参数设为FALSE，不验证脚本签名
    iRet = SecureCom::SysExecScript(strInput, param, &vecResult, MP_FALSE);
    if (iRet != MP_SUCCESS) {
        iRettmp = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(
            OS_LOG_ERROR, "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iRettmp);
        iRet = iRettmp;
    }
#endif
    TRANSFORM_RETURN_CODE(iRet, ERROR_HOST_THIRDPARTY_EXEC_FAILED);
    if (iRet == ERROR_COMMON_SCRIPT_FILE_NOT_EXIST) {
        COMMLOG(OS_LOG_WARN, "%s is not exist.", fileName.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }

    if (iRet != MP_SUCCESS) {
        iRet = (iRet == INTER_ERROR_SRCIPT_FILE_NOT_EXIST) ? ERROR_COMMON_SCRIPT_FILE_NOT_EXIST : iRet;
        COMMLOG(OS_LOG_ERROR, "Exec task step thirdparty script failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Exec task step thirdparty script succ.");
    return MP_SUCCESS;
}

mp_int32 TaskStepScript::UpdateTaskStatusFile()
{
    std::string memTaskStatusAndProgress = "";

    std::stringstream ss;
    ss << m_stepId;
    memTaskStatusAndProgress += ss.str() + ":";
    ss.clear();

    ss << GetStatus();
    memTaskStatusAndProgress += ss.str() + ":";
    ss.clear();

    ss << GetProgress();
    memTaskStatusAndProgress += ss.str();
    ss.clear();

    CThreadAutoLock tlock(&m_statusFileLocker);
    mp_int32 iRet = CIPCFile::WriteInput(m_stepId, memTaskStatusAndProgress);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Write task status file failed, iRet = %d", iRet);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
