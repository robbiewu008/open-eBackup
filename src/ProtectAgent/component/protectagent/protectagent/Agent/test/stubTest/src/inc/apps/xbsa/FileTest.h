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
#ifndef _APPS_XBSA_FILETEST_H_
#define _APPS_XBSA_FILETEST_H_

#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "xbsaclientcomm/File.h"

class FileTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

inline void FileTest::SetUp() {}

inline void FileTest::TearDown() {}

inline void FileTest::SetUpTestCase() {}

inline void FileTest::TearDownTestCase() {}

#endif
