#ifndef _AGENT_CHANGE_IP_TEST_H_
#define _AGENT_CHANGE_IP_TEST_H_

#include "cunitpub/publicInc.h"
#include "tools/agentcli/ChangeIP.h"
#include "tools/agentcli/TestHost.h"

class CChangeIpTest: public testing::Test
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

class TestHostTest: public testing::Test
{
};

#endif /* _AGENT_CHANGE_IP_TEST_H_; */

