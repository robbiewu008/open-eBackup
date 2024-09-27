#ifndef _AGENT_BACKUP_STEP_PREPARE_NAS_MEDIA_TEST_H_
#define _AGENT_BACKUP_STEP_PREPARE_NAS_MEDIA_TEST_H_

#include <vector>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/ConfigXmlParse.h"

class ReportJobDetailFactoryTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};


void ReportJobDetailFactoryTest::SetUp() {}

void ReportJobDetailFactoryTest::TearDown() {}

void ReportJobDetailFactoryTest::SetUpTestCase() {}

void ReportJobDetailFactoryTest::TearDownTestCase() {}

#endif