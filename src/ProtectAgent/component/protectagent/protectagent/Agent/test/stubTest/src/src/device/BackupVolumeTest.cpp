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
#include "device/BackupVolumeTest.h"

TEST_F(BackupVolumeTest,UnitTest){
    mp_int32 ret;
    mp_string disk;
    mp_string returnStr;

    BackupVolume work("test","test","test","test");

    returnStr = work.GetVolWwn();
    EXPECT_EQ(returnStr, "test");

    returnStr = work.GetIsExtend();
    EXPECT_EQ(returnStr, "test");

    returnStr = work.GetIsMetaData();
    EXPECT_EQ(returnStr, "test");

    returnStr = work.GetVolumeType();
    EXPECT_EQ(returnStr, "test");
}
