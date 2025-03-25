/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Author: w30029850
 * Create: 2022-08-26.
 */

#include <list>
#include <cstdio>
#include <fstream>  
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "Scanner.h"
#include "log/Log.h"
#include "ScannerTime.h"
#include "ScannerUtils.h"
#include "ScanMgr.h"
#include "NfsContextWrapper.h"
#include "NFSSyncCbData.h"

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
    constexpr auto MODULE = "NFS_FOLDER_TRAVERSAL_TEST";
    const std::string LOG_NAME = "NFS_FOLDER_TRAVERSAL_TEST.log";
    constexpr int ONE_GB = 1024 * 1024 * 1024;
    constexpr auto MAX_QUEUE_LIMIT = 100;

    /**
    * tmp/
    *   |--NfsScannerTest/
    *           |--meta/
    *           |--scan/
    *           |--fs/
    *           |--log/
    */
    const std::string PATH_FOR_TEST_CASE = "/tmp/NfsScannerTest";
    const std::string PATH_FOR_META = PATH_FOR_TEST_CASE + "/meta";
    const std::string PATH_FOR_CTRL = PATH_FOR_TEST_CASE+  "/scan";
    const std::string PATH_FOR_FS = PATH_FOR_TEST_CASE+  "/fs"; // to simulate a filesyetem
    const std::string LOG_PATH = PATH_FOR_TEST_CASE + "/log";

    const std::string NFS_ERROR_MSG = "NFS_ERR_MOCK";
}

static std::map<struct nfsdir*, std::string> opendirMap {};
static std::vector<struct nfsdirent> nfsDirents {};

static void GeneratedCopyCtrlFileCb(void *usrData, std::string ctrlFile)
{}

static void GeneratedHardLinkCtrlFileCb(void *usrData, std::string ctrlFile)
{}

static void GeneratedRfiCtrlFileCb(void *usrData, RfiCbStruct cbParam)
{}

void static SetupDirForTest()
{
    FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE);
    if (!FS_SCANNER::CreateDir(PATH_FOR_TEST_CASE)) {
        ERRLOG("failed to create dir %s", PATH_FOR_TEST_CASE.c_str());
    }
    if (!FS_SCANNER::CreateDir(PATH_FOR_META)) {
        ERRLOG("failed to create dir %s", PATH_FOR_META.c_str());
    }
    string metaLatest = PATH_FOR_META + "/latest";
    if (!FS_SCANNER::CreateDir(metaLatest)) {
        ERRLOG("failed to create dir %s", metaLatest.c_str());
    }
    if (!FS_SCANNER::CreateDir(PATH_FOR_CTRL)) {
        ERRLOG("failed to create dir %s", PATH_FOR_CTRL.c_str());
    }
    if (!FS_SCANNER::CreateDir(LOG_PATH)) {
        ERRLOG("failed to create dir %s", LOG_PATH.c_str());
    }
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

    scanConfig.scanType = ScanJobType::FULL;
    scanConfig.scanIO = IOEngine::LIBNFS;
    scanConfig.scanCheckPointEnable = false;
    scanConfig.lastBackupTime = 0;

    scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
    scanConfig.scanHardlinkResultCb = GeneratedHardLinkCtrlFileCb;
    scanConfig.rfiCtrlCb = GeneratedRfiCtrlFileCb;

    scanConfig.isGetDirExtAcl = true;
    scanConfig.isGetFileExtAcl = true;
}

static bool MonitorScanner(std::shared_ptr<Scanner> &scanner)
{
    do {
        auto status = scanner->GetStatus();
        if (status == SCANNER_STATUS::COMPLETED) {
            scanner->GetStatistics();
            break;
        }
        if (status == SCANNER_STATUS::ABORTED) {
            scanner->GetStatistics();
            break;
        }
        scanner->GetStatistics();
        sleep(10);
    } while (true);
    return true;
}

static void CreateFile(const std::string& path, const std::string output = "helloworld")
{
    std::ofstream file(path);
    file << output;
}

static void SetupFileTree(const std::map<std::string, std::vector<std::string>> &fs)
{
    FS_SCANNER::RemoveDir(PATH_FOR_FS);
    if (!FS_SCANNER::CreateDir(PATH_FOR_FS)) {
        ERRLOG("failed to create dir %s", PATH_FOR_FS.c_str());
    }
    
    for (const auto &entry: fs) {
        string realDirPath = PATH_FOR_FS + entry.first;
        if (!FS_SCANNER::CreateDir(realDirPath)) {
            ERRLOG("failed to create dir %s", realDirPath.c_str());
        }
        for (const std::string& filename: entry.second) {
            string realFilePath = realDirPath + "/" + filename;
            CreateFile(realFilePath);
        }
    }
}

