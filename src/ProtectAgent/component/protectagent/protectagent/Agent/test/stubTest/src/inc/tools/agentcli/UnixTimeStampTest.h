#ifndef _UNIXTIMESTAMP_TEST_H_
#define _UNIXTIMESTAMP_TEST_H_

#include "cunitpub/publicInc.h"
#include "tools/agentcli/UnixTimeStamp.h"


class CUnixTimeStampTest : public testing::Test
{
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
