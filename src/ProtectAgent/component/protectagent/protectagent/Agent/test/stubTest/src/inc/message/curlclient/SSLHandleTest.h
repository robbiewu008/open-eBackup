/*******************************************************************************
Copyright (C), 1988-2014, Huawei Tech. Co., Ltd.
********************************************************************************
Filename                    : SSLHandle.h
Description                 : The implemention about SSL
Version                     : 1.0.0.0
Date                        : 6/12/2014
Author                      : lili 00254913
Function list               :
Updated Record              :
<Author>                             <Date>              <Version>         <Description>

Remark                      :
*********************************************************************************/
#ifndef _SSLHANDLETEST_H_
#define _SSLHANDLETEST_H_
#define private public
#include "message/curlclient/SSLHandle.h"
#include "gtest/gtest.h"
#include "stub.h"

class SSLHandleTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void SSLHandleTest::SetUp()
{}

void SSLHandleTest::TearDown()
{}

void SSLHandleTest::SetUpTestCase()
{}

void SSLHandleTest::TearDownTestCase()
{}
#endif

