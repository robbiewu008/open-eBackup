#include "ScannerImpl.h"
#include "PosixFolderTraversal.h"
#include "SmbFolderTraversal.h"
#include "NfsFolderTraversal.h"
#include "DefaultFolderTraversal.h"
#include "ScannerUtils.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"


using namespace std;

class ScannerImplTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<ScannerImpl> m_ins {nullptr};
};

void ScannerImplTest::SetUp() {
    ScanConfig scanConfig;
    m_ins = std::make_unique<ScannerImpl>(scanConfig);
}
void ScannerImplTest::TearDown() {}
void ScannerImplTest::SetUpTestCase() {}
void ScannerImplTest::TearDownTestCase() {}

TEST_F(ScannerImplTest, Abort) 
{
    m_ins->m_status = SCANNER_STATUS::SUCCESS;
    SCANNER_STATUS ret = m_ins->Abort();
    EXPECT_EQ(ret, SCANNER_STATUS::SUCCESS);

    m_ins->m_status = SCANNER_STATUS::SCAN_IN_PROGRESS;
    ret = m_ins->Abort();
    m_ins->m_status = SCANNER_STATUS::ABORTED;
    EXPECT_EQ(ret, SCANNER_STATUS::SUCCESS);
}

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

TEST_F(ScannerImplTest, SetOutputDirForScan) 
{
    stub.set(ADDR(FSScannerCheckPoint, IsScanRestarted), Stub_Func_True);
    m_ins->SetOutputDirForScan();
    stub.reset(ADDR(FSScannerCheckPoint, IsScanRestarted));

    stub.set(ADDR(FSScannerCheckPoint, IsScanRestarted), Stub_Func_False);
    stub.set(ADDR(ScannerImpl, CheckAndCreateOutputPath), Stub_Func_False);
    m_ins->SetOutputDirForScan();
    stub.reset(ADDR(FSScannerCheckPoint, IsScanRestarted));
    stub.reset(ADDR(ScannerImpl, CheckAndCreateOutputPath));
}

static int Stub_Func_STATUS_SUCCESS()
{
    return static_cast<int>(SCANNER_STATUS::SUCCESS);
}
static int Stub_Func_STATUS_SECONDARY_SERVER_NOT_REACHABLE()
{
    return static_cast<int>(SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE);
}
// TEST_F(ScannerImplTest, GenerateMeta) 
// {
//     stub.set(ADDR(ControlGenerator, Poll), Stub_Func_STATUS_SUCCESS);
//     m_ins->m_config.scanType = ScanJobType::CONTROL_GEN;
//     m_ins->StartGenerator();

//     stub.set(ADDR(ControlGenerator, GenerateMeta), Stub_Func_STATUS_SECONDARY_SERVER_NOT_REACHABLE);
//     m_ins->GenerateMeta();
//     EXPECT_EQ(m_ins->m_status, SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE);
// }

static SCANNER_STATUS Stub_Func_Scanner_Status_Success()
{
    return SCANNER_STATUS::SUCCESS;
}
 
static SCANNER_STATUS Stub_Func_Scanner_Status_Failure()
{
    return SCANNER_STATUS::FAILED;
}


TEST_F(ScannerImplTest, GenerateDiffControl) 
{
    m_ins->GenerateDiffControl();

    stub.set(ADDR(ControlGenerator, GenerateDiffControl), Stub_Func_Scanner_Status_Success);
    m_ins->GenerateDiffControl();
    stub.reset(ADDR(ControlGenerator, GenerateDiffControl));

    stub.set(ADDR(ControlGenerator, GenerateDiffControl), Stub_Func_Scanner_Status_Failure);
    m_ins->GenerateDiffControl();
    stub.reset(ADDR(ControlGenerator, GenerateDiffControl));
}

TEST_F(ScannerImplTest, CacheInProgressEvent) 
{
    m_ins->m_status = SCANNER_STATUS::META_WRITE_COMPLETED;
    m_ins->CacheInProgressEvent();
    EXPECT_EQ(m_ins->m_status, SCANNER_STATUS::CACHE_MERGE_IN_PROGRESS);
}

TEST_F(ScannerImplTest, CtrlDiffInProgressEvent) 
{
    m_ins->m_status = SCANNER_STATUS::CACHE_MERGE_COMPLETED;
    m_ins->CtrlDiffInProgressEvent();
    EXPECT_EQ(m_ins->m_status, SCANNER_STATUS::CTRL_DIFF_IN_PROGRESS);
}

TEST_F(ScannerImplTest, CtrlDiffCompletedEvent) 
{
    m_ins->m_status = SCANNER_STATUS::SUCCESS;
    m_ins->CtrlDiffInProgressEvent();

    stub.set(ADDR(ControlGenerator, Clean), Stub_Func_False);
    m_ins->m_status = SCANNER_STATUS::INIT;
    m_ins->CtrlDiffCompletedEvent();
    EXPECT_EQ(m_ins->m_status, SCANNER_STATUS::CTRL_DIFF_COMPLETED);  
    stub.reset(ADDR(ControlGenerator, Clean));
}

