/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * Description: XBSA database(sqlite) operation interface.
 * Create: 2021-06-10
 * Author: 
 */
#ifndef _BSA_INTF_ADAPTOR_TEST_H_
#define _BSA_INTF_ADAPTOR_TEST_H_
#include "apps/dws/XBSAServer/BsaIntfAdaptor.h"
#include "gtest/gtest.h"
#include "stub.h"
using namespace std;

class BsaIntfAdaptorTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void BsaIntfAdaptorTest::SetUp() {}

void BsaIntfAdaptorTest::TearDown() {}

void BsaIntfAdaptorTest::SetUpTestCase() {}

void BsaIntfAdaptorTest::TearDownTestCase() {}

#endif