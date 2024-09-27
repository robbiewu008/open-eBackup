#ifndef THRIFTSERVICETEST_H_
#define THRIFTSERVICETEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class ThriftServiceTest :  public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

class ThriftFactoryTest :  public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void ThriftServiceTest::SetUp() {}

void ThriftServiceTest::TearDown() {}

void ThriftServiceTest::SetUpTestCase() {}

void ThriftServiceTest::TearDownTestCase() {}


#endif