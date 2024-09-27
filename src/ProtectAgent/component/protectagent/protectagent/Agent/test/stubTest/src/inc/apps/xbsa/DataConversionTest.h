#ifndef _APPS_XBSA_DATACONVERSIONTEST_H_
#define _APPS_XBSA_DATACONVERSIONTEST_H_

#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "xbsaclientcomm/DataConversion.h"

class DataConversionTest : public testing::Test{
public:
    Stub stub;
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

inline void DataConversionTest::SetUp() {}

inline void DataConversionTest::TearDown() {}

inline void DataConversionTest::SetUpTestCase() {}

inline void DataConversionTest::TearDownTestCase() {}

#endif
