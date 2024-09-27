/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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

