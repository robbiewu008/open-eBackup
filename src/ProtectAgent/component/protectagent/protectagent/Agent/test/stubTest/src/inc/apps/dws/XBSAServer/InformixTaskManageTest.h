/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: XBSA informix.
 * Create: 2023-11-07
 * Author: 
 */
#ifndef _DWS_TASK_MANAGER_TEST_H_
#define _DWS_TASK_MANAGER_TEST_H_
#define private public
#include "apps/dws/XBSAServer/InformixTaskManage.h"
#include "gtest/gtest.h"
#include "stub.h"
using namespace std;

class InformixTaskManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void InformixTaskManagerTest::SetUp() {}

void InformixTaskManagerTest::TearDown() {}

void InformixTaskManagerTest::SetUpTestCase() {}

void InformixTaskManagerTest::TearDownTestCase() {}

#endif