/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * Description: XBSA database(sqlite) operation interface.
 * Create: 2021-06-10
 * Author: 
 */
#ifndef _BSA_DB_TEST_H_
#define _BSA_DB_TEST_H_
#define private public
#include "apps/dws/XBSAServer/CTimer.h"
#include "gtest/gtest.h"
#include "stub.h"
using namespace std;

class CTimerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void CTimerTest::SetUp() {}

void CTimerTest::TearDown() {}

void CTimerTest::SetUpTestCase() {}

void CTimerTest::TearDownTestCase() {}

#endif