/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "apps/oraclenative/TaskStepOracleNativeMediaLog.h"

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

TaskStepOracleNativeMediaLog::TaskStepOracleNativeMediaLog(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNativeMedia(id, taskId, name, ratio, order)
{}

TaskStepOracleNativeMediaLog::~TaskStepOracleNativeMediaLog()
{}

mp_int32 TaskStepOracleNativeMediaLog::Init(const Json::Value& param)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to init mediaLog.", m_taskId.c_str());
    mp_int32 iRet = InitialDBInfo(param);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // volumes paras
    iRet = InitialVolInfo(param, LOG_VOLUMES_BIT);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    iRet = CheckVolsValid(LOG_VOLUMES_BIT);
    if (iRet != MP_SUCCESS) {
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

mp_int32 TaskStepOracleNativeMediaLog::InitialVolumes(mp_string& diskList)
{
    return GetDiskListByWWN(logVols, diskList);
}

mp_int32 TaskStepOracleNativeMediaLog::InitMountPath()
{
#ifdef WIN32
    static const mp_string ntfsType = "ntfs";
    // using simple volumen when it's windows, and mount to Agent install path
    mp_string mountPath = CPath::GetInstance().GetRootPath() + "\\..\\..\\..\\advbackup\\log\\" + dbName;
    mp_char logPath[MAX_FULL_PATH_LEN];
    mp_bool ret = PathCanonicalize(logPath, mountPath.c_str());
    if (ret == MP_FALSE) {
        COMMLOG(OS_LOG_ERROR, "real backup data path failed, errorno[%d].", GetLastError());
        return MP_FAILED;
    }
    mountPath = logPath;
    backupMedium.InitFs("", "", mountPath, ntfsType);
#else
    static const mp_string xfsType = "xfs";

    // File system mount path
    mp_string vgName = VG_NAME_PRE_LOG + dbName;
    mp_string lvName = LV_NAME_PRE_LOG + dbName;
    mp_string mountPath = FS_MOUNTPATH_PRE_LOG + dbName;
    // get file system, initial FS param
    mp_string backupFSType = xfsType;
    // can't get fs type when restore,livemount,instancerestore
    backupMedium.InitFs(vgName, lvName, mountPath, backupFSType);
#endif

    // ASM system mount path
    mp_string dgName = DG_NAME_PRE_LOG + dbName;
    backupMedium.InitAsm(dgName, asmInstance, asmUser, asmPwd, 0);

    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeMediaLog::PrepareMediumInfo()
{
#ifdef WIN32
    // the windows block log mount path is need to write into database for backup process
    mp_string paramID = dbName + STR_DASH + dbUUID + STR_DASH + m_taskId;
    mp_int32 iRet = UpdateParam(paramID,
        "logPath",
        (dbType == ORA_DBTYPE_ASM) ? backupMedium.GetAsmMountPath() : backupMedium.GetFsMountPath());
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "save log nas path failed, iRet=%d.", iRet);
        return MP_FAILED;
    }
#endif
    return MP_SUCCESS;
}
