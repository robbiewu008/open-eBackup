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
#ifndef __AGENT_IP_TEST_H__
#define __AGENT_IP_TEST_H__

#define private public

#include "securecom/Ip.h"
#include "gtest/gtest.h"
 #include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/File.h"
#include "common/Log.h"
#include "stub.h"

class IPTest: public testing::Test {
public:
    Stub stub;
};

mp_int32 StubCheckHostLinkStatus(
    const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_SUCCESS;
}



#endif
