#ifndef _AGENT_CLI_TEST_H_
#define _AGENT_CLI_TEST_H_

#include "cunitpub/publicInc.h"
#include "tools/agentcli/ShowStatus.h"

class CShowStatusTest: public testing::Test
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



#endif /* _AGENT_CLI_TEST_H_; */

