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
#ifndef _AGENT_MESSAGE_HANDLER_TEST_H__
#define _AGENT_MESSAGE_HANDLER_TEST_H__

#define private public
#define protected public
#include "message/tcp/MessageHandler.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"
class MessageHandlerTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void MessageHandlerTest::SetUp() {}

void MessageHandlerTest::TearDown() {}

void MessageHandlerTest::SetUpTestCase() {}

void MessageHandlerTest::TearDownTestCase() {}

mp_int32 StubMessageHandlerGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#endif