std::string RecoverNfsPathToPosixTestPath(const std::string &nfsPath)
{
    string posixPath = nfsPath;
    if (posixPath.empty()) {
        posixPath = "/";
    }
    if (posixPath.front() != '/') {
        posixPath = string("/") + posixPath;
    }
    if (posixPath != "/") {
        posixPath = PATH_FOR_FS + posixPath;
    } else {
        posixPath = PATH_FOR_FS;
    }

    INFOLOG("recover nfs path %s to %s", nfsPath.c_str(), posixPath.c_str());
    return posixPath;
}

static void NfsService_stub(void *obj)
{}

static int NfsMount_stub_success(void *obj)
{
    INFOLOG("enter NfsMount_stub_success");
    return Module::SUCCESS;
}

static void NfsDestroyContext_stub_success(void *obj)
{
    INFOLOG("enter NfsDestroyContext_stub_success");
    return;
}

static int NfsStat64_stub_success(void *obj, const char *path, struct nfs_stat_64 *st)
{
    INFOLOG("enter NfsStat64_stub_success");
    return Module::SUCCESS;
}

static char *NfsGetError_stub(void *obj)
{
    INFOLOG("enter NfsGetError_stub");
    return const_cast<char*>(NFS_ERROR_MSG.c_str());
}

static void NfsV3GetAcl_stub(void *obj, struct RpcClient* client, struct nfsdirent* &nfsdirent, std::string &aclValue)
{
    aclValue = "DEMO ACL FOR NFS";
    INFOLOG("enter NfsV3GetAcl_stub");
}

static void NfsOpendirMock(const char *path, nfs_cb cb, void *privateData, int status)
{
    nfs_context* nfs = nullptr;
    string nfsPath = path;
    string posixPath = RecoverNfsPathToPosixTestPath(nfsPath);
    INFOLOG("mock nfs opendir %s to posix opendir %s", nfsPath.c_str(), posixPath.c_str());
    struct nfsdir *nfsDir = reinterpret_cast<struct nfsdir*>(opendir(posixPath.c_str()));
    opendirMap[nfsDir] = posixPath;

    if (status == 0) {
        struct opendir_cb_data callbackData {};
        callbackData.private_data = privateData;
        callbackData.eof = true;
        cb(status, nfs, nfsDir, &callbackData);
    } else {
        cb(status, nfs, nfsDir, privateData);
    }
}

static int NfsOpendirAsync_stub_success(void *obj, const char *path, nfs_cb cb, void *privateData)
{
    NfsOpendirMock(path, cb, privateData, 0);
    INFOLOG("enter NfsOpendirAsync_stub_success");
    return Module::SUCCESS;
}

static int NfsOpendirAsyncScan_stub_success(void *obj, const char *path, nfs_cb cb, void *privateData, nfs_fh_scan &fh)
{
    NfsOpendirMock(path, cb, privateData, 0);
    INFOLOG("enter NfsOpendirAsyncScan_stub_success");
    return Module::SUCCESS;
}

// set resData.m_status < 0 to invoke HandleFailedDirectory
static int NfsOpendirAsync_stub_failed(void *obj, const char *path, nfs_cb cb, void *privateData)
{
    NfsOpendirMock(path, cb, privateData, -1);
    INFOLOG("enter NfsOpendirAsync_stub_failed");
    return Module::SUCCESS;
}

// set resData.m_status < 0 to invoke HandleFailedDirectory
static int NfsOpendirAsyncScan_stub_failed(void *obj, const char *path, nfs_cb cb, void *privateData, nfs_fh_scan &fh)
{
    NfsOpendirMock(path, cb, privateData, -1);
    INFOLOG("enter NfsOpendirAsyncScan_stub_failed");
    return Module::SUCCESS;
}

static int NfsOpendirAsyncScanResume_stub_success(void *obj, void *privateData)
{
    INFOLOG("enter NfsOpendirAsyncScanResume_stub");
    return Module::SUCCESS;
}

static struct nfsdirent *NfsReadDir_stub(void *obj, struct nfsdir *nfsdir)
{
    INFOLOG("enter NfsReadDir_stub");
    DIR *dir = reinterpret_cast<DIR*>(nfsdir);
    struct dirent *posixDirent = readdir(dir);
    if (posixDirent != nullptr) {
        nfsDirents.emplace_back();
        auto &newNfsDirent = nfsDirents.back();
        struct nfsdirent *nfsDirent = &newNfsDirent;
        nfsDirent->name = posixDirent->d_name;
        if (posixDirent->d_type == DT_DIR) {
            INFOLOG("mock nfs readdir, return dir %s", nfsDirent->name);
            nfsDirent->mode = 0x4000;
        } else {
            INFOLOG("mock nfs readdir, return file %s", nfsDirent->name);
            nfsDirent->mode = 0;
        }
        return nfsDirent;
    }
    INFOLOG("mock nfs readdir, read completed");
    return nullptr;
}

