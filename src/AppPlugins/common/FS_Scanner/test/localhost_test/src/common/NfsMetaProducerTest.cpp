/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Author: a00587389
 * Create: 12-09-2022
 */

#include <list>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/acl.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "Scanner.h"
#include "ScannerTime.h"
#include "ScannerUtils.h"
#include "NfsMetaProducer.h"

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
    constexpr auto MODULE = "NFS_META_PRODUCER_TEST";
    const std::string LOG_NAME = "NFS_META_PRODUCER_TEST.log";
    const std::string LOG_PATH = "/home/NFS_META_PRODUCER_TEST/log/";
    constexpr int ONE_GB = 1024 * 1024 * 1024;

    const std::string PATH_FOR_META = "/home/NFS_META_PRODUCER_TEST/meta";
    const std::string PATH_FOR_CTRL = "/home/NFS_META_PRODUCER_TEST/scan";

    constexpr auto MAX_QUEUE_LIMIT = 100;
}

static int NfsMount_stub(void *obj)
{
    std::cout << "NfsMount stub invoked" << std::endl;
    return Module::SUCCESS;
}

static void NfsService_stub(void *obj)
{
    std::cout << "NfsService stub invoked" << std::endl;
    return;
}

static void NfsV3GetAcl_stub(struct RpcClient* client, struct nfsdirent* &nfsdirent, std::string &aclValue)
{
    std::cout << "NfsV3GetAcl stub invoked" << std::endl;
    aclValue = "ACL:S-1-1-0:0x0/0x13/0x1f01ff";
    return;
}

static int NfsOpendirAsync_stub(const char *path, nfs_cb cb, void *privateData)
{
    cout << "NfsOpendirAsync Stub Invoked" << endl;
    return Module::SUCCESS;
}

static int NfsOpendirAsyncScan_stub(const char *path, nfs_cb cb, void *privateData, nfs_fh_scan &fh)
{
    cout << "NfsOpendirAsyncScan Stub Invoked" << endl;
    return Module::SUCCESS;
}

static int NfsStat64_stub(const char *path, struct nfs_stat_64 *st)
{
    std::cout << "NfsStat64 stub invoked" << std::endl;
    return Module::SUCCESS;
}

static void ScannerOpendirAsyncCb_stub(int status, struct nfs_context *nfs, void *nfsDir, void *privateData)
{
    std::cout << "ScannerOpendirAsyncCb stub invoked" << std::endl;
    return;
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
    scanConfig.isGetDirExtAcl = false;

    scanConfig.dFilter = ScanDirectoryFilter{FILTER_TYPE::DISABLED, {}};
    scanConfig.fFilter = ScanFileFilter{FILTER_TYPE::DISABLED, {}};

    scanConfig.nfs.m_serverIp = "10.28.xx.xx";
    scanConfig.nfs.m_serverPath = "/share1";
    scanConfig.nfs.maxOpendirReqCount = 200;
}

class NfsMetaProducerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    std::shared_ptr<StatisticsMgr> statsMgr {};
    std::shared_ptr<ScanQueue> scanQueue;
    std::shared_ptr<BufferQueue<DirStat>> input;
    std::shared_ptr<BufferQueue<DirectoryScan>> output;
    std::shared_ptr<ScanFilter> scanFilter;
    std::shared_ptr<FSScannerCheckPoint> chkPntMgr;
    ScanConfig scanConfig {};
    ScanInfo scanInfo {};
    std::string enqueuePath {"/"};
    Stub stub;
};

void NfsMetaProducerTest::SetUp()
{

    stub.set(ADDR(NfsContextWrapper, NfsService), NfsService_stub);
    stub.set(ADDR(NfsContextWrapper, NfsStat64), NfsStat64_stub);
    stub.set(ADDR(NfsContextWrapper, NfsOpendirAsync), NfsOpendirAsync_stub);
    stub.set(ADDR(NfsContextWrapper, NfsOpendirAsyncScan), NfsOpendirAsyncScan_stub);
    stub.set(ADDR(NfsContextWrapper, NfsV3GetAcl), NfsV3GetAcl_stub);
    stub.set(ADDR(NfsMetaProducer, ScannerOpendirAsyncCb), ScannerOpendirAsyncCb_stub);

    FillCommonScanConfig(scanConfig);
    statsMgr = make_shared<StatisticsMgr>();
    output = make_shared<BufferQueue<DirectoryScan>>(MAX_QUEUE_LIMIT);
    scanFilter = make_shared<ScanFilter>(scanConfig.dFilter, scanConfig.fFilter);
    chkPntMgr = make_shared<FSScannerCheckPoint>(scanConfig, scanInfo);
    input = make_shared<BufferQueue<DirStat>>(MAX_QUEUE_LIMIT);
    string scanOutputDir = scanConfig.metaPath + LATEST;
    scanQueue = make_shared<ScanQueue>(input, scanOutputDir, 10000, 8000);
}

void NfsMetaProducerTest::TearDown()
{}

void NfsMetaProducerTest::SetUpTestCase()
{}

void NfsMetaProducerTest::TearDownTestCase()
{}

TEST_F(NfsMetaProducerTest, NfsOpendirAsync)
{
    DirStat dirStat1 {};
    dirStat1.m_path = "/home/abc/pqr";
    dirStat1.m_filterFlag = 14;
    scanQueue->Push(dirStat1);

    NfsMetaProducer metaProducer(scanQueue, output, scanConfig, statsMgr, scanFilter, chkPntMgr);
    metaProducer.Produce(20);
}

TEST_F(NfsMetaProducerTest, NfsOpendirAsyncScan)
{
    DirStat dirStat1 {};
    dirStat1.m_path = "/home/abc/pqr";
    strcpy(dirStat1.m_fh.value, "abcdefghij");
    dirStat1.m_fh.len = 10;

    scanQueue->Push(dirStat1);
    NfsMetaProducer metaProducer(scanQueue, output, scanConfig, statsMgr, scanFilter, chkPntMgr);
    metaProducer.Produce(20);
}

TEST_F(NfsMetaProducerTest, SendDirToWriteQueue)
{
    DirStat dirStat1 {};
    dirStat1.m_path = "/root";
    dirStat1.m_filterFlag = FLAG_NON_RECURSIVE;
    scanQueue->Push(dirStat1);

    NfsMetaProducer metaProducer(scanQueue, output, scanConfig, statsMgr, scanFilter, chkPntMgr);
    metaProducer.Produce(20);
}