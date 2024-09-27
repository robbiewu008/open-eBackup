#ifndef __AGENT_VMWARENATIVE_DOUBLEQUEUE_CACHE_TEST_H__
#define __AGENT_VMWARENATIVE_DOUBLEQUEUE_CACHE_TEST_H__

#define private public
#include "dataprocess/vmwarenative/DoubleQueue.h"
#include "dataprocess/vmwarenative/Define.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"
class DoubleQueueTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void DoubleQueueTest::SetUp() {}

void DoubleQueueTest::TearDown() {}

void DoubleQueueTest::SetUpTestCase() {}

void DoubleQueueTest::TearDownTestCase() {}

mp_int32 StubFileDoubleQueueGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

VMWARE_DISK_RET_CODE StubWrite(void* obj,
    const uint64_t &offsetInBytes, uint64_t &bufferSizeInBytes, const unsigned char *buffer, std::string &errDesc)
{
    VMwareDiskApi* o = (VMwareDiskApi*)obj;
    return MP_FAILED;
}

#endif