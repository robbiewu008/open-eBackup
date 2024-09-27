#ifndef _AGENT_CCHGNGXPWD_TEST_
#define _AGENT_CCHGNGXPWD_TEST_

#include "cunitpub/publicInc.h"
#include "tools/agentcli/ChgNgxPwd.h"
#include "tools/agentcli/ChgHostSN.h"

class CChgNgxPwdTest: public testing::Test
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

class ChgHostSNTest : public testing::Test
{
};

#endif /* _AGENT_CCHGNGXPWD_TEST_; */

