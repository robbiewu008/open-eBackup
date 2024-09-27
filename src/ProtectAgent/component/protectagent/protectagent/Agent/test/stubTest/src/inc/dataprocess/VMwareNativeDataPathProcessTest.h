#ifndef __AGENT_VMWARENATIVE_DATAPATHPROCESS_TEST_H__
#define __AGENT_VMWARENATIVE_DATAPATHPROCESS_TEST_H__

#define private public
#include "dataprocess/datapath/VMwareNativeDataPathProcess.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeDataPathProcessTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

public:
    Stub stub;
};

void VMwareNativeDataPathProcessTest::SetUp()
{}

void VMwareNativeDataPathProcessTest::TearDown()
{}

void VMwareNativeDataPathProcessTest::SetUpTestCase()
{}

void VMwareNativeDataPathProcessTest::TearDownTestCase()
{}

#endif