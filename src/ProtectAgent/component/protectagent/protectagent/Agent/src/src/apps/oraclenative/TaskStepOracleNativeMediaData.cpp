#include "apps/oraclenative/TaskStepOracleNativeMediaData.h"

#ifdef WIN32
#include <shlwapi.h>
#endif
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/Path.h"
#include "message/tcp/CDppMessage.h"
#include "taskmanager/TaskContext.h"
#include "apps/oracle/OracleDefines.h"

using namespace std;

TaskStepOracleNativeMediaData::TaskStepOracleNativeMediaData(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNativeMedia(id, taskId, name, ratio, order)
{}

TaskStepOracleNativeMediaData::~TaskStepOracleNativeMediaData()
{}

mp_int32 TaskStepOracleNativeMediaData::Init(const Json::Value& param)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to init mediaData.", m_taskId.c_str());
    mp_int32 iRet = InitialDBInfo(param);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init DB Info failed, ret=%d.", iRet);
        return iRet;
    }

    // volumes paras
    iRet = InitialVolInfo(param, DATA_VOLUMES_BIT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init volumes paras failed, ret=%d.", iRet);
        return iRet;
    }

    iRet = CheckVolsValid(DATA_VOLUMES_BIT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "check vols validity failed, ret=%d.", iRet);
        return iRet;
    }

    iRet = InitMountPath();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "initial mount path failed, ret=%d.", iRet);
        return iRet;
    }

    iRet = InitExtrasParams(param);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "initial extras params failed, ret=%d.", iRet);
        return iRet;
    }
    backupMedium.InitExtrasParams(hostRole);

    m_stepStatus = STATUS_INITIAL;
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeMediaData::InitialVolumes(mp_string& diskList)
{
    return GetDiskListByWWN(dataVols, diskList);
}

mp_int32 TaskStepOracleNativeMediaData::InitMountPath()
{
#ifdef WIN32
    static const mp_string ntfsType = "ntfs";
    // using simple volumen when it's windows, and mount to Agent install path
    mp_string mountPath = CPath::GetInstance().GetRootPath() + "\\..\\..\\..\\advbackup\\data\\" + dbName;
    mp_char dataPath[MAX_FULL_PATH_LEN];
    mp_bool ret = PathCanonicalize(dataPath, mountPath.c_str());
    if (ret == MP_FALSE) {
        COMMLOG(OS_LOG_ERROR, "real backup log path failed, errorno[%d].", GetLastError());
        return MP_FAILED;
    }
    mountPath = dataPath;
    backupMedium.InitFs("", "", mountPath, ntfsType);
#else
    static const mp_string xfsType = "xfs";
    // file system mount path
    mp_string vgName = VG_NAME_PRE_DATA + dbName;
    mp_string lvName = LV_NAME_PRE_DATA + dbName;
    mp_string mountPath = FS_MOUNTPATH_PRE_DATA + dbName;
    // can't get fs type when restore,livemount,instancerestore
    backupMedium.InitFs(vgName, lvName, mountPath, xfsType);
    backupMedium.InitFsType(xfsType);
#endif
    // asm mount path
    mp_string dgName = DG_NAME_PRE_DATA + dbName;
    backupMedium.InitAsm(dgName, asmInstance, asmUser, asmPwd, 1);
    backupMedium.InitMetaPath(mountPath);

    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeMediaData::PrepareMediumInfo()
{
#ifdef WIN32
    // the nas mount path is need to write into database for backup process
    mp_string paramID = dbName + STR_DASH + dbUUID + STR_DASH + m_taskId;
    mp_int32 iRet = UpdateParam(paramID,
        "dataPath",
        (dbType == ORA_DBTYPE_ASM) ? backupMedium.GetAsmMountPath() : backupMedium.GetFsMountPath());
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "save data path failed, iRet=%d.", iRet);
        return MP_FAILED;
    }

    iRet = UpdateParam(paramID, "metaPath", backupMedium.GetMetaPath());
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "save metadata path failed, iRet=%d.", iRet);
        return MP_FAILED;
    }
#endif
    return MP_SUCCESS;
}
