/******************************************************************************
    Copyright (C), 2001-2021, Huawei Tech. Co., Ltd.
    ******************************************************************************
    File Name     : AppProtectJobHandlerTest.h
    Version       : Initial Draft
    Author        : w00510599
    Created       :
    Last Modified :
    Description   : single instance, transfer task data
    History       :
******************************************************************************/
#ifndef _APP_PROTECT_JOB_HANDLER_TEST_H_
#define _APP_PROTECT_JOB_HANDLER_TEST_H_

#define private public
#define protected public

#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/Log.h"
#include "taskmanager/externaljob/JobStateDB.h"
#include "apps/appprotect/CommonDef.h"
#include <message/curlclient/DmeRestClient.h>
#include "taskmanager/externaljob/AppProtectJobHandler.h"

class AppProtectJobHandlerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    Json::Value m_root;
};

#endif