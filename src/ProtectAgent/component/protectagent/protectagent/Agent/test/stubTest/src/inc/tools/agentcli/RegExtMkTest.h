#ifndef _AGENT_REG_EXT_MK_TEST_H_
#define _AGENT_REG_EXT_MK_TEST_H_

#include "cunitpub/publicInc.h"
#include "tools/agentcli/RegExtMk.h"

class CRegExtMkTest: public testing::Test
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