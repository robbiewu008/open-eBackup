#ifndef _APPS_XBSA_XBSATEST_H_
#define _APPS_XBSA_XBSATEST_H_

#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "xbsa/xbsa.h"
#include "xbsaclientcomm/ThriftClientMgr.h"

class XbsaTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

inline void XbsaTest::SetUp() {}

inline void XbsaTest::TearDown() {}

inline void XbsaTest::SetUpTestCase() {}

inline void XbsaTest::TearDownTestCase() {}

#endif
