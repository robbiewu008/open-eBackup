#ifndef __AGENT_VMWARENATIVE_BLOCKSTORAGEDEVICE_TEST_H__
#define __AGENT_VMWARENATIVE_BLOCKSTORAGEDEVICE_TEST_H__

#define private public
#include "dataprocess/vmwarenative/BlockStorageDevice.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"
class BlockStorageDeviceTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void BlockStorageDeviceTest::SetUp() {}

void BlockStorageDeviceTest::TearDown() {}

void BlockStorageDeviceTest::SetUpTestCase() {}

void BlockStorageDeviceTest::TearDownTestCase() {}

mp_int32 StubFileBlockStorageDeviceGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

bool StubDeQueueToDataLun(FILE *file, int done, int &condition)
{
    return true;
}

mp_uint64 StubGetNumberOfDataBlockCompleted()
{
    return 0;
}

#endif