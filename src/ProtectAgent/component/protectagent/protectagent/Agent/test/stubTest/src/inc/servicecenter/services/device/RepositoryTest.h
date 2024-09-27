#ifndef _REPOSITORY_TEST_H
#define _REPOSITORY_TEST_H

#include "stub.h"
#include "gtest/gtest.h"
#include "common/Types.h"

#define private public  // hack complier
#define protected public

class RepositoryTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void RepositoryTest::SetUp() {}

void RepositoryTest::TearDown() {}

void RepositoryTest::SetUpTestCase() {}

void RepositoryTest::TearDownTestCase() {}

#endif