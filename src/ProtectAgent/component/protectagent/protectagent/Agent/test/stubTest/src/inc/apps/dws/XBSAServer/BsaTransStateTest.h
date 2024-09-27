/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * Description: XBSA database(sqlite) operation interface.
 * Create: 2021-06-10
 * Author: 
 */
#ifndef _BSA_TRANS_STATE_TEST_H_
#define _BSA_TRANS_STATE_TEST_H_
#define private public
#include "apps/dws/XBSAServer/BsaTransManager.h"
#include "gtest/gtest.h"
#include "stub.h"
using namespace std;

class BsaTransStateBaseTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void BsaTransStateBaseTest::SetUp() {}

void BsaTransStateBaseTest::TearDown() {}

void BsaTransStateBaseTest::SetUpTestCase() {}

void BsaTransStateBaseTest::TearDownTestCase() {}

#endif