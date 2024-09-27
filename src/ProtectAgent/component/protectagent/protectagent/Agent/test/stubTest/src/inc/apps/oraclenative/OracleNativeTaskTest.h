#ifndef _AGENT_ORACLE_NATIVE_TASK_TEST_
#define _AGENT_ORACLE_NATIVE_TASK_TEST_
#define protected public

#include "cunitpub/publicInc.h"
#include "apps/oraclenative/OracleNativeTask.h"
#include "gtest/gtest.h"
#include "stub.h"

class OracleNativeTaskTest : public testing::Test {
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
