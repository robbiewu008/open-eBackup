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
#include "tools/agentcli/UnixTimeStampTest.h"

using namespace std;

TEST_F(CUnixTimeStampTest, handleTest)
{
    mp_string timestring = "2021-07-13 15:27:58";
    mp_string transMode = "Date2Unix";

    UnixTimeStamp unixTimeObj;
    mp_int32 ret = unixTimeObj.Handle(timestring, transMode);
    EXPECT_EQ(MP_SUCCESS, ret);

    timestring = "1626422313";
    transMode = "Unix2Date";
    ret = unixTimeObj.Handle(timestring, transMode);
    EXPECT_EQ(MP_SUCCESS, ret);

    transMode = "";
    ret = unixTimeObj.Handle(timestring, transMode);
    EXPECT_EQ(MP_FAILED, ret);
}


TEST_F(CUnixTimeStampTest, DateTransforUnixStampTest)
{
    mp_string timeString = "";
    UnixTimeStamp unixTimeObj;
    mp_int32 ret = unixTimeObj.DateTransforUnixStamp(timeString);
    EXPECT_EQ(MP_FAILED, ret);

    timeString = "2021-07-13 15:27:58";
    ret = unixTimeObj.DateTransforUnixStamp(timeString);
    EXPECT_EQ(MP_SUCCESS, ret);

    timeString = "2021-07-13_15:27:58";
    ret = unixTimeObj.DateTransforUnixStamp(timeString);
    EXPECT_EQ(MP_FAILED, ret);

    timeString = "2021/07/13/15:27:58";
    ret = unixTimeObj.DateTransforUnixStamp(timeString);
    EXPECT_EQ(MP_FAILED, ret);

    timeString = "12345678";
    ret = unixTimeObj.DateTransforUnixStamp(timeString);
    EXPECT_EQ(MP_FAILED, ret);
}

TEST_F(CUnixTimeStampTest, UnixStampTranforDateTest)
{
    mp_string timeString = "1626422313";
    UnixTimeStamp unixTimeObj;
    mp_int32 ret = unixTimeObj.UnixStampTranforDate(timeString);
    EXPECT_EQ(MP_SUCCESS, ret);

    timeString = "1324abcdf";
    ret = unixTimeObj.UnixStampTranforDate(timeString);
    EXPECT_EQ(MP_FAILED, ret);

    timeString = "";
    ret = unixTimeObj.UnixStampTranforDate(timeString);
    EXPECT_EQ(MP_FAILED, ret);
}
