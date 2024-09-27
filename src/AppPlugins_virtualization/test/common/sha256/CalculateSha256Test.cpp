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
#include <list>
#include <cstdio>
#include <securec.h>
#include "gtest/gtest.h"
#include "openssl/sha.h"
#include "stub.h"
#include "define/Types.h"
#include "common/Constants.h"
#include "common/sha256/Sha256.h"

using namespace VirtPlugin;
namespace {
const int32_t MAX_BYTE_SHA256 = 32;
}

namespace HDT_TEST {
class CalculateSha256Test : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};  

static int32_t Stub_SHA256_Init(SHA256_CTX* shctx)
{
    return FAILED;
}

static int32_t Stub_SHA256_Update(SHA256_CTX* shctx, const void* pobj, size_t len)
{
    return FAILED;
}

static int32_t Stub_SHA256_Final(unsigned char *md, SHA256_CTX *c)
{
    return FAILED;
}
/*
 * 测试用例：计算SHA256值
 * 前置条件：初始化SHA256结构体失败
 * CHECK点：计算SHA256值失败
 */
TEST_F(CalculateSha256Test, CalculateSha256_Failed1)
{
    std::shared_ptr<uint8_t[]> shabuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(shabuf.get(), DIRTY_RANGE_BLOCK_SIZE, 0, DIRTY_RANGE_BLOCK_SIZE);
    std::shared_ptr<uint8_t[]> outbuf = std::make_unique<uint8_t[]>(MAX_BYTE_SHA256);
    memset_s(outbuf.get(), MAX_BYTE_SHA256, 0, MAX_BYTE_SHA256);
    Stub stub;
    stub.set(SHA256_Init, Stub_SHA256_Init);
    int ret = CalculateSha256::CalculateSha256Value(shabuf, DIRTY_RANGE_BLOCK_SIZE, outbuf);
    EXPECT_EQ(FAILED, ret);
    stub.reset(SHA256_Init);
}

/*
 * 测试用例：计算SHA256值
 * 前置条件：更新SHA256结构体失败
 * CHECK点：计算SHA256值失败
 */
TEST_F(CalculateSha256Test, CalculateSha256_Failed2)
{
    std::shared_ptr<uint8_t[]> shabuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(shabuf.get(), DIRTY_RANGE_BLOCK_SIZE, 0, DIRTY_RANGE_BLOCK_SIZE);
    std::shared_ptr<uint8_t[]> outbuf = std::make_unique<uint8_t[]>(MAX_BYTE_SHA256);
    memset_s(outbuf.get(), MAX_BYTE_SHA256, 0, MAX_BYTE_SHA256);
    Stub stub;
    stub.set(SHA256_Update, Stub_SHA256_Update);
    int ret = CalculateSha256::CalculateSha256Value(shabuf, DIRTY_RANGE_BLOCK_SIZE, outbuf);
    EXPECT_EQ(FAILED, ret);
    stub.reset(SHA256_Update);
}

/*
 * 测试用例：计算SHA256值
 * 前置条件：更新SHA256结构体失败
 * CHECK点：计算SHA256值失败
 */
TEST_F(CalculateSha256Test, CalculateSha256_Failed3)
{
    std::shared_ptr<uint8_t[]> shabuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(shabuf.get(), DIRTY_RANGE_BLOCK_SIZE, 0, DIRTY_RANGE_BLOCK_SIZE);
    std::shared_ptr<uint8_t[]> outbuf = std::make_unique<uint8_t[]>(MAX_BYTE_SHA256);
    memset_s(outbuf.get(), MAX_BYTE_SHA256, 0, MAX_BYTE_SHA256);
    Stub stub;
    stub.set(SHA256_Final, Stub_SHA256_Final);
    int ret = CalculateSha256::CalculateSha256Value(shabuf, DIRTY_RANGE_BLOCK_SIZE, outbuf);
    EXPECT_EQ(FAILED, ret);
    stub.reset(SHA256_Update);
}

/*
 * 测试用例：计算SHA256值
 * 前置条件：成功初始化SHA256结构体
 * CHECK点：计算SHA256值成功
 */
TEST_F(CalculateSha256Test, CalculateSha256_SUCCESS)
{
    std::shared_ptr<uint8_t[]> shabuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(shabuf.get(), DIRTY_RANGE_BLOCK_SIZE, 0, DIRTY_RANGE_BLOCK_SIZE);
    std::shared_ptr<uint8_t[]> outbuf = std::make_unique<uint8_t[]>(MAX_BYTE_SHA256);
    memset_s(outbuf.get(), MAX_BYTE_SHA256, 0, MAX_BYTE_SHA256);
    Stub stub;
    int ret = CalculateSha256::CalculateSha256Value(shabuf, DIRTY_RANGE_BLOCK_SIZE, outbuf);
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：计算SHA256文件偏移量
 * 前置条件：计算偏移量成功
 * CHECK点：计算SHA256文件偏移量
 */
TEST_F(CalculateSha256Test, CalculateSha256Deviation_SUCCESS)
{
    const uint64_t blockIndex = 108;
    uint64_t inDev = blockIndex * DIRTY_RANGE_BLOCK_SIZE;
    uint64_t outDev = 0;
    CalculateSha256::CalculateSha256Deviation(inDev, outDev);

    EXPECT_EQ(outDev, blockIndex * MAX_BYTE_SHA256);
}
}