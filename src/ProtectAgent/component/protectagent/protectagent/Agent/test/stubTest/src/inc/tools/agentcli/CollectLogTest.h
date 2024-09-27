#ifndef _AGENT_COLLECT_LOG_TEST_H_
#define _AGENT_COLLECT_LOG_TEST_H_

#include "cunitpub/publicInc.h"
#include "tools/agentcli/CollectLog.h"
#include "common/Utils.h"

class CCollectLogTest: public testing::Test
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


#endif /* _AGENT_COLLECT_LOG_TEST_H_; */


