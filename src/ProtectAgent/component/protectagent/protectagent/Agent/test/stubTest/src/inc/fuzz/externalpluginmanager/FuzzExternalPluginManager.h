#ifndef _FUZZ_EXTERNALPLUGINMANAGER_H
#define _FUZZ_EXTERNALPLUGINMANAGER_H
#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class FuzzExternalPluginManager : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub m_stub;
};

void FuzzExternalPluginManager::SetUp() {}

void FuzzExternalPluginManager::TearDown() {}

void FuzzExternalPluginManager::SetUpTestCase() {}

void FuzzExternalPluginManager::TearDownTestCase() {}
#endif