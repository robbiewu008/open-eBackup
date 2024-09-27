#ifndef _AGENT_TASKMANAGER_TEST_H_
#define _AGENT_TASKMANAGER_TEST_H_
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#ifdef LINUX
#include <gperftools/malloc_extension.h>
#endif
#define private public
#define protected public

class TaskManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void TaskManagerTest::SetUp() {}

void TaskManagerTest::TearDown() {}

void TaskManagerTest::SetUpTestCase() {}

void TaskManagerTest::TearDownTestCase() {}

#endif