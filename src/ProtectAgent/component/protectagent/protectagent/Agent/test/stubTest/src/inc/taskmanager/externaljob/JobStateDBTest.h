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