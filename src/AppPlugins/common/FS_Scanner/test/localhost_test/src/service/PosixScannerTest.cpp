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
#include "LibScanner.h"

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
    constexpr auto MODULE = "POSIX_FOLDER_TRAVERSAL_TEST";
    const std::string LOG_NAME = "POSIX_FOLDER_TRAVERSAL_TEST.log";
    constexpr int ONE_GB = 1024 * 1024 * 1024;
    constexpr auto MAX_QUEUE_LIMIT = 100;
    constexpr auto JOB_ID = "114514";

    /**
    * tmp/
    *   |--PosixScannerTest/
    *           |--meta/
    *           |--scan/
    *           |--fs/
    *           |--log/
    */
    const std::string PATH_FOR_TEST_CASE = "/tmp/PosixScannerTest";
    const std::string PATH_FOR_META = PATH_FOR_TEST_CASE + "/meta";
    const std::string PATH_FOR_CTRL = PATH_FOR_TEST_CASE+  "/scan";
    const std::string PATH_FOR_FS = PATH_FOR_TEST_CASE+  "/fs"; // to simulate a filesyetem
    const std::string LOG_PATH = PATH_FOR_TEST_CASE + "/log";
}

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
    scanConfig.jobId = JOB_ID;
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
    scanConfig.scanIO = IOEngine::POSIX;
    scanConfig.scanCheckPointEnable = false;
    scanConfig.lastBackupTime = 0;

    scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
    scanConfig.scanHardlinkResultCb = GeneratedHardLinkCtrlFileCb;
    scanConfig.rfiCtrlCb = GeneratedRfiCtrlFileCb;
}

static bool MonitorScanner(std::shared_ptr<Scanner> &scanner)
{
    do {
        SCANNER_STATUS status = scanner->GetStatus();
        if (status == SCANNER_STATUS::COMPLETED) {
            scanner->GetStatistics();
            return true;
        }
        if (status == SCANNER_STATUS::ABORTED) {
            scanner->GetStatistics();
            return false;
        }
        scanner->GetStatistics();
        sleep(1);
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

class PosixScannerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
private:
    Stub stub;
};

void PosixScannerTest::SetUp()
{
    SetupDirForTest();
}

void PosixScannerTest::TearDown()
{
    FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE);
}

void PosixScannerTest::SetUpTestCase()
{}

void PosixScannerTest::TearDownTestCase()
{}

static void TestGetCtrlFile(std::string ctrlPath, std::vector<string>& ctrlFile)
{
    std::vector<string> fileList {};
    EXPECT_TRUE(FS_SCANNER::GetFileListInDirectory(ctrlPath, fileList));
    EXPECT_TRUE(fileList.size() != 0);
    for (auto &item : fileList) {
        EXPECT_TRUE(item.find("txt.tmp") == std::string::npos);
        if (item.find("control_") != std::string::npos) {
            ctrlFile.push_back(item);
        }
    }
    EXPECT_TRUE(ctrlFile.size() > 0);
}

static bool TestCheckCtrlFileCont(std::vector<string>& ctrlFile, std::string keyCont)
{
    for (auto &fileName : ctrlFile) {
        std::vector<std::string> fileContent {};
        EXPECT_TRUE(FS_SCANNER::ReadFile(fileName, fileContent));
        for (auto &line : fileContent) {
            if (line.find(keyCont) != string::npos) {
                return true;
            }
        }
    }
    return false;
}

/*
 * 用例名称：主机场景扫描
 * 前置条件：无
 * check点：正确生成meta/ctrl
 **/
TEST_F(PosixScannerTest, PosixScannerTest_No_Filter)
{
    SetupFileTree({
        {
            "/home", {}
        }, {
            "/home/user1", {"1.txt", "2.txt", "3.txt"}
        }, {
            "/home/user2", {"1.png", "2.png", "3.png"}
        }, {
            "/home/user3", {"1.jpg", "2.jpg", "3.jpg"}
        }, {
            "/etc", {}
        }, {
            "/etc/apache", {"apache.conf"}
        }
    });

    const std::string jobId = "JOBID";
    ScanConf scanConf {};
    scanConf.jobId = const_cast<char*>(jobId.c_str());
    scanConf.metaPath = const_cast<char*>(PATH_FOR_META.c_str());
    scanConf.metaPathForCtrlFiles = const_cast<char*>(PATH_FOR_CTRL.c_str());
    
    Scanner* scanner = static_cast<Scanner*>(CreateScannerInst(scanConf));
    std::vector<std::string> enqueueList {
        "/etc/apache/apache.conf",
        "/home"
    };
    for (auto& path : enqueueList) {
        scanner->Enqueue(PATH_FOR_FS + path, PATH_FOR_FS);
    }

    ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner, jobId.c_str()));
    GetStatistics(scanner);
    DestroyScannerInst(scanner);
}

/*
 * 用例名称：主机场景扫描带过滤
 * 前置条件：无
 * check点：正确生成meta/ctrl
 **/
TEST_F(PosixScannerTest, PosixScannerTest_WithFilter)
{
    SetupFileTree({
        {
            "/home", {}
        }, {
            "/home/user1", {"1.txt", "2.txt", "3.txt"}
        }, {
            "/home/user2", {"1.png", "2.png", "3.png"}
        }, {
            "/home/user3", {"1.jpg", "2.jpg", "3.jpg"}
        }, {
            "/etc", {}
        }, {
            "/etc/apache", {"apache.conf"}
        }
    });

    ScanConfig scanConfig {};
    FillCommonScanConfig(scanConfig);
    scanConfig.dFilter = ScanDirectoryFilter{FILTER_TYPE::EXCLUDE, {"/var"}};
    scanConfig.fFilter = ScanFileFilter{FILTER_TYPE::DISABLED, {}};

    std::shared_ptr<Scanner> scanner = ScanMgr::CreateScanInst(scanConfig);
    std::vector<std::string> enqueueList {
        "/etc/apache/apache.conf",
        "/home"
    };
    for (auto& path : enqueueList) {
        scanner->Enqueue(PATH_FOR_FS + path, PATH_FOR_FS);
    }
    ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner));
    scanner->Destroy();
}

