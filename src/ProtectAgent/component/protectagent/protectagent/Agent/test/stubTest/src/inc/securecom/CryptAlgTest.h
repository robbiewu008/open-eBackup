#ifndef __AGENT_CRYPT_ALG_TEST_H__
#define __AGENT_CRYPT_ALG_TEST_H__
#define private public

#include "securecom/CryptAlg.h"
#include "gtest/gtest.h"
#include "stub.h"

class  CryptAlgTest: public testing::Test{
public:
    Stub stub;
};
#endif

