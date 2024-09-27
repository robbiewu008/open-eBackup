#ifndef _AGENT_APP_PLUGIN_TEST_H_
#define _AGENT_APP_PLUGIN_TEST_H_

#include "cunitpub/publicInc.h"
#include "plugins/app/AppPlugin.h"
#include "common/JsonUtils.h"
class CAppPluginTest: public testing::Test
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


#endif /* _AGENT_APP_PLUGIN_TEST_H_; */

