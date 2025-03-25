#include "gtest/gtest.h"
#include "ScanMgr.h"
#include "ScannerImpl.h"
#include "stub.h"

using namespace std;
using namespace Module;
using namespace FS_SCANNER;

class ScanMgrTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    ScanMgr& instance = ScanMgr::GetInstance();
};

void ScanMgrTest::SetUp()
{
    
}

void ScanMgrTest::TearDown()
{}

void ScanMgrTest::SetUpTestCase()
{}

void ScanMgrTest::TearDownTestCase()
{
    // FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE);
}

static SCANNER_STATUS Stub_SUCCESS(void* obj)
{
    return SCANNER_STATUS::SUCCESS;
}

static SCANNER_STATUS Stub_FAILED(void* obj)
{
    return SCANNER_STATUS::FAILED;
}

static void FillScanConfig(ScanConfig& config)
{
    config.reqID  = 0;
    config.jobId = "1234";

    config.scanIO = IOEngine::DEFAULT;
    config.lastBackupTime = 0;

    /* config meta path */
    config.curDcachePath  = "m_scanInputMetePath";
    config.metaPathForCtrlFiles = "m_scanControlFilePath";

    config.maxOpendirReqCount = 4000;

    /* 记录线程数 */
    config.maxCommonServiceInstance = 1;

    config.usrData = nullptr;
    config.scanResultCb = [](void* usrData, std::string) {};
    config.scanHardlinkResultCb = [](void* usrData, std::string) {};

    config.scanCtrlMaxDataSize = "SCAN_CTRL_MAX_DATA_SIZE";
    config.scanCtrlMinDataSize = "SCAN_CTRL_MIN_DATA_SIZE";
    config.scanCtrlFileTimeSec = 5;
    config.scanCtrlMaxEntriesFullBkup = 10000;
    config.scanCtrlMaxEntriesIncBkup = 1000;
    config.scanCtrlMinEntriesFullBkup = 10000;
    config.scanCtrlMinEntriesIncBkup = 1000;
    config.scanMetaFileSize = 1024*1024*1024; // one GB

    config.maxWriteQueueSize = 1000;
    config.scanType = ScanJobType::CONTROL_GEN; // use dcache/fcache to create control file, skip enqueue
    config.triggerTime = 0;
    config.generatorIsFull =  true;
    config.scanCheckPointEnable = false;
}

TEST_F(ScanMgrTest, GetInstance)
{
    ScanMgr& instance1 = ScanMgr::GetInstance();
    ScanMgr& instance2 = ScanMgr::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(ScanMgrTest, CreateScanInst)
{
    ScanConfig config {};
    FillScanConfig(config);
    unique_ptr<Scanner> pScan = ScanMgr::CreateScanInst(config);
    EXPECT_FALSE(pScan == nullptr);
    pScan->Destroy();
}

TEST_F(ScanMgrTest, NewScanInst)
{
    ScanConfig config {};
    FillScanConfig(config);
    Scanner *pScan = ScanMgr::NewScanInst(config);
    EXPECT_FALSE(pScan == nullptr);
    pScan->Destroy();
    delete pScan;
}

TEST_F(ScanMgrTest, RegisterTask)
{
    instance.RegisterTask(ScanTaskLevel::REGULAR, "regular_task");
    EXPECT_FALSE(instance.m_taskList.empty());
    EXPECT_EQ(instance.m_taskList.size(), 1);
    instance.RegisterTask(ScanTaskLevel::LOWLEVEL, "lowlevel_task");
    EXPECT_EQ(instance.m_taskList.size(), 2);
    instance.m_taskList.clear();
}

TEST_F(ScanMgrTest, HoldRunningTask)
{
    instance.RegisterTask(ScanTaskLevel::REGULAR, "regular_task");
    instance.RegisterTask(ScanTaskLevel::LOWLEVEL, "lowlevel_task");
    instance.RegisterTask(ScanTaskLevel::REGULAR, "task1");
    instance.RegisterTask(ScanTaskLevel::REGULAR, "task2");

    EXPECT_TRUE(instance.HoldRunningTask("regular_task"));
    EXPECT_FALSE(instance.HoldRunningTask("task1"));
    EXPECT_FALSE(instance.HoldRunningTask("task2"));
    EXPECT_FALSE(instance.HoldRunningTask("lowlevel_task"));
    instance.m_taskList.clear();
}

TEST_F(ScanMgrTest, ReleaseRunningTask)
{
    instance.RegisterTask(ScanTaskLevel::REGULAR, "regular_task");
    instance.RegisterTask(ScanTaskLevel::LOWLEVEL, "lowlevel_task");

    instance.HoldRunningTask("regular_task");
    EXPECT_TRUE(instance.ReleaseRunningTask("lowlevel_task"));
    EXPECT_TRUE(instance.ReleaseRunningTask("regular_task"));
    EXPECT_EQ(instance.m_taskList.size(), 0);
    instance.m_taskList.clear();
}

TEST_F(ScanMgrTest, Initiate)
{
    ScanConfig config {};
    FillScanConfig(config);
    EXPECT_TRUE(instance.Initiate(config));
    instance.Destroy();
}

TEST_F(ScanMgrTest, StartScan)
{
    ScanConfig config {};
    FillScanConfig(config);
    instance.Initiate(config);
    typedef int (*fptr)(ScannerImpl*);
    fptr ScannerImpl_Start = (fptr)(&ScannerImpl::Start);
    stub.set(ScannerImpl_Start, Stub_FAILED);
    EXPECT_EQ(instance.StartScan(), SCANNER_STATUS::FAILED);
    instance.Destroy();
}

TEST_F(ScanMgrTest, GetScanStatus)
{
    ScanConfig config {};
    FillScanConfig(config);
    instance.Initiate(config);
    typedef int (*fptr)(ScannerImpl*);
    fptr ScannerImpl_Start = (fptr)(&ScannerImpl::GetStatus);
    stub.set(ScannerImpl_Start, Stub_FAILED);
    EXPECT_EQ(instance.GetScanStatus(), SCANNER_STATUS::FAILED);
    instance.Destroy();
}

TEST_F(ScanMgrTest, AbortScan)
{
    ScanConfig config {};
    FillScanConfig(config);
    instance.Initiate(config);
    typedef int (*fptr)(ScannerImpl*);
    fptr ScannerImpl_Start = (fptr)(&ScannerImpl::Abort);
    stub.set(ScannerImpl_Start, Stub_FAILED);
    EXPECT_EQ(instance.AbortScan(), SCANNER_STATUS::FAILED);
    instance.Destroy();
}