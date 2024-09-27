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
#include <iostream>
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "FSBackupUtils.h"

using namespace std;

const uint64_t NUM1023 = 1023;
class FSBackupUtilsTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void FSBackupUtilsTest::SetUp()
{}

void FSBackupUtilsTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void FSBackupUtilsTest::SetUpTestCase()
{}

void FSBackupUtilsTest::TearDownTestCase()
{}

TEST_F(FSBackupUtilsTest, RemoveLeadingSlashes)
{
    string str = "/";
    FSBackupUtils::RemoveLeadingSlashes(str);
}

TEST_F(FSBackupUtilsTest, RemoveTrailingSlashes)
{
    string str = "./";
    FSBackupUtils::RemoveTrailingSlashes(str);
}

TEST_F(FSBackupUtilsTest, GenerateRandomStr)
{
    FSBackupUtils::GenerateRandomStr();
}

TEST_F(FSBackupUtilsTest, GetCurrentTime)
{
    FSBackupUtils::GetCurrentTime();
}

TEST_F(FSBackupUtilsTest, GetMilliSecond)
{
    FSBackupUtils::GetMilliSecond();
}

TEST_F(FSBackupUtilsTest, FormatSpeed)
{
    uint64_t speed = NUM1023 * MB_SIZE;
    FSBackupUtils::FormatSpeed(speed);

    speed = NUM1023 * GB_SIZE;
    FSBackupUtils::FormatSpeed(speed);

    speed = NUM1023 * TB_SIZE;
    FSBackupUtils::FormatSpeed(speed);

    speed = NUM1023 * PB_SIZE;
    FSBackupUtils::FormatSpeed(speed);

    speed = NUM1023 * KB_SIZE;
    FSBackupUtils::FormatSpeed(speed);

    speed = NUM1024 * TB_SIZE;
    FSBackupUtils::FormatSpeed(speed);
}

TEST_F(FSBackupUtilsTest, GetDateTimeString)
{
    uint64_t timeValue = 10;
    FSBackupUtils::GetDateTimeString(timeValue);
}

TEST_F(FSBackupUtilsTest, GetUmask)
{
    FSBackupUtils::GetUmask();
}

TEST_F(FSBackupUtilsTest, CheckMetaFileVersion)
{
    string dir = ".";
    FSBackupUtils::CheckMetaFileVersion(dir);
    FSBackupUtils::CheckMetaFileVersion(dir);
}