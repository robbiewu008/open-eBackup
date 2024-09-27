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
#include "securecom/CodeConvertTest.h"
#include <openssl/ssl.h>
#include "stub.h"
#include "common/Log.h"
#include "common/Types.h"

namespace {
const mp_int32 MAX_ENCODE_BASE64_LENGTH = 10*1024*1024; // 10MB

mp_int32 stubFailed(mp_void *pthis)
{
    return MP_FAILED;
}
}

static mp_void StubCLoggerLog(mp_void){
    return;
}
/*
* 用例名称：base64转换成功
* 前置条件：无
* check点：将字符转换后，再转换回来，字符不变
*/
TEST_F(CodeConvertTest, EncodeAndDecodeTest){
    stub.set(&CLogger::Log, StubCLoggerLog);

    mp_string EncodeBase64In = "123456";
    mp_string EncodeBase64Out;
    mp_string DecodeBase64Out;

    CodeConvert::EncodeBase64(EncodeBase64In.size() * 2, EncodeBase64In, EncodeBase64Out);
    CodeConvert::DecodeBase64(EncodeBase64Out.size() * 2, EncodeBase64Out, DecodeBase64Out);
    EXPECT_EQ(DecodeBase64Out, EncodeBase64In);

    mp_uint64 EncodeBase64InSize = MAX_ENCODE_BASE64_LENGTH + 1;
    mp_uint64 EncodeBase64OutSize = EncodeBase64InSize;
    mp_bool bRet = CodeConvert::EncodeBase64(EncodeBase64InSize, EncodeBase64In, EncodeBase64Out);
    mp_bool bRet2 = CodeConvert::DecodeBase64(EncodeBase64OutSize, EncodeBase64Out, DecodeBase64Out);
    EXPECT_EQ(bRet, false);
    EXPECT_EQ(bRet, bRet2);

    EncodeBase64InSize = 0;
    EncodeBase64OutSize = EncodeBase64InSize;
    bRet = CodeConvert::EncodeBase64(EncodeBase64InSize, EncodeBase64In, EncodeBase64Out);
    bRet2 = CodeConvert::DecodeBase64(EncodeBase64OutSize, EncodeBase64Out, DecodeBase64Out);
    EXPECT_EQ(bRet, false);
    EXPECT_EQ(bRet, bRet2);

    EncodeBase64In = "123456";
    stub.set(EVP_DecodeBlock, stubFailed);
    CodeConvert::EncodeBase64(EncodeBase64In.size() * 2, EncodeBase64In, EncodeBase64Out);
    bRet2 = CodeConvert::DecodeBase64(EncodeBase64Out.size() * 2, EncodeBase64Out, DecodeBase64Out);
    EXPECT_EQ(bRet2, false);
}
