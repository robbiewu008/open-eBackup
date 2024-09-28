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
#include "dataprocess/DataContextTest.h"

TEST_F(DataContextTest, GetSockFd) {
    mp_socket sockFd = 1234;
    DataContext om;
    om.SetSockFd(sockFd);
    mp_int32 iRet = sockFd == om.GetSockFd() ? 1 : 0;
    EXPECT_EQ(1, iRet);
}

TEST_F(DataContextTest, GetDiskFdByName) {
    DataContext om;
    om.SetDiskFdByName("test", 1234);
    mp_int32 iRet = om.GetDiskFdByName("test");
    EXPECT_NE(MP_FAILED, iRet);
}
