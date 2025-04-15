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
#ifndef AGENT_BACKUP_STEP_VMWARENATIVE_PREPARE_VMWARENASMEDIA_H
#define AGENT_BACKUP_STEP_VMWARENATIVE_PREPARE_VMWARENASMEDIA_H

#include "taskmanager/TaskStepPrepareNasMedia.h"

static const mp_string STEPNAME_PREPARE_VMWARENASMEDIA = "TaskStepPrepareVMwareNasMedia";
class TaskStepPrepareVMwareNasMedia : public TaskStepPrepareNasMedia {
public:
    TaskStepPrepareVMwareNasMedia(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    ~TaskStepPrepareVMwareNasMedia();
    mp_int32 Init(const Json::Value &param);
    mp_int32 Run();
    mp_void SetNasMediaParam(const mp_string &mountParam);
    mp_int32 GetStorageParam(const Json::Value &param);

    mp_string GetNasStorageIP()
    {
        return m_strNasStorageIP;
    }
    mp_string GetOtherStorageIP()
    {
        return m_OtherNasStorageIP;
    }
    mp_string GetNasSharePath()
    {
        return m_strNasSharePath;
    }
    mp_string GetParentTaskId()
    {
        return m_strParentTaskId;
    }

private:
    mp_void SetLogInfo(const mp_string &label, const mp_int32 &errorCode, const std::vector<std::string> &errorParams);
    mp_int32 GetDataTurboAuthInfoFromParam(const Json::Value &param);
    mp_string m_strParentTaskId;
    mp_string m_strDiskTaskId;
    mp_int32 m_iStorageProtocol;
    mp_string m_strNasStorageIP;
    mp_string m_OtherNasStorageIP;
    mp_string m_strNasSharePath;
    mp_string m_dataturboUser;
    mp_string m_dataturboPassword;
    mp_bool isLinkEncry;
    std::vector<std::string> storageDataturboIps;
};

#endif
