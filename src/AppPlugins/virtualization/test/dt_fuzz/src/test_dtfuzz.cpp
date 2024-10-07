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
#include "secodeFuzz.h"
#include <stdio.h>
#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

int main(int argc, char** argv)
{
    DT_Set_Report_Path((char *)"./logs/"); //设置报告输出路径
    ::testing::GTEST_FLAG(output) = "xml:./report/Virtual_report.xml";
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

