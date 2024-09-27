#pragma once
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#define private public

class PluginAnyTimeRestoreTest : public testing::Test {

public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void PluginAnyTimeRestoreTest::SetUp() {}

void PluginAnyTimeRestoreTest::TearDown() {}

void PluginAnyTimeRestoreTest::SetUpTestCase() {}

void PluginAnyTimeRestoreTest::TearDownTestCase() {}