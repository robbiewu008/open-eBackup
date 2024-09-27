#ifndef _AGENT_BACKUP_TASK_STEP_SCRIPT_TEST_
#define _AGENT_BACKUP_TASK_STEP_SCRIPT_TEST_

#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"

class TaskStepPreScriptTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void TaskStepPreScriptTest::SetUp() {}

void TaskStepPreScriptTest::TearDown() {}

void TaskStepPreScriptTest::SetUpTestCase() {}

void TaskStepPreScriptTest::TearDownTestCase() {}

#endif