#ifndef __AGENT_TIME_OUT_TEST_H__
#define __AGENT_TIME_OUT_TEST_H__
#define private public

#include "common/Utils.h"
#include "common/TimeOut.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "common/Log.h"
#include "stub.h"

using namespace std;

class TimeOutTest: public testing::Test{
public:
    Stub stub;
};
#endif