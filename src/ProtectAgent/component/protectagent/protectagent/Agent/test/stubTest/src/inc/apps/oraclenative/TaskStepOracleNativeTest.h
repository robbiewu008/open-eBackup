/*
 * Copyright (C), 2001-2020, Huawei Tech. Co., Ltd.
 * Description: oracle native backup base class
 * Author: wangguitao w00510599
 * Create: 2020-01-27
 * History:
 *   2020-01-27 initial
 */
#ifndef _AGENT_BACKUP_STEP_TESTORACLENATIVE_
#define _AGENT_BACKUP_STEP_TESTORACLENATIVE_
#define protected public
#include "apps/oraclenative/TaskStepOracleNative.h"
#include "cunitpub/publicInc.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeTest : public testing::Test {
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
