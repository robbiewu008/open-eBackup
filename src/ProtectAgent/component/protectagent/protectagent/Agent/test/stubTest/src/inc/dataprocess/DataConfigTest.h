#ifndef __DATACONFIGTEST_H__
#define __DATACONFIGTEST_H__
#define private public
#include "dataprocess/dataconfig/DataConfig.h"
#include "tinyxml2.h"
#include "common/Types.h"
#include "common/TimeOut.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class CDataConfigTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void CDataConfigTest::SetUp() {}

void CDataConfigTest::TearDown() {}

void CDataConfigTest::SetUpTestCase() {}

void CDataConfigTest::TearDownTestCase() {}

mp_int32 StubGetValueInt32Success(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32Fail(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_FAILED;
}

mp_int32 StubGetValueStringSuccess(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueStringFail(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}

mp_int32 StubSetValueSuccess(const mp_string& strSection, const mp_string& strKey, mp_string strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubSetValueFail(const mp_string& strSection, const mp_string& strKey, mp_string strValue)
{
    return MP_FAILED;
}

#endif
