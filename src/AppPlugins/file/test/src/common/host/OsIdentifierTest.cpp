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
#include "named_stub.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "gmock/gmock-actions.h"
#include "host/OsIdentifier.h"

using namespace std;
using namespace FilePlugin;

/*
 * 用例名称: CheckSystemInfoSuccess
 * 前置条件：
 * check点：获取系统信息成功
 */
TEST(OsIdentifierTest, CheckSystemInfoSuccess) 
{
    EXPECT_NO_THROW(OsIdentifier::CreateSystemNameRecordFile("/tmp"));
    EXPECT_NO_THROW(OsIdentifier::CreateSystemVersionRecordFile("/tmp"));
    EXPECT_NO_THROW(OsIdentifier::ReadSystemName("/tmp"));
    EXPECT_NO_THROW(OsIdentifier::ReadSystemVersion("/tmp"));
}
