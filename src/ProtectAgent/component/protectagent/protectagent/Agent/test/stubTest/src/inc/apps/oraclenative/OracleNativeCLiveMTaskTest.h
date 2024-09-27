
/* 
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved. 
 * Description: no on-premies database cancel live moun task 
 * Author: wangguitao
 * Create: 2020-02-04 
 * Notes: cancel database livemount
 */
#ifndef _AGENT_ORACLE_NATIVE_CANCEL_LIVEMOUNT_TASK_TEST_
#define _AGENT_ORACLE_NATIVE_CANCEL_LIVEMOUNT_TASK_TEST_
#define private public
#include "apps/oraclenative/OracleNativeCLiveMTask.h"
#include "cunitpub/publicInc.h"
#include "gtest/gtest.h"
#include "stub.h"

class OracleNativeCLiveMTaskTest : public testing::Test {
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
