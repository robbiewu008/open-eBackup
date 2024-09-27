#ifndef __AGENT_JSONUTILS_TEST_H__
#define __AGENT_JSONUTILS_TEST_H__

#define private public

#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Defines.h"
#include "common/ErrorCode.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"

using namespace std;

class JsonUtilsTest: public testing::Test{
public:
    Stub stub;
};
#endif