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
#ifndef _DME_JOB_STATE_DB_TEST_H__
#define _DME_JOB_STATE_DB_TEST_H__

#include "taskmanager/externaljob/JobStateDB.h"
#include "gtest/gtest.h"
#include "stub.h"
#include <memory>

class JobStateDBTest : public testing::Test {
public:
    void SetUp()
    {
        m_mainID = mp_string("11111");
        m_status = 0;
    }

    void TearDown()
    {
    }

    static void SetUpTestCase()
    {
    }

    static void TearDownTestCase()
    {
    }

    mp_string m_mainID;
    mp_string m_subID;
    mp_int32 m_status;
};

#endif