/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * Description: XBSA database(sqlite) operation interface.
 * Create: 2021-06-10
 * Author: 
 */
#ifndef _BSA_TRANS_MANAGER_TEST_H_
#define _BSA_TRANS_MANAGER_TEST_H_
#define private public
#include "apps/dws/XBSAServer/BsaTransManager.h"
#include "gtest/gtest.h"
#include "stub.h"
using namespace std;

class BsaTransactionTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void BsaTransactionTest::SetUp() {}

void BsaTransactionTest::TearDown() {}

void BsaTransactionTest::SetUpTestCase() {}

void BsaTransactionTest::TearDownTestCase() {}

#endif