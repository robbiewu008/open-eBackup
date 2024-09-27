#ifndef BSA_OBJ_MANAGER_TEST_H
#define BSA_OBJ_MANAGER_TEST_H

#include "apps/dws/XBSAServer/BsaObjManager.h"
#include "gtest/gtest.h"
#include "stub.h"

using namespace std;

class BsaObjManagerTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    Stub stub;
};

#endif