#ifndef _AGENT_BACKUP_STEP_LINKTARGET_TEST_H_
#define _AGENT_BACKUP_STEP_LINKTARGET_TEST_H_
#define protected public
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"


class TaskStepLinkTargetTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void TaskStepLinkTargetTest::SetUp() {}

void TaskStepLinkTargetTest::TearDown() {}

void TaskStepLinkTargetTest::SetUpTestCase() {}

void TaskStepLinkTargetTest::TearDownTestCase() {}

#endif