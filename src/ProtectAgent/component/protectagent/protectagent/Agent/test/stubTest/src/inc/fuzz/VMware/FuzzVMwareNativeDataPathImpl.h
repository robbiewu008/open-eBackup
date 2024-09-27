#ifndef _AGENT_VMWAREVMRESTORE_PLUGIN_TEST_H_
#define _AGENT_VMWAREVMRESTORE_PLUGIN_TEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class FuzzVMwareNativeDataPathIml : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub m_stub;
};

void FuzzVMwareNativeDataPathIml::SetUp() {}

void FuzzVMwareNativeDataPathIml::TearDown() {}

void FuzzVMwareNativeDataPathIml::SetUpTestCase() {}

void FuzzVMwareNativeDataPathIml::TearDownTestCase() {}

#endif /* _AGENT_VMWAREVMRESTORE_PLUGIN_TEST_H_; */

