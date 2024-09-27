#ifndef _UPGRADE_HANSLE_TEST_H_
#define _UPGRADE_HANSLE_TEST_H_

#include "cunitpub/publicInc.h"
#include "plugins/host/ModifyPluginHandle.h"

class CModifyPluginHandleTest: public testing::Test
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


#endif /* _ADD_CONTROLLER_TEST_H_; */