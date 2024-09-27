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
#ifndef __AGENT_TASKSTEP_BACKUP_PREPARATION_H__
#define __AGENT_TASKSTEP_BACKUP_PREPARATION_H__

#include "apps/vmwarenative/TaskStepBackupPreparation.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepBackupPreparationTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepBackupPreparationTest::SetUp()
{}

void TaskStepBackupPreparationTest::TearDown()
{}

void TaskStepBackupPreparationTest::SetUpTestCase()
{}

void TaskStepBackupPreparationTest::TearDownTestCase()
{}

#endif
