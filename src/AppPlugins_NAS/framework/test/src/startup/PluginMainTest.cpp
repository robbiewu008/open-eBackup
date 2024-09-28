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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "PluginThriftServer.h"
#include "PluginThriftClientConfig.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

// using namespace AppProtect;

namespace {
    const int INNER_ERROR = 200;
}

class PluginMainTest : public testing::Test {
};

TEST_F(PluginMainTest, PluginThriftServerStart)
{
    std::string ip = "";
    uint32_t port = 0;
    startup::PluginThriftClientConfig::GetInstance().Configure(ip, port);
    PluginThriftServer::GetInstance().Configure(ip, port);
    bool ret = PluginThriftServer::GetInstance().Start();
    EXPECT_EQ(ret, false);
}
