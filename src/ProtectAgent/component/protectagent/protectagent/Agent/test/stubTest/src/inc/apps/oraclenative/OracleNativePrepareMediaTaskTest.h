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
#ifndef _AGENT_ORACLE_NATIVE_PREPARE_MEDIA_TASK_TEST_
#define _AGENT_ORACLE_NATIVE_PREPARE_MEDIA_TASK_TEST_
#define private public
#include "apps/oraclenative/OracleNativePrepareMediaTask.h"
#include "cunitpub/publicInc.h"
#include "gtest/gtest.h"
#include "stub.h"

class OracleNativePrepareMediaTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        init_cunit_data();
    }

    static void TearDownTestCase(void)
    {
        destroy_cunit_data();
    }
    Stub stub;
protected:
    virtual void SetUp()
    {
        reset_cunit_counter();
    }

    virtual void TearDown()
    {
        ;
    }
};

#endif