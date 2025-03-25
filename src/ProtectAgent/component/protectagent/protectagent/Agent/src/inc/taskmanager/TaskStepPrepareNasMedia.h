#ifndef AGENT_BACKUP_STEP_PREPARE_NAS_MEDIA_H
#define AGENT_BACKUP_STEP_PREPARE_NAS_MEDIA_H

#include <vector>
#include "TaskStep.h"
#include "common/Types.h"
#include "common/ErrorCode.h"

static const mp_string STEPNAME_PREPARENASMEDIA = "TaskStepPrepareNasMedia";
static const mp_string WIN_HOST_PREPARE_NAS = "preparenasmedia.bat";
static const mp_string WIN_HOST_UMOUNT_NAS = "umountnasmedia.bat";
struct DataturboMountParam {
    mp_string storageName;
    std::vector<mp_string> vecDataturboIP;
    mp_string authUser;
    mp_string authPwd;
};

class TaskStepPrepareNasMedia : public TaskStep {
public:
    TaskStepPrepareNasMedia(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepPrepareNasMedia();

    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 Cancel();
    mp_int32 Cancel(Json::Value& respParam);
    mp_int32 Redo(mp_string& innerPID);
    mp_int32 Stop(const Json::Value& param);
    mp_int32 Update(const Json::Value& param);
    mp_int32 Update(Json::Value& param, Json::Value& respParam);
    mp_int32 Finish(const Json::Value& param);
    mp_int32 Finish(Json::Value& param, Json::Value& respParam);

    mp_int32 MountNasMedia(const mp_string& scriptParam, std::vector<mp_string>& vecRst);
    mp_int32 UmountNasMedia(const mp_string& scriptParam, std::vector<mp_string>& vecRst);
#ifndef WIN32
    mp_int32 MountDataturboMedia(const mp_string& scriptParam, std::vector<mp_string>& vecRst,
        DataturboMountParam &param);
    mp_int32 CheckAndCreateDataturboLink(const DataturboMountParam &param);
#endif
    mp_bool m_accessNASOverFC = MP_FALSE;
    mp_bool isSrcDedup = MP_FALSE;
};

#endif