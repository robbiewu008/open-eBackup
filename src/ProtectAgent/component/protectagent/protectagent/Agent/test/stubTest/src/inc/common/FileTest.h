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
#ifndef __AGENT_FILE_TEST_H__
#define __AGENT_FILE_TEST_H__
#define private public
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/File.h"
#include "gtest/gtest.h"
 #include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "stub.h"


typedef mp_void (CLogger::*CLoggerLogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CMpFileTest: public testing::Test{
public:
    Stub stub;
};

class CIPCFileTest: public testing::Test{
public:
    Stub stub;
};


#endif

