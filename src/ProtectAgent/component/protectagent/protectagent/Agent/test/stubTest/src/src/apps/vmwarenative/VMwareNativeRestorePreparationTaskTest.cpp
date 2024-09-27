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
#include "apps/vmwarenative/VMwareNativeRestorePreparationTaskTest.h"

TEST_F(VMwareNativeRestorePreparationTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeRestorePreparationTask task(taskID);
    Json::Value param;
    // task.InitTaskStep(param);
}

TEST_F(VMwareNativeRestorePreparationTaskTest, PrepareIscsiMediaStub)
{
    mp_string taskID = "1";
    VMwareNativeRestorePreparationTask task(taskID);
    Json::Value param;
    // task.PrepareIscsiMedia(param);
}

TEST_F(VMwareNativeRestorePreparationTaskTest, PrepareNasMediaStub)
{
    mp_string taskID = "1";
    VMwareNativeRestorePreparationTask task(taskID);
    Json::Value param;
    // task.PrepareNasMedia(param);
}

TEST_F(VMwareNativeRestorePreparationTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeRestorePreparationTask task(taskID);
    task.CreateTaskStep();
}