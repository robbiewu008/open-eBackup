#include "PosixFolderTraversal.h"
#include "log/Log.h"
#include "ScannerUtils.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

using namespace std;

class PosixFolderTraversalTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<PosixFolderTraversal> m_ins {nullptr};
};

void PosixFolderTraversalTest::SetUp() 
{
    std::shared_ptr<StatisticsMgr> statsMgr;
    std::shared_ptr<FSScannerCheckPoint> chkPntMgr;
    std::shared_ptr<ScanFilter> scanFilter;
    std::shared_ptr<BufferQueue<DirectoryScan>> buffer;
    ScanConfig config;
    m_ins = std::make_unique<PosixFolderTraversal>(statsMgr, chkPntMgr, scanFilter, buffer, config);
}
void PosixFolderTraversalTest::TearDown() {}
void PosixFolderTraversalTest::SetUpTestCase() {}
void PosixFolderTraversalTest::TearDownTestCase() {}

static bool Stub_Func_True()
{
    return true;
}
static bool Stub_Func_False()
{
    return false;
}
static void Stub_Func_Void()
{}

/*
 * 用例名称：PushDirToWriteQueue
 * 前置条件：无
 * check点：添加目录到写队列
 **/
TEST_F(PosixFolderTraversalTest, PushDirToWriteQueue) 
{
    // Stub stub;
    // stub.set(Module::runShellCmdWithOutput, runShellCmdWithOutput_failed);

    // string str = "xxx";
    // bool ret = PluginUtils::CheckDeviceNetworkConnect(str);
    // EXPECT_EQ(ret, false);

    // stub.reset(Module::runShellCmdWithOutput);

    // struct stat st;
    // string path = "/a/file.txt";
    // string prefix = "/usr";
    // uint8_t filterFlag = 0;
    // m_ins->PushDirToScanQueue(st, path, prefix, filterFlag);
}

/*
 * 用例名称：Suspend
 * 前置条件：无
 * check点：暂缓
 **/
TEST_F(PosixFolderTraversalTest, Suspend) 
{
    int ret = static_cast<int>(m_ins->Suspend());
    EXPECT_EQ(ret, 0);
}

/*
 * 用例名称：Resume
 * 前置条件：无
 * check点：重新开始
 **/
TEST_F(PosixFolderTraversalTest, Resume) 
{
    int ret = static_cast<int>(m_ins->Resume());
    EXPECT_EQ(ret, 0);
}

/*
 * 用例名称：Abort
 * 前置条件：无
 * check点：中止
 **/
TEST_F(PosixFolderTraversalTest, Abort) 
{
    SCANNER_STATUS ret = m_ins->Abort();
    EXPECT_EQ(ret, SCANNER_STATUS::SUCCESS);
}