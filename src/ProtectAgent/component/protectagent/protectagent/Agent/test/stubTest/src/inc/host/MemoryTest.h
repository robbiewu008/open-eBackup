#ifndef __AGENT_MEMORY_TEST_H__
#define __AGENT_MEMORY_TEST_H__

#define private public

#include "common/CSystemExec.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "common/Log.h"
#include "host/Memory.h"
#include "stub.h"

using namespace std;

class MemoryTest: public testing::Test{
public:
    Stub stub;
};
#endif