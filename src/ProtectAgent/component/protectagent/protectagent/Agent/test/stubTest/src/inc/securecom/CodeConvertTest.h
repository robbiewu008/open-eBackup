#ifndef __AGENT_CODE_CONVERT_TEST_H__
#define __AGENT_CODE_CONVERT_TEST_H__
#define private public

#include "securecom/CodeConvert.h"
#include "gtest/gtest.h"
#include "stub.h"

class  CodeConvertTest: public testing::Test{
public:
    Stub stub;
};
#endif

