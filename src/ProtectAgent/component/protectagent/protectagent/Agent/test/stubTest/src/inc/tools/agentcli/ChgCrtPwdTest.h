#ifndef _CHG_CRT_PWD_TEST_H_
#define _CHG_CRT_PWD_TEST_H_

#include "cunitpub/publicInc.h"
#include "tools/agentcli/ChgCrtPwd.h"

class CChgCrtPwdTest: public testing::Test
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


#endif /* _CHG_CRT_PWD_TEST_H_; */

