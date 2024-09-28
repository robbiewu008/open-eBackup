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
#include <iostream>
#include <string>
#include <openssl/ssl.h>
#include "gtest/gtest.h"
#include "stub.h"
#include "curl_http/CodeConvert.h"

namespace CurlHttpTest {
class CodeConvertTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CodeConvertTest::SetUp() {}

void CodeConvertTest::TearDown() {}

void CodeConvertTest::SetUpTestCase() {}

void CodeConvertTest::TearDownTestCase() {}

TEST_F(CodeConvertTest, ConvertBinary2ASCII_Success)
{
    std::string targetStr = "QUJDREVmZ2hpaktMTU5Pb3Byc3RVVldYWXowMTIzNDU2Nzg5";
    std::string strInput = "ABCDEfghijKLMNOoprstUVWXYz0123456789";
    std::string strOutput;

    Module::CodeConvert::ConvertBinary2ASCII(strInput, strOutput);
    EXPECT_EQ(targetStr, strOutput);
}

TEST_F(CodeConvertTest, ConvertASCII2Binary_Success)
{
    std::string targetStr = "ABCDEfghijKLMNOoprstUVWXYz0123456789";;
    std::string strInput = "QUJDREVmZ2hpaktMTU5Pb3Byc3RVVldYWXowMTIzNDU2Nzg5";
    std::string strOutput;

    Module::CodeConvert::ConvertASCII2Binary(strInput, strOutput);
    EXPECT_EQ(targetStr, strOutput);
}

TEST_F(CodeConvertTest, EncodeBase64_Success)
{
    std::string targetStr = "QUJDREVmZ2hpaktMTU5Pb3Byc3RVVldYWXowMTIzNDU2Nzg5";
    std::string strInput = "ABCDEfghijKLMNOoprstUVWXYz0123456789";
    std::string strOutput;

    Module::CodeConvert::EncodeBase64(strInput.length() * 2, strInput, strOutput);
    EXPECT_EQ(targetStr, strOutput);
}

TEST_F(CodeConvertTest, DecodeBase64_Success)
{
    std::string targetStr = "ABCDEfghijKLMNOoprstUVWXYz0123456789";
    std::string strInput = "QUJDREVmZ2hpaktMTU5Pb3Byc3RVVldYWXowMTIzNDU2Nzg5";
    std::string strOutput;

    Module::CodeConvert::DecodeBase64(strInput.length(), strInput, strOutput);
    EXPECT_EQ(targetStr, strOutput);
}
}
