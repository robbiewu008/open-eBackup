#ifndef __AGENT_VMWARENATIVE_DATAPATHIMPL_TEST_H__
#define __AGENT_VMWARENATIVE_DATAPATHIMPL_TEST_H__

#define private public
#include "dataprocess/datapath/VMwareNativeDataPathImpl.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"


class VMwareNativeDataPathImplTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

public:
    Stub stub;
};

void VMwareNativeDataPathImplTest::SetUp()
{}

void VMwareNativeDataPathImplTest::TearDown()
{}

void VMwareNativeDataPathImplTest::SetUpTestCase()
{}

void VMwareNativeDataPathImplTest::TearDownTestCase()
{}

mp_int32 StubVMwareNativeDataPathImplGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#endif