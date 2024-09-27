#ifndef THRIFTPLUGINTEST_H_
#define THRIFTPLUGINTEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class ThriftPluginTest :  public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void ThriftPluginTest::SetUp() {}

void ThriftPluginTest::TearDown() {}

void ThriftPluginTest::SetUpTestCase() {}

void ThriftPluginTest::TearDownTestCase() {}

#endif