TEST_F(ScannerImplTest, CleanInProgressEvent) 
{
    m_ins->m_status = SCANNER_STATUS::SUCCESS;
    m_ins->CleanInProgressEvent();

    m_ins->m_status = SCANNER_STATUS::INIT;
    m_ins->CleanInProgressEvent();
    EXPECT_EQ(m_ins->m_status, SCANNER_STATUS::CLEAN_IN_PROGRESS);  
}

static int Stub_Func_STATUS_CACHE_MERGE_IN_PROGRESS()
{
    return static_cast<int>(SCANNER_STATUS::CACHE_MERGE_IN_PROGRESS);
}
static int Stub_Func_STATUS_CTRL_DIFF_IN_PROGRESS()
{
    return static_cast<int>(SCANNER_STATUS::CTRL_DIFF_IN_PROGRESS);
}
static int Stub_Func_STATUS_CTRL_DIFF_COMPLETED()
{
    return static_cast<int>(SCANNER_STATUS::CTRL_DIFF_COMPLETED);
}
static int Stub_Func_STATUS_CLEAN_IN_PROGRESS()
{
    return static_cast<int>(SCANNER_STATUS::CLEAN_IN_PROGRESS);
}
static int Stub_Func_STATUS_FAILED()
{
    return static_cast<int>(SCANNER_STATUS::FAILED);
}
TEST_F(ScannerImplTest, PollGenerator) 
{
    stub.set(ADDR(ControlGenerator, Poll), Stub_Func_STATUS_CACHE_MERGE_IN_PROGRESS);
    m_ins->m_status = SCANNER_STATUS::CLEAN_IN_PROGRESS;
    m_ins->PollGenerator();

    stub.set(ADDR(ControlGenerator, Poll), Stub_Func_STATUS_CTRL_DIFF_IN_PROGRESS);
    m_ins->PollGenerator();

    stub.set(ADDR(ControlGenerator, Poll), Stub_Func_STATUS_CTRL_DIFF_COMPLETED);
    m_ins->PollGenerator();

    stub.set(ADDR(ControlGenerator, Poll), Stub_Func_STATUS_CLEAN_IN_PROGRESS);
    m_ins->PollGenerator();

    stub.set(ADDR(ControlGenerator, Poll), Stub_Func_STATUS_FAILED);
    m_ins->PollGenerator();

    stub.set(ADDR(ControlGenerator, Poll), Stub_Func_STATUS_SUCCESS);
    m_ins->PollGenerator();

    m_ins->m_generatorPtr = nullptr;
    m_ins->PollGenerator();
    // EXPECT_EQ(m_ins->m_generatorPtr, nullptr);

    stub.reset(ADDR(ControlGenerator, Poll));
}

static SCANNER_STATUS Stub_Destroy()
{
    return SCANNER_STATUS::SUCCESS;
}
TEST_F(ScannerImplTest, HandleJobFailure) 
{
    typedef SCANNER_STATUS (*fptr)(DefaultFolderTraversal*);
    fptr DefaultFolderTraversal_Destroy = (fptr)(&DefaultFolderTraversal::Destroy);
    ScanConfig scanConfig;
    scanConfig.scanIO = IOEngine::DEFAULT;
    ScannerImpl scannerImpl(scanConfig);
    stub.set(DefaultFolderTraversal_Destroy, Stub_Func_Void);
    stub.set(ADDR(ControlGenerator, Clean), Stub_Func_Void);
    m_ins->HandleJobFailure();
    EXPECT_EQ(m_ins->m_status, SCANNER_STATUS::ABORTED);
    stub.reset(DefaultFolderTraversal_Destroy);
    stub.reset(ADDR(ControlGenerator, Clean));
}

TEST_F(ScannerImplTest, CheckAndCreateOutputPath) 
{
    string dirPath = "/a/file.txt";
    stub.set(FS_SCANNER::CheckAndCreateDirectory, Stub_Func_False);
    stub.set(FS_SCANNER::CheckParentDirIsReachable, Stub_Func_False);
    bool ret = m_ins->CheckAndCreateOutputPath(dirPath);
    EXPECT_EQ(ret, false);
    stub.reset(FS_SCANNER::CheckAndCreateDirectory);
    stub.reset(FS_SCANNER::CheckParentDirIsReachable); 

    stub.set(FS_SCANNER::CheckAndCreateDirectory, Stub_Func_False);
    stub.set(FS_SCANNER::CheckParentDirIsReachable, Stub_Func_False);
    stub.set(FS_SCANNER::CheckAndCreateDirectory, Stub_Func_False);
    ret = m_ins->CheckAndCreateOutputPath(dirPath);
    EXPECT_EQ(ret, false);
    stub.reset(FS_SCANNER::CheckAndCreateDirectory);
    stub.reset(FS_SCANNER::CheckParentDirIsReachable); 
    stub.reset(FS_SCANNER::CheckAndCreateDirectory);
}