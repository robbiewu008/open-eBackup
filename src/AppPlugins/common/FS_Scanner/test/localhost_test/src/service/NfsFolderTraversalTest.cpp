/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Author: a00587389
 * Create: 2022-09-01.
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

#include "NfsFolderTraversal.h"

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
    constexpr auto MAX_QUEUE_LIMIT = 100;
    constexpr int ONE_GB = 1024 * 1024 * 1024;
    const std::string PATH_FOR_META = "/home/NFS_FOLDER_TRAVERSAL_TEST/meta";
    const std::string PATH_FOR_CTRL = "/home/NFS_FOLDER_TRAVERSAL_TEST/scan";
}

static void FillCommonScanConfig(ScanConfig& scanConfig)
{
    scanConfig.jobId = "114514";
    scanConfig.reqID = 1919810;

    /* config meta path */
    scanConfig.metaPath = PATH_FOR_META;
    scanConfig.metaPathForCtrlFiles = PATH_FOR_CTRL;
    scanConfig.maxOpendirReqCount = 4000;

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
    scanConfig.fFilter = ScanFileFilter{FILTER_TYPE::DISABLED, {}};

    /* Nfs Config Values */
    scanConfig.nfs.m_contextCount = 0;
}

class NfsFolderTraversalTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    shared_ptr<StatisticsMgr> statsMgr {};
    shared_ptr<ScanFilter> scanFilter {};
    shared_ptr<BufferQueue<DirectoryScan>> buffer {};
    shared_ptr<FSScannerCheckPoint> chkPntMgr {};
    ScanConfig scanConfig {};
    ScanInfo scanInfo {};
    string enqueuePath {"/"};
    Stub stub;
};

void NfsFolderTraversalTest::SetUp()
{
    FillCommonScanConfig(scanConfig);
    scanFilter = make_shared<ScanFilter>(scanConfig.dFilter, scanConfig.fFilter);
    statsMgr = make_shared<StatisticsMgr>();
    buffer = make_shared<BufferQueue<DirectoryScan>>(MAX_QUEUE_LIMIT);
    chkPntMgr = make_shared<FSScannerCheckPoint>(scanConfig, scanInfo);
}

void NfsFolderTraversalTest::TearDown()
{}

void NfsFolderTraversalTest::SetUpTestCase()
{}

void NfsFolderTraversalTest::TearDownTestCase()
{}

static bool ProducerThread_True_stub(void* obj)
{
    cout << "Nfs ProducerThread Start True stub\n";
    return true;
}

static void ProducerThread_Exit_stub(void* obj)
{
    cout << "Nfs ProducerThread Exit stub\n";
    return;
}

TEST_F(NfsFolderTraversalTest, Enqueue)
{
    auto traversalPtr = std::make_unique<NfsFolderTraversal>(statsMgr, scanFilter, buffer, scanConfig, chkPntMgr);
    if (traversalPtr != nullptr) {
        cout << "Nfs Folder Traversal is Not Null Pointer\n";
    }

    EXPECT_EQ(traversalPtr->Enqueue(enqueuePath), SCANNER_STATUS::SUCCESS);
    cout << "m_scanQueue size: " << traversalPtr->m_input->GetSize() << endl;
    traversalPtr->ProcessCheckPointContainers();
    EXPECT_EQ(traversalPtr->Suspend(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Resume(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Abort(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Start(), SCANNER_STATUS::FAILED);
    EXPECT_EQ(traversalPtr->Destroy(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Poll(), SCANNER_STATUS::FAILED);
}

TEST_F(NfsFolderTraversalTest, Filter)
{
    scanConfig.dFilter = ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
        "/dir1"
    }};

    scanFilter = make_shared<ScanFilter>(scanConfig.dFilter, scanConfig.fFilter);
    auto traversalPtr = std::make_unique<NfsFolderTraversal>(statsMgr, scanFilter, buffer, scanConfig, chkPntMgr);
    if (traversalPtr != nullptr) {
        cout << "Nfs Folder Traversal is Not Null Pointer\n";
    }

    EXPECT_EQ(traversalPtr->Enqueue(enqueuePath), SCANNER_STATUS::SUCCESS);
    cout << "m_scanQueue size: " << traversalPtr->m_input->GetSize() << endl;
    traversalPtr->ProcessCheckPointContainers();
    EXPECT_EQ(traversalPtr->Suspend(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Resume(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Abort(), SCANNER_STATUS::SUCCESS);
    EXPECT_EQ(traversalPtr->Start(), SCANNER_STATUS::FAILED);
    EXPECT_EQ(traversalPtr->Destroy(), SCANNER_STATUS::SUCCESS);
}