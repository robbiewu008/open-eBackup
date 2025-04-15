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
#ifndef AGENT_VMWARENATIVE_BACKUP_OPENDISK_TASK
#define AGENT_VMWARENATIVE_BACKUP_OPENDISK_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeBackupOpenDiskTask : public VMwareNativeTask {
public:
    VMwareNativeBackupOpenDiskTask(const mp_string &taskID);
    virtual ~VMwareNativeBackupOpenDiskTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
};

#endif
