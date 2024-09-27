#ifndef _AGENT_TASK_STEP_SCRIPT_TEST_H_
#define _AGENT_TASK_STEP_SCRIPT_TEST_H_
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#define private public

class TaskStepScriptTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void TaskStepScriptTest::SetUp() {}

void TaskStepScriptTest::TearDown() {}

void TaskStepScriptTest::SetUpTestCase() {}

void TaskStepScriptTest::TearDownTestCase() {}

#endif