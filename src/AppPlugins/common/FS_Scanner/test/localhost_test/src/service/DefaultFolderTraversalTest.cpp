#include "DefaultFolderTraversal.h"
#include "log/Log.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

using namespace std;

class DefaultFolderTraversalTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<DefaultFolderTraversal> m_ins {nullptr};
};

void DefaultFolderTraversalTest::SetUp() 
{
    std::shared_ptr<StatisticsMgr> statsMgr = std::make_shared<StatisticsMgr>();
    m_ins = std::make_unique<DefaultFolderTraversal>(statsMgr);
}
void DefaultFolderTraversalTest::TearDown() {}
void DefaultFolderTraversalTest::SetUpTestCase() {}
void DefaultFolderTraversalTest::TearDownTestCase() {}

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
 * 用例名称:Enqueue
 * 前置条件：
 * check点：扫描目录入队列
 */
TEST_F(DefaultFolderTraversalTest, Enqueue) 
{
    std::string directory = "a";
    std::string prefix = "b";
    uint8_t filterFlag = 0;
    SCANNER_STATUS ret = m_ins->Enqueue(directory, prefix, filterFlag);
    EXPECT_EQ(ret, SCANNER_STATUS::SUCCESS);
}

/*
 * 用例名称:Start
 * 前置条件：
 * check点：开始
 */
TEST_F(DefaultFolderTraversalTest, Start) 
{
    SCANNER_STATUS ret = m_ins->Start();
    EXPECT_EQ(ret, SCANNER_STATUS::SUCCESS);
}

/*
 * 用例名称:Poll
 * 前置条件：
 * check点：
 */
TEST_F(DefaultFolderTraversalTest, Poll) 
{
    SCANNER_STATUS ret = m_ins->Poll();
    EXPECT_EQ(ret, SCANNER_STATUS::SCAN_READ_COMPLETED);
}

/*
 * 用例名称:Suspend
 * 前置条件：
 * check点：暂缓
 */
TEST_F(DefaultFolderTraversalTest, Suspend) 
{
    SCANNER_STATUS ret = m_ins->Suspend();
    EXPECT_EQ(ret, SCANNER_STATUS::SUCCESS);
}

/*
 * 用例名称:Resume
 * 前置条件：
 * check点：重新开始
 */
TEST_F(DefaultFolderTraversalTest, Resume) 
{
    SCANNER_STATUS ret = m_ins->Resume();
    EXPECT_EQ(ret, SCANNER_STATUS::SUCCESS);
}

/*
 * 用例名称:Abort
 * 前置条件：
 * check点：异步中止运行扫描实例
 */
TEST_F(DefaultFolderTraversalTest, Abort) 
{
    SCANNER_STATUS ret = m_ins->Abort();
    EXPECT_EQ(ret, SCANNER_STATUS::SUCCESS);
}

/*
 * 用例名称:Destroy
 * 前置条件：
 * check点：同步中止运行扫描实例
 */
TEST_F(DefaultFolderTraversalTest, Destroy) 
{
    m_ins->ProcessCheckPointContainers();
    SCANNER_STATUS ret = m_ins->Destroy();
    EXPECT_EQ(ret, SCANNER_STATUS::SUCCESS);
}