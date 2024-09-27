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
#ifndef __GET_FILE_CLIENT_2_ARCHIVE_H__
#define __GET_FILE_CLIENT_2_ARCHIVE_H__

#include <sstream>
#include <vector>
#include <map>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/CMpTime.h"
#include "common/DB.h"
#include "common/Path.h"
#include "securecom/CryptAlg.h"
#include "common/ConfigXmlParse.h"
#include "message/archivestream/ArchiveStreamClientHandler.h"
#include "common/Uuid.h"
#include "gtest/gtest.h"
#include "stub.h"
#include <openssl/ssl.h>

class TestGetFileClient2Archive : public testing::Test {
public:
    void SetUp()
    {
    }

    void TearDown()
    {
    }

    static void SetUpTestCase()
    {
    }

    static void TearDownTestCase()
    {
    }
    static int times;
};

#endif