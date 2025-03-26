#include "PosixFolderTraversal.h"
#include "log/Log.h"
#include "ScannerUtils.h"

#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

using namespace std;

class PosixFolderTraversalTest2 : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<PosixFolderTraversal> m_ins {nullptr};
};

void PosixFolderTraversalTest2::SetUp() 
{
    std::shared_ptr<StatisticsMgr> statsMgr;
    std::shared_ptr<FSScannerCheckPoint> chkPntMgr;
    std::shared_ptr<ScanFilter> scanFilter;
    std::shared_ptr<BufferQueue<DirectoryScan>> buffer;
    ScanConfig config;
    m_ins = std::make_unique<PosixFolderTraversal>(statsMgr, chkPntMgr, scanFilter, buffer, config);
}
void PosixFolderTraversalTest2::TearDown() 
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}
void PosixFolderTraversalTest2::SetUpTestCase() {}
void PosixFolderTraversalTest2::TearDownTestCase() {}

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
 * 用例名称：EnqueueDirToScanQueue
 * 前置条件：无
 * check点：目录转入ScanQueue
 **/
TEST_F(PosixFolderTraversalTest2, EnqueueDirToScanQueue)
{
    MOCKER_CPP(&ScanFilter::AllFiltersDisabled)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&FSScannerCheckPoint::IsScanRestarted)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&ScanFilter::ShouldStopTraverse)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&PosixFolderTraversal::PushDirToWriteQueue)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&PosixFolderTraversal::PushDirToScanQueue)
            .stubs()
            .will(ignoreReturnValue());
    struct stat st;
    string path;
    string prefix;
    uint8_t filterFlag;
    EXPECT_EQ(m_ins->EnqueueDirToScanQueue(st, path, prefix, filterFlag), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(m_ins->EnqueueDirToScanQueue(st, path, prefix, filterFlag), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(m_ins->EnqueueDirToScanQueue(st, path, prefix, filterFlag), SCANNER_STATUS::SUCCESS);
}

/*
 * 用例名称：EnqueueFileToWriteQueue
 * 前置条件：无
 * check点：文件转入WriteQueue
 **/
TEST_F(PosixFolderTraversalTest2, EnqueueFileToWriteQueue)
{
    MOCKER_CPP(&ScanFilter::AllFiltersDisabled)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&ScanFilter::AcceptFile, bool(ScanFilter::*)(std::string, const std::string&)const) // 重载函数
            .stubs()
            .will(returnValue(false));        
    string str = "aaa";
    MOCKER_CPP(&FS_SCANNER::GetParentDirOfFile)
            .stubs()
            .will(returnValue(str));
    struct stat st;
    string path;
    string prefix;
    EXPECT_EQ(m_ins->EnqueueFileToWriteQueue(st, path, prefix), SCANNER_STATUS::SUCCESS);

    m_ins->m_completedPush = true;
    EXPECT_EQ(m_ins->PushUncompletedDirsToWriteQueue(), true);
}