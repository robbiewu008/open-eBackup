/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * Description: XBSA database(sqlite) operation interface.
 * Create: 2021-06-10
 * Author: 
 */
#ifndef _DWS_TASK_MANAGER_TEST_H_
#define _DWS_TASK_MANAGER_TEST_H_
#define private public
#include "apps/dws/XBSAServer/DwsTaskManage.h"
#include "gtest/gtest.h"
#include "stub.h"
using namespace std;

class DwsTaskManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void DwsTaskManagerTest::SetUp() {}

void DwsTaskManagerTest::TearDown() {}

void DwsTaskManagerTest::SetUpTestCase() {}

void DwsTaskManagerTest::TearDownTestCase() {}

#endif