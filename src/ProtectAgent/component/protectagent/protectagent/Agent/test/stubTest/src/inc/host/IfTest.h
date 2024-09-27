#ifndef __AGENT_IF_TEST_H__
#define __AGENT_IF_TEST_H__

#define private public

#include "host/If.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

using namespace std;

class  IfTest: public testing::Test{
public:
    Stub stub;
};
#endif