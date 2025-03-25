#include "ScanQueueUtils.h"
#include "securec.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

using namespace std;

class ScanQueueUtilsTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    Stub stub;
};

void ScanQueueUtilsTest::SetUp() {}
void ScanQueueUtilsTest::TearDown() {}
void ScanQueueUtilsTest::SetUpTestCase() {}
void ScanQueueUtilsTest::TearDownTestCase() {}

/*
 * 用例名称：TranslateDirStat
 * 前置条件：无
 * check点：转变目录状态
 **/
TEST_F(ScanQueueUtilsTest, TranslateDirStat) 
{
    DirStat dirStat;
    dirStat.m_fh.len = 0;
    DirStatReadWrite dirStatRw;
    bool ret = SCAN_QUEUE_UTILS::PartialMapDirStatToDirStatRW(dirStat, dirStatRw);
    EXPECT_EQ(ret, true);

    dirStat.m_fh.len = 1;
    dirStatRw.m_fh.len = 1;
    ret = SCAN_QUEUE_UTILS::PartialMapDirStatToDirStatRW(dirStat, dirStatRw);
    EXPECT_EQ(ret, true);

    ret = SCAN_QUEUE_UTILS::PartialRecoverDirStatFromDirStatRW(dirStat, dirStatRw);
    EXPECT_EQ(ret, true);

    ret = SCAN_QUEUE_UTILS::PartialRecoverDirStatFromDirStatRW(dirStat, dirStatRw);
    EXPECT_EQ(ret, true);
}

static bool Stub_TranslateDirStat_false(void* obj) 
{
    return false;
}
/*
 * 用例名称：WriteDirStatToBuffer
 * 前置条件：无
 * check点：写目录状态到Buffer
 **/
TEST_F(ScanQueueUtilsTest, WriteDirStatToBuffer) 
{
    stringstream buffer;
    DirStat dirStat;
    uint32_t lengthWritten = 0;
    bool ret = SCAN_QUEUE_UTILS::WriteDirStatToBuffer(buffer, dirStat, lengthWritten);
    EXPECT_EQ(ret, true);
}

/*
 * 用例名称：ReadDirStatFromBuffer
 * 前置条件：无
 * check点：从Buffer中读目录状态
 **/
TEST_F(ScanQueueUtilsTest, ReadDirStatFromBuffer) 
{
    stringstream file;
    file << "hello";
    DirStat dirStat;
    bool ret = SCAN_QUEUE_UTILS::ReadDirStatFromBuffer(file, dirStat);
    EXPECT_EQ(ret, false);
}



