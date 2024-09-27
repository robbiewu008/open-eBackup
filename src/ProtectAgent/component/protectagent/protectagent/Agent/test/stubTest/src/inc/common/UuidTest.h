#ifndef _UUIDTEST_H_
#define _UUIDTEST_H_

#include "common/Uuid.h"
#include "gtest/gtest.h"
#include "common/ConfigXmlParse.h"
#include "stub.h"

class CUuidNumTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

mp_void StubCUuidNumVoid(mp_void* pthis){
    return;
}

void CUuidNumTest::SetUp() {}

void CUuidNumTest::TearDown() {}

void CUuidNumTest::SetUpTestCase() {}

void CUuidNumTest::TearDownTestCase() {}

#endif
