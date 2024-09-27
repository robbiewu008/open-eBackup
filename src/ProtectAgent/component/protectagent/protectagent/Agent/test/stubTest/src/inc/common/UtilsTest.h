#ifndef _UTILSTEST_H_
#define _UTILSTEST_H_

#ifndef WIN32
#include <signal.h>
#include <libgen.h>
#endif
#include <sstream>
#include "securec.h"
#include "gtest/gtest.h"
#include "stub.h"

class UtilsTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void UtilsTest::SetUp() {}

void UtilsTest::TearDown() {}

void UtilsTest::SetUpTestCase() {}

void UtilsTest::TearDownTestCase() {}

#endif
