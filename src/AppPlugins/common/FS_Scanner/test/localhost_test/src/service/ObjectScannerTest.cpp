/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Prouce meta&xmeta thread for object storage.
* Author: w00444223
* Create: 2023-12-14
*/

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"

#include "log/Log.h"
#include "common/Path.h"
#include "ScanMgr.h"
#include "ScannerUtils.h"
#include "ObjectInode.h"
#include "interface/CloudServiceInterface.h"
#include "CloudServiceTest.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace Module;

namespace {
    constexpr int ONE_GB = 1024 * 1024 * 1024;
    constexpr int MAX_QUEUE_LIMIT = 100;
    std::string SCAN_OUT_ROOT_PATH;
    std::string META_PATH;
    std::string CTRL_PATH;
    BackupTestPara g_testBackupPara;
}

class ObjectScannerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

static void TestCreateDir(std::string path)
{
    std::string cmd = "mkdir -p " + path;
    system(cmd.c_str());
}

static void TestDeleteDir(std::string path)
{
    std::string cmd = "rm -rf " + path;
    system(cmd.c_str());
}

static std::unique_ptr<CloudServiceInterface> CreateInstStub(const StorageConfig& storageConfig)
{
    if (storageConfig.storageType == StorageType::HUAWEI) {
        return std::make_unique<CloudServiceTest>(storageConfig.verifyInfo, g_testBackupPara);
    } else {
        return nullptr;
    }
}

void ObjectScannerTest::SetUp()
{
    SCAN_OUT_ROOT_PATH = Module::CPath::GetInstance().GetConfPath() + "/scan";
    META_PATH = SCAN_OUT_ROOT_PATH + "/meta";
    CTRL_PATH = SCAN_OUT_ROOT_PATH +  "/ctrl";
    TestCreateDir(META_PATH + "/latest");
    stub.set(&CloudServiceManager::CreateInst, CreateInstStub);
}

void ObjectScannerTest::TearDown()
{
    stub.reset(&CloudServiceManager::CreateInst);
    TestDeleteDir(SCAN_OUT_ROOT_PATH);
}

void ObjectScannerTest::SetUpTestCase()
{}

void ObjectScannerTest::TearDownTestCase()
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
    EXPECT_TRUE(ctrlFile.size() >= 0);
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

static std::string TestGetHashValue(std::string text)
{
    auto genInode = std::make_shared<ObjectInode>();
    return std::to_string(genInode->GetInodeValue(text.c_str(), text.length()));
}

static void GeneratedCopyCtrlFileCb(void *usrData, std::string ctrlFile)
{
    std::cout << "Generated copy ctrl file: " << ctrlFile << std::endl;
}

static void FillScanConfig(ScanConfig& scanConfig)
{
    scanConfig.jobId = "30016470";
    scanConfig.reqID = 30016470;

    scanConfig.scanType = ScanJobType::FULL;
    scanConfig.scanIO = IOEngine::OBJECTSTORAGE;
    scanConfig.lastBackupTime = 0;

    /* config meta path */
    scanConfig.metaPath =  META_PATH;
    scanConfig.metaPathForCtrlFiles = CTRL_PATH;

    /* 记录线程数 */
    scanConfig.maxCommonServiceInstance = 1;
    scanConfig.producerThreadCount = 1;
    scanConfig.scanCtrlMaxDataSize = "52428800"; // 50M
    scanConfig.scanCtrlMinDataSize = "1048576"; // 1M
    scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
}

static bool MonitorScanner(std::unique_ptr<Scanner> &scanner)
{
    INFOLOG("Enter Monitor Scanner");
    do {
        auto status = scanner->GetStatus();
        if (status == SCANNER_STATUS::COMPLETED) {
            return true;
        }
        if (status == SCANNER_STATUS::ABORTED) {
            return false;
        }
        sleep(1);
    } while (true);

    INFOLOG("Exit Monitor Scanner");
    return true;
}

TEST_F(ObjectScannerTest, TestStartScanner)
{
    ScanConfig scanConfig {};
    FillScanConfig(scanConfig);
    ObjectStorageBucket bucket{};
    bucket.bucketName = "hcs-test-10";
    scanConfig.obs.buckets.emplace_back(bucket);
    scanConfig.obs.authArgs.storageType = StorageType::HUAWEI;

    auto scanner = ScanMgr::CreateScanInst(scanConfig);
    EXPECT_TRUE(scanner != nullptr);
    EXPECT_TRUE(scanner->Start() == SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner));
    scanner->Destroy();
    scanner.reset();

    // 检查扫描结果的control文件内容
    std::vector<std::string> ctrlFile1 {};
    TestGetCtrlFile(CTRL_PATH, ctrlFile1);
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile1, TestGetHashValue("file11")));
    EXPECT_TRUE(TestCheckCtrlFileCont(ctrlFile1, TestGetHashValue("file12")));

    // 删除control文件
    std::string cmd = "rm -f " + CTRL_PATH + "/*";
    system(cmd.c_str());

    // 第二步：细细粒度恢复扫描
    scanConfig.scanType = ScanJobType::CONTROL_GEN;
    scanConfig.generatorIsFull = true;
    scanConfig.curDcachePath = META_PATH + "/latest";
    scanConfig.dCtrlFltr = { "/hcs-test-10/" + TestGetHashValue("/subdir1/") };
    scanConfig.fCtrlFltr = { "/hcs-test-10/" + TestGetHashValue("/subdir2/") + "/" + TestGetHashValue("file31") };
    scanner = ScanMgr::CreateScanInst(scanConfig);
    EXPECT_TRUE(scanner != nullptr);
    EXPECT_TRUE(scanner->Start() == SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner));
    scanner->Destroy();
    scanner.reset();

    // 检查扫描结果的control文件内容
    std::vector<std::string> ctrlFile2 {};
    TestGetCtrlFile(CTRL_PATH, ctrlFile2);
    EXPECT_FALSE(TestCheckCtrlFileCont(ctrlFile2, TestGetHashValue("file11")));
    EXPECT_FALSE(TestCheckCtrlFileCont(ctrlFile2, TestGetHashValue("file12")));
}
