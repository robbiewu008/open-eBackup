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
#ifndef _AGENT_INIT_GLOBAL_
#define _AGENT_INIT_GLOBAL_

#include <sstream>

#include <unistd.h>
#include "gtest/gtest.h"

#define private public

#include "stub.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "common/CSystemExec.h"
#include "securecom/Password.h"
#include "common/ConfigXmlParse.h"
#include "message/rest/interfaces.h"

typedef mp_void (*pOrgGetInput)(mp_string strHint, mp_string& strInput, mp_int32 iInputLen);
typedef mp_void (*StubGetInputType)(mp_string strHint, mp_string &strInput);
typedef mp_int32 (*pStubIntType)(mp_void);
typedef mp_bool (*pStubBoolType)(mp_void);
typedef mp_string (*pStubStringType)(mp_void);
typedef mp_string (*pStubCstringType)(mp_void);
typedef mp_void (*pStubVoidType)(mp_void);
typedef mp_int32 (*pStubRetType)(mp_void);


template<typename inputType>
inline mp_string toString(inputType val)
{
    mp_string strval; 
    std::ostringstream oss;
    
    oss << val;
    strval = oss.str();
    
    return strval;
}

mp_void stub_set_cpasswdString(const mp_string& strHint, mp_string& strInput);
mp_void stub_set_cpasswdLongString(const mp_string& strHint, mp_string& strInput);
static mp_int32 StubGetJsonArrayJson(const Json::Value& jsValue, std::vector<Json::Value>& vecValue);
mp_void stub_set_string(mp_string &strInput);
mp_void stub_set_numberStr(mp_string &strInput);

mp_bool stub_return_bool(mp_void);
mp_bool stub_return_bool_true(void);

mp_int32 stub_return_ret(mp_void);
mp_int32 stub_return_number(mp_void);
mp_string stub_return_string(mp_void);
mp_string stub_return_cstring(mp_void);
mp_void stub_return_nothing(mp_void);
mp_bool stub_return_success(mp_void);

mp_void reset_cunit_counter(mp_void);
mp_void init_cunit_data();
mp_void destroy_cunit_data();

#endif /* _AGENT_INIT_GLOBAL_; */



