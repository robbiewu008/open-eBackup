#ifndef __AGENT_GPU_TEST_H__
#define __AGENT_GPU_TEST_H__

#define private public
#define protected public

#include "host/Gpu.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"

using namespace std;

class  GpuTest: public testing::Test{
public:
    Stub stub;
};
#endif