
#ifndef __DISK_DATA_INTERFACE_TEST_H__
#define __DISK_DATA_INTERFACE_TEST_H__

#include "dataprocess/datareadwrite/DiskStream.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class DiskStreamTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void DiskStreamTest::SetUp() {}

void DiskStreamTest::TearDown() {}

void DiskStreamTest::SetUpTestCase() {}

void DiskStreamTest::TearDownTestCase() {}

mp_int32 StubDiskStreamGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetDiskFdByName(const mp_string &diskName) {
    return 1;
}

#endif