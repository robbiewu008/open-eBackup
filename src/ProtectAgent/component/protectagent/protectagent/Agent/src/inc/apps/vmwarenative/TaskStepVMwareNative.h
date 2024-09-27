#ifndef AGENT_BACKUP_STEP_VMWARENATIVE_H
#define AGENT_BACKUP_STEP_VMWARENATIVE_H

#include "common/Types.h"
#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/value.h"
#include "taskmanager/TaskStep.h"
#include "taskmanager/TaskContext.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "message/tcp/CDppMessage.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"

class TaskStepVMwareNative : public TaskStep {
public:
    TaskStepVMwareNative(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    virtual ~TaskStepVMwareNative();

    virtual mp_int32 Init(const Json::Value &param);
    virtual mp_int32 Run();
    virtual mp_int32 Cancel();
    virtual mp_int32 Cancel(Json::Value &respParam);
    virtual mp_int32 Stop(const Json::Value &param);
    virtual mp_int32 Update(const Json::Value &param);
    virtual mp_int32 Update(Json::Value &param, Json::Value &respParam);
    virtual mp_int32 Finish(const Json::Value &param);
    virtual mp_int32 Finish(Json::Value &param, Json::Value &respParam);

    mp_int32 DataProcessLogic(Json::Value &param, Json::Value &respParam, mp_uint32 reqCmd, mp_uint32 rspCmd);
    mp_bool IsVddkLibInited(mp_void);
    mp_int32 Redo(mp_string &innerPID);

protected:
    mp_int32 FillSpeedAndProgress(mp_string vecResult, mp_int32 &status, mp_int32 &progress);
    mp_int32 ConvertStatus(const mp_string &status);
    mp_int32 ConvertBackupSpeed(mp_string speed);
    mp_string GetThumbPrint(const mp_string& pIp, mp_uint32 uPort);

protected:
    Json::Value m_reqMsgToDataProcess;
    Json::Value m_respMsgFromDataProcess;
    mp_bool m_bTargetLinked;
    mp_bool m_bDiskScaned;
    mp_string m_strCurrentVddkVersion;
    // timeout between rdagent and dataprocess
    mp_int32 m_internalTimeout;
    // hostagent system type
    mp_int32 m_iSystemVirt;
    mp_bool m_bObtainSystemVirt;

private:
    mp_int32 ExchangeMsgWithDataProcessService(
        Json::Value &param, Json::Value &respParam, mp_uint32 reqCmd, mp_uint32 rspCmd, mp_uint32 timeout = 30);
    mp_int32 ResponseMsgProcesser(
        mp_uint32 exCmd, mp_uint64 exSeq, CDppMessage *dppMsg, mp_int32 &dppError, Json::Value &respParam);
    mp_void CreateFadeResponseMsg(
        mp_uint32 rspCmd, mp_int32 errorCode, const mp_string &errorDetail, Json::Value &respParam);

    mp_int32 ObtainSystemVirtValue();
    mp_bool DPRestartRecently();
};

#endif
