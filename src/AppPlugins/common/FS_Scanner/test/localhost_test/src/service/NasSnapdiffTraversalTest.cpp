/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Author: w30029850
 * Create: 2022-08-26.
 */

#include <list>
#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "Scanner.h"
#include "log/Log.h"
#include "ScannerTime.h"

#include "NasSnapdiffTraversal.h"
#include "DeviceManager.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace std;
using namespace Module;

namespace {
    constexpr auto MODULE = "NAS_SNAPDIFF_TRAVERSAL_TEST";
    const std::string LOG_NAME = "NAS_SNAPDIFF_TRAVERSAL_TEST.log";
    const std::string LOG_PATH = "/home/NAS_SNAPDIFF_TRAVERSAL_TEST/log/";
    constexpr int ONE_GB = 1024 * 1024 * 1024;

    const std::string PATH_FOR_META = "/home/NAS_SNAPDIFF_TRAVERSAL_TEST/meta";
    const std::string PATH_FOR_CTRL = "/home/NAS_SNAPDIFF_TRAVERSAL_TEST/scan";

    constexpr auto MAX_QUEUE_LIMIT = 100;
    constexpr auto PROGRESS_MAX = "100";
}

typedef int (*Fptr_EndSnapshotDiffSession)(OceanstorNas*, std::string);
Fptr_EndSnapshotDiffSession ptr_EndSnapshotDiffSession = (Fptr_EndSnapshotDiffSession)(&OceanstorNas::EndSnapshotDiffSession);

typedef int (*Fptr_StartSnapshotDiffSession)(OceanstorNas*, std::string, std::string, std::string&);
Fptr_StartSnapshotDiffSession ptr_StartSnapshotDiffSession = (Fptr_StartSnapshotDiffSession)(&OceanstorNas::StartSnapshotDiffSession);

typedef int (*Fptr_GetSnapshotDiffChanges)(OceanstorNas*, std::string sessionId, SnapdiffInfo& SnapdiffInfo,
    SnapdiffMetadataInfo metadatInfo[], int metadataListLen);
Fptr_GetSnapshotDiffChanges ptr_GetSnapshotDiffChanges = (Fptr_GetSnapshotDiffChanges)(&OceanstorNas::GetSnapshotDiffChanges);

static void InitHttpStatusCodeForRetry_stub(void* obj)
{
    std::cout << "InitHttpStatusCodeForRetry stub invoked" << std::endl;
}

static int EndSnapshotDiffSession_stub(void* obj, std::string sessionID)
{
    std::cout << "EndSnapshotDiffSession stub invoked" << std::endl;
    return Module::SUCCESS;
}

static std::unique_ptr<Module::ControlDevice> InitOceanStorDevice_fail_stub(void* obj)
{
    return nullptr;
}

static int32_t GetSnapshotDiffChanges_stub(void* obj, std::string sessionId, SnapdiffInfo& snapdiffInfo,
    SnapdiffMetadataInfo metadatInfo[], int metadataListLen)
{
    std::cout << "GetSnapshotDiffChanges stub invoked" << std::endl;
    snapdiffInfo.numChanges = 6;
    snapdiffInfo.progress = PROGRESS_MAX;

    metadatInfo[0].filePath = "/home";
    metadatInfo[0].fType = SNAPDIFF_METADATA_INFO_F_TYPE::DIR_TYPE;

    metadatInfo[1].filePath = "/home/1.txt";
    metadatInfo[1].fType = SNAPDIFF_METADATA_INFO_F_TYPE::FILE_TYPE;

    metadatInfo[2].filePath = "/etc";
    metadatInfo[2].fType = SNAPDIFF_METADATA_INFO_F_TYPE::DIR_TYPE;

    metadatInfo[3].filePath = "/root/2.avi";
    metadatInfo[3].fType = SNAPDIFF_METADATA_INFO_F_TYPE::FILE_TYPE;

    metadatInfo[4].filePath = "/"; // illegal return. just for test
    metadatInfo[4].fType = SNAPDIFF_METADATA_INFO_F_TYPE::FILE_TYPE;

    metadatInfo[5].filePath = "/root";
    metadatInfo[5].fType = SNAPDIFF_METADATA_INFO_F_TYPE::DIR_TYPE;

    metadatInfo[6].filePath = "/4.txt";
    metadatInfo[6].fType = SNAPDIFF_METADATA_INFO_F_TYPE::DIR_TYPE;
    return Module::SUCCESS;
}

