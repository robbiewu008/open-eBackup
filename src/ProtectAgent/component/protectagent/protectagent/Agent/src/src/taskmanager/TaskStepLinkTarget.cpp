#include "taskmanager/TaskStepLinkTarget.h"

#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "securecom/RootCaller.h"
#include "taskmanager/TaskContext.h"
#include "host/host.h"

using namespace std;

TaskStepLinkTarget::TaskStepLinkTarget(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStep(id, taskId, name, ratio, order)
{}

TaskStepLinkTarget::~TaskStepLinkTarget()
{}

mp_int32 TaskStepLinkTarget::Init(const Json::Value& param)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to init linkTarget.", m_taskId.c_str());

    if (!param.isObject()) {
        COMMLOG(OS_LOG_ERROR, "The value storage of backup Json key is not object.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    scsiTargets = param;

    m_stepStatus = STATUS_INITIAL;
    return MP_SUCCESS;
}

mp_int32 TaskStepLinkTarget::Run()
{
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to link scsi target...", m_taskId.c_str());
    if (m_stepStatus != STATUS_INITIAL) {
        COMMLOG(OS_LOG_ERROR, "TaskStepLinkTarget have not be initialized");
        return MP_FAILED;
    }

    CHost host;
    mp_int32 iRet = host.LinkiScsiTarget(scsiTargets);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    m_stepStatus = STATUS_INPROGRESS;
    COMMLOG(OS_LOG_INFO, "Link scsi target succ.");
    return MP_SUCCESS;
}

mp_int32 TaskStepLinkTarget::Stop(const Json::Value& param)
{
    m_stepStatus = STATUS_COMPLETED;
    return MP_SUCCESS;
}

mp_int32 TaskStepLinkTarget::Redo(mp_string& innerPID)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepLinkTarget::Cancel()
{
    m_stepStatus = STATUS_DELETED;
    return MP_SUCCESS;
}

mp_int32 TaskStepLinkTarget::Cancel(Json::Value& respParam)
{
    m_stepStatus = STATUS_DELETED;
    return MP_SUCCESS;
}

mp_int32 TaskStepLinkTarget::Update(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepLinkTarget::Update(Json::Value& param, Json::Value& respParam)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepLinkTarget::Finish(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepLinkTarget::Finish(Json::Value& param, Json::Value& respParam)
{
    return MP_SUCCESS;
}
