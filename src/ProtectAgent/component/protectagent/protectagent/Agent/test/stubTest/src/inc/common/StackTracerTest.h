#ifndef __AGENT_STACK_TRACER_TEST_H__
#define __AGENT_STACK_TRACER_TEST_H__
#define private public

#include "common/StackTracer.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "common/Log.h"
#include "stub.h"

using namespace std;

class StackTracerTest: public testing::Test{
public:
    Stub stub;
};
#endif