#ifndef __AGENT_SEMAF_TEST_H__
#define __AGENT_SEMAF_TEST_H__

#include "common/Semaf.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "common/Log.h"
#include "stub.h"

using namespace std;

class SemafTest: public testing::Test{
public:
    Stub stub;
};
#endif