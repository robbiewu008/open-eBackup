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
#ifndef _ROOTCALLER_H_
#define _ROOTCALLER_H_

#define private public

#include "common/RootCaller.h"
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

using namespace std;

class CRootCallerTest: public testing::Test{
public:
    Stub stub;
};

//*******************************************************************************
mp_int32 StubWriteInput(mp_string& strUniqueID, mp_string& strInput){
    return -1;
}

mp_int32 StubReadResult(mp_string& strUniqueID, vector<mp_string>& vecRlt){
    return -1;
}

static mp_int32 StubExecSystemWithoutEcho(mp_string strLogCmd, mp_string& strCommand, mp_bool bNeedRedirect){
    return 0;
}

mp_int32 StubRootCallerResultSuccess(mp_void){
    return MP_SUCCESS;
}

mp_int32 StubRootCallerResultFailed(mp_void){
    return MP_FAILED;
}

#endif
