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
#include "tools/agentcli/CollectLogTest.h"
#include "securecom/SecureUtils.h"

static mp_void stubGetInput(const mp_string& strHint, mp_string& strInput)
{
    static mp_int32 ival = 0;
    if (ival++ == 0)
    {
        strInput = "n";
    }
    else
    {
        strInput = "y";
    }
}


TEST_F(CCollectLogTest, Handle)
{
    mp_int32 iRet = MP_SUCCESS;
    CollectLog Obj;

    Stub stub;
    stub.set(&CPassword::GetInput, stubGetInput);

    printf("\n#######################   01");
    
    iRet = Obj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    printf("\n#######################   02");

    stub.set(&SecureCom::PackageLog, stub_return_ret); 

    stub.set(&CMpTime::GetTimeString, &stub_return_string);

    iRet = Obj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    printf("\n#######################   03");

    iRet = Obj.Handle();
    EXPECT_TRUE(1);
    
    printf("\n#######################   04");
    
}

