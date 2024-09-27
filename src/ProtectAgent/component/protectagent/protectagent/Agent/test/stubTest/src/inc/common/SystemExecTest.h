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
#ifndef _SYSTEMEXECTEST_H_
#define _SYSTEMEXECTEST_H_

#define private public

#ifndef WIN32
#include <signal.h>
#include <libgen.h>
#endif
#include <sstream>
#include "securec.h"
#include "common/CSystemExec.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "securecom/UniqueId.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "gtest/gtest.h"
#include "stub.h"

class CSystemExecTest: public testing::Test{
public:
    Stub stub;
};


//*******************************************************************************
mp_bool CSystemExecTestStubFileExist(const mp_string& pszFilePath){
    return 1;
}

mp_bool CSystemExecTestStubCheckCmdDelimiter(const mp_string& str){
    return 0;
}

mp_int32 CSystemExecStubExecSystemWithoutEcho(mp_string& strCommand, mp_bool bNeedRedirect){
    return 0;
}

#endif
