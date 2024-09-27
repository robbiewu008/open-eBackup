#ifndef _AGENT_TASKREDO_FUNCCONTAINER_TEST_H_
#define _AGENT_TASKREDO_FUNCCONTAINER_TEST_H_

#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"

class TaskRedoFuncContainerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void TaskRedoFuncContainerTest::SetUp() {}

void TaskRedoFuncContainerTest::TearDown() {}

void TaskRedoFuncContainerTest::SetUpTestCase() {}

void TaskRedoFuncContainerTest::TearDownTestCase() {}



#endif