#ifndef _AGENT_ORACLE_NATIVE_RESTORE_TASK_TEST_
#define _AGENT_ORACLE_NATIVE_RESTORE_TASK_TEST_
#define private public
#define protected public
#include "apps/oraclenative/OracleNativeRestoreTask.h"
#include "cunitpub/publicInc.h"
#include "gtest/gtest.h"
#include "stub.h"

class OracleNativeRestoreTaskTest : public testing::Test {
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
