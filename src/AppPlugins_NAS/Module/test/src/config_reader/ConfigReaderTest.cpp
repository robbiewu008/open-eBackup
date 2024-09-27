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
#include <stdio.h>
#include <iostream>
#include "config_reader/ConfigIniReader.h"
#include "config_reader/ConfigIniReaderImpl.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "llt_stub/stub.h"
#include "llt_stub/addr_pri.h"

TEST(ConfigReaderTEST, AdminNodeGetLeaderIPOK)
{
    EXPECT_STREQ("127.0.0.1", Module::ConfigReader::AdminNodeGetLeaderIP().c_str());
}

TEST(ConfigReaderTEST, getAdminNodeIAMUserOK)
{
    EXPECT_STREQ("", Module::ConfigReader::getAdminNodeIAMUser().c_str());
}


TEST(ConfigReaderTEST, getAdminNodeIAMPasswdOK)
{
    EXPECT_STREQ("", Module::ConfigReader::getAdminNodeIAMPasswd().c_str());
}

TEST(ConfigReaderTEST, getBackupNodeIAMUserOK)
{
    EXPECT_STREQ("", Module::ConfigReader::getBackupNodeIAMUser().c_str());
}

TEST(ConfigReaderTEST, getBackupNodeIAMPasswdOK)
{
    EXPECT_STREQ("", Module::ConfigReader::getBackupNodeIAMPasswd().c_str());
}

TEST(ConfigReaderTEST, GetPrimaryAndStandbyIPOK)
{
    EXPECT_STREQ("127.0.0.1", Module::ConfigReader::GetPrimaryAndStandbyIP().c_str());
}

TEST(ConfigReaderTEST, GetEbkMgrBindPortOK)
{
    EXPECT_STREQ("5570", Module::ConfigReader::GetEbkMgrBindPort().c_str());
}

