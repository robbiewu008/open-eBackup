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
#ifndef AGENT_BACKUP_STEP_PREPARE_ORACLENASMEDIA_H
#define AGENT_BACKUP_STEP_PREPARE_ORACLENASMEDIA_H

#include "common/Types.h"
#include "taskmanager/TaskStepPrepareNasMedia.h"
#include "apps/oraclenative/TaskStepOracleNative.h"

static const mp_string STEPNAME_PREPARE_ORACLENASMEDIA = "TaskStepOracleNativeNasMedia";
class TaskStepOracleNativeNasMedia : public TaskStepPrepareNasMedia {
public:
    TaskStepOracleNativeNasMedia(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeNasMedia();

    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 Cancel();
    mp_int32 Stop(const Json::Value& param);
    mp_int32 Update(const Json::Value& param);
    mp_int32 Finish(const Json::Value& param);

private:
    mp_int32 InitPara(const Json::Value& param);
    mp_int32 UpdateDatabaseParam(const std::vector<mp_string> &vecRst);
    mp_int32 MountStorageNasMedia(std::vector<mp_string> &vecRst);
    mp_int32 GetStorageIP(const Json::Value& jsonStor);
    void SetLogInfo(const mp_string& label, const mp_int32& errorCode, const std::vector<std::string>& errorParams);
    mp_int32 m_hostRole;
    mp_int32 m_taskType;
    mp_int32 m_storType;
    mp_string m_dataSharePath;
    std::vector<mp_string> m_VecDataOwnerIps;
    std::vector<mp_string> m_VecDataOtherIps;
    mp_string m_logSharePath;
    std::vector<mp_string> m_VecLogOwnerIps;
    std::vector<mp_string> m_VecLogOtherIps;
    std::vector<mp_string> m_VecDataturboIps;
    mp_string m_dbName;
    mp_string m_dbUUID;
    // user and key are used in iscsi CHAP or cifs authentication
    mp_string m_authUser;
    mp_string m_authKey;
    mp_int32 m_isLinkEncry = 0;
    mp_int32 m_isSrcDedup = 0;
};

#endif
