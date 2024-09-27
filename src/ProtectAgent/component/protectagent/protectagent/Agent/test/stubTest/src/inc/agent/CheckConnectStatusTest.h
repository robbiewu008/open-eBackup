#ifndef __CHECK_CONNET_STATUS_TEST_H__
#define __CHECK_CONNET_STATUS_TEST_H__

#define private public
#define protected public
#include "host/CheckConnectStatus.h"
#include "gtest/gtest.h"
#include "stub.h"

class CCheckConnectStatusTest : public testing::Test
{
public:
    Stub stub;
    CheckConnectStatus worker;
};

/*
void CCheckConnectStatusTest::SetUp() {}

void CCheckConnectStatusTest::TearDown() {}

void CCheckConnectStatusTest::SetUpTestCase() {}

void CCheckConnectStatusTest::TearDownTestCase() {}
*/
#endif
