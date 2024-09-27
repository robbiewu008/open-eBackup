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
#ifndef _KMCCALLBACK_H_
#define _KMCCALLBACK_H_

#define private public

#include "securecom/KmcCallback.h"
#include "common/Utils.h"
#include "securecom/UniqueId.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/CSystemExec.h"
#include "common/ConfigXmlParse.h"
#include "securec.h"
#include "gtest/gtest.h"
#include "stub.h"
#include <vector>


class KmcCallbackTest: public testing::Test{
public:
    Stub stub;
};

#endif