static void NfsCloseDir_stub(void *obj, struct nfsdir *nfsDir)
{
    INFOLOG("enter NfsCloseDir_stub");
    DIR* posixDir = reinterpret_cast<DIR*>(nfsDir);
    closedir(posixDir);
}

class NfsScannerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
private:
    Stub stub;
};

void NfsScannerTest::SetUp()
{
    SetupDirForTest();

    stub.set(ADDR(NfsContextWrapper, NfsService), NfsService_stub);
    stub.set(ADDR(NfsContextWrapper, NfsMount), NfsMount_stub_success);
    stub.set(ADDR(NfsContextWrapper, NfsDestroyContext), NfsDestroyContext_stub_success);
    stub.set(ADDR(NfsContextWrapper, NfsStat64), NfsStat64_stub_success);
    stub.set(ADDR(NfsContextWrapper, NfsOpendirAsync), NfsOpendirAsync_stub_success);
    stub.set(ADDR(NfsContextWrapper, NfsOpendirAsyncScan), NfsOpendirAsyncScan_stub_success);
    stub.set(ADDR(NfsContextWrapper, NfsOpendirAsyncScanResume), NfsOpendirAsyncScanResume_stub_success);
    stub.set(ADDR(NfsContextWrapper, NfsGetError), NfsGetError_stub);
    stub.set(ADDR(NfsContextWrapper, NfsV3GetAcl), NfsV3GetAcl_stub);
    stub.set(ADDR(NfsContextWrapper, NfsReadDir), NfsReadDir_stub);
    stub.set(ADDR(NfsContextWrapper, NfsCloseDir), NfsCloseDir_stub);
}

void NfsScannerTest::TearDown()
{
    FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE);
    opendirMap.clear();
    nfsDirents.clear();
}

void NfsScannerTest::SetUpTestCase()
{}

void NfsScannerTest::TearDownTestCase()
{}

/*
 * 用例名称：Nfs扫描，正常场景
 * 前置条件：无
 * check点：正确生成meta/ctrl
 **/
TEST_F(NfsScannerTest, NfsScannerTest_No_Filter)
{
    // SetupFileTree({
    //     {
    //         "/", {"1.txt", "2.png", "3.jpg"}
    //     }, {
    //         "/dir1", {"11.txt", "22.png", "33.jpg"}
    //     }
    // });

    // ScanConfig scanConfig {};
    // FillCommonScanConfig(scanConfig);
    // std::shared_ptr<Scanner> scanner = ScanMgr::CreateScanInst(scanConfig);
    // scanner->Enqueue("");
    // ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    // EXPECT_TRUE(MonitorScanner(scanner));
    // scanner->Destroy();
    // scanner.reset();
}

/*
 * 用例名称：Nfs扫描，带过滤
 * 前置条件：无
 * check点：正确生成meta/ctrl
 **/
TEST_F(NfsScannerTest, NfsScannerTest_With_Filter)
{
    // SetupFileTree({
    //     {
    //         "/", {"1.txt", "2.png", "3.jpg"}
    //     }, {
    //         "/dir1", {"11.txt", "22.png", "33.jpg"}
    //     }, {
    //         "/dir1/dir2", {"111.txt", "222.png", "333.jpg"}
    //     }
    // });

    // ScanConfig scanConfig {};
    // FillCommonScanConfig(scanConfig);
    // scanConfig.dFilter = ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
    //     "/dir1/dir2"
    // }};
    // scanConfig.fFilter = ScanFileFilter{FILTER_TYPE::DISABLED, {}};

    // std::shared_ptr<Scanner> scanner = ScanMgr::CreateScanInst(scanConfig);
    // scanner->Enqueue("");
    // ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    // EXPECT_TRUE(MonitorScanner(scanner));
    // scanner->Destroy();
    // scanner.reset();
}

/*
 * 用例名称：Nfs扫描，异常场景
 * 前置条件：无
 * check点：
 **/
TEST_F(NfsScannerTest, NfsScannerTest_With_Error)
{
    SetupFileTree({
        {
            "/", {"1.txt", "2.png", "3.jpg"}
        }, {
            "/dir1", {"11.txt", "22.png", "33.jpg"}
        }
    });

    stub.set(ADDR(NfsContextWrapper, NfsOpendirAsync), NfsOpendirAsync_stub_failed);
    stub.set(ADDR(NfsContextWrapper, NfsOpendirAsyncScan), NfsOpendirAsyncScan_stub_failed);

    ScanConfig scanConfig {};
    FillCommonScanConfig(scanConfig);
    std::shared_ptr<Scanner> scanner = ScanMgr::CreateScanInst(scanConfig);
    scanner->Enqueue("");
    ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner));
    scanner->Destroy();
    scanner.reset();
}