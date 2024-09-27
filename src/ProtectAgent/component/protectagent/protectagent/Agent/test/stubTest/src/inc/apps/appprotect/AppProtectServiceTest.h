/******************************************************************************
    Copyright (C), 2001-2021, Huawei Tech. Co., Ltd.
    ******************************************************************************
    File Name     : AppProtectServiceTest.h
    Version       : Initial Draft
    Author        : lwx943115
    Created       :
    Last Modified :
    Description   : single instance, transfer task data
    History       :
******************************************************************************/
#ifndef _APP_PROTECT_SERVICE_TEST_H_
#define _APP_PROTECT_SERVICE_TEST_H_

#define private public
#define protected public

#include "apps/appprotect/AppProtectService.h"
#include "stub.h"
#include "gtest/gtest.h"
#include "common/Log.h"

class AppProtectServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

#endif
