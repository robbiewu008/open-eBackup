/* 
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rightsOracleNativeDismountTaskTest reserved. 
 * Description: no on-premies database live moun task 
 * Author: wangguitao
 * Create: 2020-02-04 
 * Notes: database livemount task
 */
#ifndef _AGENT_ORACLE_NATIVE_LIVEMOUNT_TASK_TEST_
#define _AGENT_ORACLE_NATIVE_LIVEMOUNT_TASK_TEST_
#define private public
#define protected public
#include "apps/oraclenative/OracleNativeLiveMTask.h"
#include "cunitpub/publicInc.h"
#include "gtest/gtest.h"
#include "stub.h"

class OracleNativeLiveMTaskTest : public testing::Test {
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
