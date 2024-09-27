#ifndef _AGENT_BACKUP_STEP_SCANDISK_TEST_H_
#define _AGENT_BACKUP_STEP_SCANDISK_TEST_H_

#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"

class TaskStepScanDiskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void TaskStepScanDiskTest::SetUp() {}

void TaskStepScanDiskTest::TearDown() {}

void TaskStepScanDiskTest::SetUpTestCase() {}

void TaskStepScanDiskTest::TearDownTestCase() {}

#endif