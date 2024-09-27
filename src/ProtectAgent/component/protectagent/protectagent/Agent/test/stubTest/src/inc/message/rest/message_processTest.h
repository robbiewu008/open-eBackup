/******************************************************************************
Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.
File Name     : http_cgi.h
Version       : Initial Draft
Author        : xuchong 00300551
Created       : 2021/3/29
Description   : 处理CGI接口消息
******************************************************************************/
#ifndef _AGENT_MESSAGE_PROCESS_TEST_H_
#define _AGENT_MESSAGE_PROCESS_TEST_H_

#include <map>
#include <stdlib.h>
#define private public
#include "stub.h"
#include "common/Types.h"
#include "common/Defines.h"  // macro DLLAPI is defined in Defines.h
#include "fcgi/include/fcgiapp.h"
#include "gtest/gtest.h"


class CRequestMsgTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};


void CRequestMsgTest::SetUp() {}

void CRequestMsgTest::TearDown() {}

void CRequestMsgTest::SetUpTestCase() {}

void CRequestMsgTest::TearDownTestCase() {}

#endif // _AGENT_HTTP_CGI_H_