static int32_t GetSnapshotDiffChanges_result0_stub(void* obj, std::string sessionId, SnapdiffInfo& snapdiffInfo,
    SnapdiffMetadataInfo metadatInfo[], int metadataListLen)
{
    std::cout << "GetSnapshotDiffChanges result 0 stub invoked" << std::endl;
    snapdiffInfo.numChanges = 0;
    snapdiffInfo.progress = PROGRESS_MAX;
    return Module::SUCCESS;
}

static int32_t StartSnapshotDiffSession_success_stub(void* obj, std::string BaseSnapshotName,
    std::string IncrementalSnapshotName, std::string& sessionId)
{
    std::cout << "StartSnapshotDiffSession success stub invoked" << std::endl;
    sessionId = "114514";
    return Module::SUCCESS;
}

static int32_t StartSnapshotDiffSession_fail_stub(void* obj, std::string BaseSnapshotName,
    std::string IncrementalSnapshotName, std::string& sessionId)
{
    std::cout << "StartSnapshotDiffSession fail stub invoked" << std::endl;
    sessionId = "114514";
    return Module::FAILED;
}

static void FillCommonScanConfig(ScanConfig& scanConfig)
{
    scanConfig.jobId = "114514";
    scanConfig.reqID = 1919810;

    /* config meta path */
    scanConfig.metaPath = PATH_FOR_META;
    scanConfig.metaPathForCtrlFiles = PATH_FOR_CTRL;
    scanConfig.maxOpendirReqCount = 4000;

    // /* 记录线程数 */
    scanConfig.maxCommonServiceInstance = 1;

    scanConfig.scanCtrlMaxDataSize = "114514";
    scanConfig.scanCtrlMinDataSize = "1919810";
    scanConfig.scanCtrlFileTimeSec = 5;
    scanConfig.scanCtrlMaxEntriesFullBkup = 100000;
    scanConfig.scanCtrlMaxEntriesIncBkup = 10000;
    scanConfig.scanCtrlMinEntriesFullBkup = 100000;
    scanConfig.scanCtrlMinEntriesIncBkup = 10000;
    scanConfig.scanMetaFileSize = ONE_GB;

    scanConfig.maxWriteQueueSize = 10000;
    scanConfig.triggerTime = Time::Now().MicroSeconds();

    scanConfig.dFilter = ScanDirectoryFilter{FILTER_TYPE::DISABLED, {}};
    scanConfig.fFilter = ScanFileFilter{FILTER_TYPE::EXCLUDE, {
        "/4.txt"
    }};
}

static void FillScanConfigForSnapdiff(ScanConfig &scanConfig)
{
    scanConfig.scanType = ScanJobType::SNAPDIFFNAS_GEN;
    scanConfig.scanIO = IOEngine::SNAPDIFFNAS;
    scanConfig.nasSnapdiffProtocol = NAS_PROTOCOL::SMB;
    scanConfig.scanCheckPointEnable = false;
    scanConfig.lastBackupTime = 0;

    scanConfig.baseSnapshotName = "TEST_SNAPSHOT_1";
    scanConfig.incSnapshotName = "TEST_SNAPSHOT_2";

    scanConfig.deviceResourceName = "/nasfs/"; // illegal name, just for test
    scanConfig.deviceUrl = "11.4.5.14";
    scanConfig.devicePort = "8088";
    scanConfig.deviceUsername = "admin";
    scanConfig.devicePassword = "Huawei@123";
    scanConfig.deviceCert = "";
    scanConfig.devicePoolID = 0;
}

class NasSnapdiffTraversalTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    std::shared_ptr<StatisticsMgr> statsMgr {};
    std::shared_ptr<ScanFilter> scanFilter;
    std::shared_ptr<BufferQueue<SnapdiffResultMap>> snapdiffBuffer;
    ScanConfig scanConfig {};
    std::string enqueuePath {"/"};
    Stub stub;
};

void NasSnapdiffTraversalTest::SetUp()
{
    stub.set(ADDR(OceanstorNas, InitHttpStatusCodeForRetry), InitHttpStatusCodeForRetry_stub);
    stub.set(ptr_StartSnapshotDiffSession, StartSnapshotDiffSession_success_stub);
    stub.set(ptr_GetSnapshotDiffChanges, GetSnapshotDiffChanges_stub);
    stub.set(ptr_EndSnapshotDiffSession, EndSnapshotDiffSession_stub);

    FillCommonScanConfig(scanConfig);
    FillScanConfigForSnapdiff(scanConfig);
    scanFilter = make_shared<ScanFilter>(scanConfig.dFilter, scanConfig.fFilter);
    statsMgr = make_shared<StatisticsMgr>();
    snapdiffBuffer = make_shared<BufferQueue<SnapdiffResultMap>>(MAX_QUEUE_LIMIT);
}

