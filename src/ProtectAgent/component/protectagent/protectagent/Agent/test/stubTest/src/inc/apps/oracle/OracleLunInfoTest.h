#ifndef __ORACLELUNINFO_TEST_H__
#define __ORACLELUNINFO_TEST_H__

#define private public
using namespace std;
#include "apps/oracle/OracleLunInfo.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/MpString.h"
// #include "common/RootCaller.h"
#include "common/CSystemExec.h"
#include "common/File.h"
#include "array/array.h"
#include <sstream>
#include "gtest/gtest.h"
#include "stub.h"
#include <vector>
using namespace std;

class OracleLunInfoTest : public testing::Test
{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

// Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* COracleTest::m_stub;
void OracleLunInfoTest::SetUp() {}

void OracleLunInfoTest::TearDown() {}

void OracleLunInfoTest::SetUpTestCase() {}

void OracleLunInfoTest::TearDownTestCase() {}

#endif

