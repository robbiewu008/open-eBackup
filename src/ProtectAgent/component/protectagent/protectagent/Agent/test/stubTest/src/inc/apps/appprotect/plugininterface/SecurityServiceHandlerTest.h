#ifndef _SECURITY_SERVICE_TEST_HEADER_
#define _SECURITY_SERVICE_TEST_HEADER_
#define private public
#define protected public

#include "gtest/gtest.h"
#include "stub.h"

class SecurityServiceHandlerTest: public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void SecurityServiceHandlerTest::SetUp() {}

void SecurityServiceHandlerTest::TearDown() {}

void SecurityServiceHandlerTest::SetUpTestCase() {}

void SecurityServiceHandlerTest::TearDownTestCase() {}
#endif