void NasSnapdiffTraversalTest::TearDown()
{}

void NasSnapdiffTraversalTest::SetUpTestCase()
{}

void NasSnapdiffTraversalTest::TearDownTestCase()
{}


TEST_F(NasSnapdiffTraversalTest, TEST_SNAPDIFF_TRAVERSAL_START_SNAPDIFF_SESSION_SUCCESS)
{
    auto traversalPtr = std::make_shared<NasSnapdiffTraversal>(statsMgr, scanFilter, snapdiffBuffer, scanConfig);
    traversalPtr->ProcessCheckPointContainers();

    EXPECT_EQ(traversalPtr->Enqueue(enqueuePath), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Poll(), SCANNER_STATUS::SCAN_IN_PROGRESS);
    EXPECT_EQ(traversalPtr->Suspend(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Resume(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Abort(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Destroy(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Start(), SCANNER_STATUS::SUCCESS);
}

TEST_F(NasSnapdiffTraversalTest, TEST_SNAPDIFF_TRAVERSAL_START_SNAPDIFF_SESSION_FAIL)
{
    stub.set(ptr_StartSnapshotDiffSession, StartSnapshotDiffSession_fail_stub);

    auto traversalPtr = std::make_shared<NasSnapdiffTraversal>(statsMgr, scanFilter, snapdiffBuffer, scanConfig);
    traversalPtr->ProcessCheckPointContainers();

    EXPECT_EQ(traversalPtr->Enqueue(enqueuePath), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Poll(), SCANNER_STATUS::SCAN_IN_PROGRESS);
    EXPECT_EQ(traversalPtr->Suspend(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Resume(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Abort(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Destroy(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Start(), SCANNER_STATUS::FAILED);
}

TEST_F(NasSnapdiffTraversalTest, TEST_SNAPDIFF_TRAVERSAL_NULL_DEVICE)
{
    stub.set(ADDR(NasSnapdiffTraversal, InitOceanStorDevice), InitOceanStorDevice_fail_stub);
    auto traversalPtr = std::make_shared<NasSnapdiffTraversal>(statsMgr, scanFilter, snapdiffBuffer, scanConfig);
    traversalPtr->ProcessCheckPointContainers();

    EXPECT_EQ(traversalPtr->Enqueue(enqueuePath), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Poll(), SCANNER_STATUS::SCAN_IN_PROGRESS);
    EXPECT_EQ(traversalPtr->Suspend(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Resume(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Abort(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Destroy(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Start(), SCANNER_STATUS::FAILED);
}

TEST_F(NasSnapdiffTraversalTest, TEST_SNAPDIFF_TRAVERSAL_NFS)
{
    scanConfig.nasSnapdiffProtocol = NAS_PROTOCOL::NFS;
    auto traversalPtr = std::make_shared<NasSnapdiffTraversal>(statsMgr, scanFilter, snapdiffBuffer, scanConfig);
    traversalPtr->ProcessCheckPointContainers();

    EXPECT_EQ(traversalPtr->Enqueue(enqueuePath), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Poll(), SCANNER_STATUS::SCAN_IN_PROGRESS);
    EXPECT_EQ(traversalPtr->Suspend(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Resume(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Abort(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Destroy(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Start(), SCANNER_STATUS::SUCCESS);
}


TEST_F(NasSnapdiffTraversalTest, TEST_SNAPDIFF_TRAVERSAL_RESULT_0)
{
    stub.set(ptr_GetSnapshotDiffChanges, GetSnapshotDiffChanges_result0_stub);
    auto traversalPtr = std::make_shared<NasSnapdiffTraversal>(statsMgr, scanFilter, snapdiffBuffer, scanConfig);
    traversalPtr->ProcessCheckPointContainers();

    EXPECT_EQ(traversalPtr->Enqueue(enqueuePath), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Poll(), SCANNER_STATUS::SCAN_IN_PROGRESS);
    EXPECT_EQ(traversalPtr->Suspend(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Resume(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Abort(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Destroy(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Start(), SCANNER_STATUS::SUCCESS);
}
