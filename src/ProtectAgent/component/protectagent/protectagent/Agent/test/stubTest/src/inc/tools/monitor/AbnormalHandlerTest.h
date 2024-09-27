#ifndef _AGENT_ABNORMALHANDLER_TEST_H_
#define _AGENT_ABNORMALHANDLER_TEST_H_
#define private public

#include "rootexec/SystemCallTest.h"
#include "tools/monitor/AbnormalHandler.h"

static Stub *gp_chmod = NULL;
class CAbnormalHandlerTest: public testing::Test
{
public:
    static void SetUpTestCase(void)
    {
        init_cunit_data();
        gp_chmod = new Stub;
        gp_chmod->set(chmod, stub_return_nothing);
    }

    static void TearDownTestCase(void)
    { 
        destroy_cunit_data();
        if (gp_chmod) delete gp_chmod;
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


#endif /* _AGENT_ABNORMALHANDLER_TEST_H_; */

