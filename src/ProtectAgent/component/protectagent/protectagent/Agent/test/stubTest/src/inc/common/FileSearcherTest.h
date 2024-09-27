#ifndef __AGENT_FILE_SEARCHER_TEST_H__
#define __AGENT_FILE_SEARCHER_TEST_H__

#include "common/FileSearcher.h"
#include "gtest/gtest.h"
#include "stub.h"

class CFileSearcherTest: public testing::Test{
public:
    Stub stub;
};

#endif

