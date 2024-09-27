#ifndef __AGENT_DB_TEST_H__
#define __AGENT_DB_TEST_H__

#define private public

#include "common/DB.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"

using namespace std;

class  DbTest: public testing::Test{
public:
    Stub stub;
};
#endif