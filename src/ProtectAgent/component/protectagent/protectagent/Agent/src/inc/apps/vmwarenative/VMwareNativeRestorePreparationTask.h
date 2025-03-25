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
#ifndef AGENT_VMWARENATIVE_RESTORE_PREPARATION_TASK
#define AGENT_VMWARENATIVE_RESTORE_PREPARATION_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeRestorePreparationTask : public VMwareNativeTask {
public:
    VMwareNativeRestorePreparationTask(const mp_string &taskID);
    virtual ~VMwareNativeRestorePreparationTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
    mp_int32 PrepareNasMedia(const Json::Value &param);
    mp_int32 PrepareIscsiMedia(const Json::Value &param);

private:
    mp_int32 m_storageProtocol;
    mp_string m_diskType;
};

#endif
