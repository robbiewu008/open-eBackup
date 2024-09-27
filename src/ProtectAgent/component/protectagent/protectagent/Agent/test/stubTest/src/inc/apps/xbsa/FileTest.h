#ifndef _APPS_XBSA_FILETEST_H_
#define _APPS_XBSA_FILETEST_H_

#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "xbsaclientcomm/File.h"

class FileTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

inline void FileTest::SetUp() {}

inline void FileTest::TearDown() {}

inline void FileTest::SetUpTestCase() {}

inline void FileTest::TearDownTestCase() {}

#endif