/*
 * 用例名称：主机场景扫描带过滤
 * 前置条件：无
 * check点：正确生成meta/ctrl
 **/
TEST_F(PosixScannerTest, PosixScannerTest_WithFilterV2)
{
    SetupFileTree({
        {
            "/home", {}
        }, {
            "/home/user1", {"1.txt", "2.txt", "3.txt"}
        }, {
            "/home/user2", {"1.png", "2.png", "3.png"}
        }, {
            "/home/user3", {"1.jpg", "2.jpg", "3.jpg"}
        }, {
            "/etc", {}
        }, {
            "/etc/apache", {"apache.conf"}
        }
    });

    ScanConfig scanConfig {};
    FillCommonScanConfig(scanConfig);
    scanConfig.enableV2 = true;
    scanConfig.pathMapper = std::make_shared<PrefixSnapshotPathMapper>(PATH_FOR_FS);
    scanConfig.dFilter = ScanDirectoryFilter{FILTER_TYPE::EXCLUDE, {"/var"}};
    scanConfig.fFilter = ScanFileFilter{FILTER_TYPE::DISABLED, {}};

    std::shared_ptr<Scanner> scanner = ScanMgr::CreateScanInst(scanConfig);
    std::vector<std::string> enqueueList {
        "/etc/apache/apache.conf",
        "/home"
    };
    for (auto& path : enqueueList) {
        scanner->EnqueueV2(PATH_FOR_FS + path);
    }
    ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner));
    scanner->Destroy();
}

/*
 * 用例名称：主机场景扫描，增量生成ctrlfile
 * 前置条件：无
 * check点：正确生成meta/ctrl
 **/
TEST_F(PosixScannerTest, PosixScannerTest_IncCacheDiff)
{
    // first full scan
    SetupFileTree({
        { "/home", {} },
        { "/home/user1", {"1.txt", "2.txt", "3.txt"} },
        { "/home/user2", {"1.png", "2.png", "3.png"} }
    });

    ScanConfig scanConfig {};
    FillCommonScanConfig(scanConfig);
    std::shared_ptr<Scanner> scanner = ScanMgr::CreateScanInst(scanConfig);
    std::vector<std::string> enqueueList { "/home" };
    for (auto& path : enqueueList) {
        scanner->Enqueue(PATH_FOR_FS + path, PATH_FOR_FS);
    }
    ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner));
    scanner->Destroy();

    // second inc scan, delete /home/user1, /home/user2/1.png, add /home/user3
    FS_SCANNER::Rename(PATH_FOR_META + "/latest", PATH_FOR_META + "/previous");
    SetupFileTree({
        { "/home", {} },
        { "/home/user2", {"2.png", "3.png"} },
        { "/home/user3", {"1.jpg", "2.jpg", "3.jpg"}}
    });
    scanConfig.scanType = ScanJobType::INC;
    scanner = ScanMgr::CreateScanInst(scanConfig);
    enqueueList = { "/home" };
    for (auto& path : enqueueList) {
        scanner->Enqueue(PATH_FOR_FS + path, PATH_FOR_FS);
    }
    ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner));
    scanner->Destroy();
}

TEST_F(PosixScannerTest, PosixScannerTest_FineGrainedRestore)
{
    // 第一步：全量扫描
    SetupFileTree({
        { "/home", {} },
        { "/home/user1", {"1.txt", "2.txt", "3.txt"} },
        { "/home/user2", {"1.png", "2.png", "3.png"} }
    });

    ScanConfig scanConfig {};
    FillCommonScanConfig(scanConfig);
    std::shared_ptr<Scanner> scanner = ScanMgr::CreateScanInst(scanConfig);
    std::vector<std::string> enqueueList { "/home" };
    for (auto& path : enqueueList) {
        scanner->Enqueue(PATH_FOR_FS + path, PATH_FOR_FS);
    }
    ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner));
    scanner->Destroy();
    scanner.reset();

    std::vector<std::string> ctrlFile1 {};
    TestGetCtrlFile(PATH_FOR_CTRL, ctrlFile1);
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile1, "1.txt"));
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile1, "2.txt"));
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile1, "3.txt"));
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile1, "1.png"));
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile1, "2.png"));
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile1, "3.png"));

    // 删除control文件
    std::string cmd = "rm -f " + PATH_FOR_CTRL + "/*";
    system(cmd.c_str());

    // 第二步：细细粒度恢复扫描
    scanConfig.scanType = ScanJobType::CONTROL_GEN;
    scanConfig.generatorIsFull = true;
    scanConfig.curDcachePath = PATH_FOR_META + "/latest";
    scanConfig.dCtrlFltr = { "/home/user1/" };
    scanConfig.fCtrlFltr = { "/home/user2/1.png" };

    scanner = ScanMgr::CreateScanInst(scanConfig);
    ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner));
    scanner->Destroy();
    scanner.reset();

    std::vector<std::string> ctrlFile2 {};
    TestGetCtrlFile(PATH_FOR_CTRL, ctrlFile2);
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile2, "1.txt"));
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile2, "2.txt"));
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile2, "3.txt"));
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile2, "1.png"));
    EXPECT_FALSE(TestCheckCtrlFileCont(ctrlFile2, "2.png"));
    EXPECT_FALSE(TestCheckCtrlFileCont(ctrlFile2, "3.png"));
}