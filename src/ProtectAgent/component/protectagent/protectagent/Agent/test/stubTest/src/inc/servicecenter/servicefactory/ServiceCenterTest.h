#ifndef SERVICECENTERTEST_H_
#define SERVICECENTERTEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class ServiceCenterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void ServiceCenterTest::SetUp() {}

void ServiceCenterTest::TearDown() {}

void ServiceCenterTest::SetUpTestCase() {}

void ServiceCenterTest::TearDownTestCase() {}

#endif