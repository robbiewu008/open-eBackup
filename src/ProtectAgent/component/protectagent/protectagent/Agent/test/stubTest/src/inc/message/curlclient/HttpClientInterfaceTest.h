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
#ifndef HTTP_CLIENT_INTERFACE_H
#define HTTP_CLIENT_INTERFACE_H
#include "curlclient/HttpClientInterface.h"
#include "gtest/gtest.h"
#include "stub.h"

class HttpClientInterfaceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void HttpClientInterfaceTest::SetUp()
{}

void HttpClientInterfaceTest::TearDown()
{}

void HttpClientInterfaceTest::SetUpTestCase()
{}

void HttpClientInterfaceTest::TearDownTestCase()
{}

#endif//HTTP_CLIENT_INTERFACE_H
