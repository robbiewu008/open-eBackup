#ifndef __AGENT_VMWARENATIVE_STORAGEDEVICEFACTORY_TEST_H__
#define __AGENT_VMWARENATIVE_STORAGEDEVICEFACTORY_TEST_H__

#include "dataprocess/vmwarenative/StorageDeviceFactory.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class StorageDeviceFactoryTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void StorageDeviceFactoryTest::SetUp() {}

void StorageDeviceFactoryTest::TearDown() {}

void StorageDeviceFactoryTest::SetUpTestCase() {}

void StorageDeviceFactoryTest::TearDownTestCase() {}

mp_int32 StubStorageDeviceFactoryGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#endif