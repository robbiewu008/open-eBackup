#ifndef _AGENT_ORACLE_NATIVE_REDO_TASK_REGISTER_TEST_
#define _AGENT_ORACLE_NATIVE_REDO_TASK_REGISTER_TEST_

#include "apps/oraclenative/OracleNativeRedoTaskRegister.h"
#include "gtest/gtest.h"
#include "stub.h"

class OracleNativeRedoTaskRegisterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void OracleNativeRedoTaskRegisterTest::SetUp()
{}

void OracleNativeRedoTaskRegisterTest::TearDown()
{}

void OracleNativeRedoTaskRegisterTest::SetUpTestCase()
{}

void OracleNativeRedoTaskRegisterTest::TearDownTestCase()
{}

#endif