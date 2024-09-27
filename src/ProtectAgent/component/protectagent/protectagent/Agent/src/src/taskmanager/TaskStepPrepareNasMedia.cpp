#include "taskmanager/TaskStepPrepareNasMedia.h"

#include <vector>
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "securecom/RootCaller.h"
#include "securecom/SecureUtils.h"
#include "common/CSystemExec.h"

using namespace std;

TaskStepPrepareNasMedia::TaskStepPrepareNasMedia(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStep(id, taskId, name, ratio, order)
{}

TaskStepPrepareNasMedia::~TaskStepPrepareNasMedia()
{}

mp_int32 TaskStepPrepareNasMedia::Init(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareNasMedia::Run()
{
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareNasMedia::MountNasMedia(const mp_string& scriptParam, vector<mp_string>& vecRst)
{
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to mount backup media.", m_taskId.c_str());

#ifdef WIN32
    mp_int32 iRet = SecureCom::SysExecScript(WIN_HOST_PREPARE_NAS, scriptParam, &vecRst);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, "Mount nas medium failed, ret %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_PREPARE_NASMEDIA, scriptParam, &vecRst);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_INTERNAL_ERROR);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Mount nas media failed, task id %s, ret %d.", m_taskId.c_str(), iRet);
        return iRet;
    }
#endif
    return iRet;
}

#ifndef WIN32
mp_int32 TaskStepPrepareNasMedia::CheckAndCreateDataturboLink(const DataturboMountParam &param)
{
    // only check ip when access nas by ip
    if (!m_accessNASOverFC && param.vecDataturboIP.size() == 0) {
        ERRLOG("File system %s have no valid ip.", param.storageName.c_str());
        return ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT;
    }

    mp_string ipList = CMpString::StrJoin(param.vecDataturboIP, ",");
    CRootCaller rootCaller;
    ostringstream scriptParam;
    scriptParam << "storageName=" << param.storageName << NODE_COLON ;
    if (!m_accessNASOverFC) {
        scriptParam << "ipList=" << ipList << NODE_COLON;
    } else {
        scriptParam << "linkType=FC" << NODE_COLON;
    }

    scriptParam << "userName=" << param.authUser << NODE_COLON << "password=" << param.authPwd << NODE_COLON;
    scriptParam << "dedupSwitch=" << isSrcDedup ? "ON" : "OFF";
    mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_CHECK_AND_CREATE_DATATURBO_LINK, scriptParam.str(), NULL);
    if (ret != MP_SUCCESS) {
        ERRLOG("Check and Create Dataturbo link Failed!");
        return ERR_CREATE_DATA_TURBO_LINK;
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareNasMedia::MountDataturboMedia(const mp_string& scriptParam, vector<mp_string>& vecRst,
    DataturboMountParam &param)
{
    mp_int32 iRet = CheckAndCreateDataturboLink(param);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create Dataturbo link Failed!");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Task(%s) begin to mount backup media through dataturbo protocol.", m_taskId.c_str());
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_PREPARE_DATATURBOMEDIA, scriptParam, &vecRst);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_INTERNAL_ERROR);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Mount Dataturbo media failed, task id %s, ret %d.", m_taskId.c_str(), iRet);
        return ERR_MOUNT_DATA_TURBO_FILE_SYSTEM;
    }
    vecRst.push_back(CMpString::StrJoin(param.vecDataturboIP, NODE_SEMICOLON));
    return MP_SUCCESS;
}
#endif

mp_int32 TaskStepPrepareNasMedia::UmountNasMedia(const mp_string& scriptParam, vector<mp_string>& vecRst)
{
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to umount backup media.", m_taskId.c_str());

#ifdef WIN32
    mp_int32 iRet = SecureCom::SysExecScript(WIN_HOST_UMOUNT_NAS, scriptParam, &vecRst);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, "Umount nas medium failed, ret %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_UMOUNT_NASMEDIA, scriptParam, &vecRst);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_INTERNAL_ERROR);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Umount nas media failed, task id %s, ret %d.", m_taskId.c_str(), iRet);
        return iRet;
    }
#endif
    return iRet;
}

mp_int32 TaskStepPrepareNasMedia::Stop(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareNasMedia::Redo(mp_string& innerPID)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareNasMedia::Cancel()
{
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareNasMedia::Cancel(Json::Value& respParam)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareNasMedia::Update(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareNasMedia::Update(Json::Value& param, Json::Value& respParam)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareNasMedia::Finish(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareNasMedia::Finish(Json::Value& param, Json::Value& respParam)
{
    return MP_SUCCESS;
}
