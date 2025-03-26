/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Author: w30029850
 * Create: 2022-08-26.
 */

#include <list>
#include <cstdio>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/acl.h>
#include <dirent.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "Scanner.h"
#include "log/Log.h"
#include "ScannerTime.h"
#include "ScannerUtils.h"
#include "ScanMgr.h"
#include "SmbContextWrapper.h"

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
    constexpr auto MODULE = "SMB_FOLDER_TRAVERSAL_TEST";
    const std::string LOG_NAME = "SMB_FOLDER_TRAVERSAL_TEST.log";
    constexpr int ONE_GB = 1024 * 1024 * 1024;
    constexpr auto MAX_QUEUE_LIMIT = 100;

    /**
    * tmp/
    *   |--SmbScannerTest/
    *           |--meta/
    *           |--scan/
    *           |--fs/
    *           |--log/
    */
    const std::string PATH_FOR_TEST_CASE = "/tmp/SmbScannerTest";
    const std::string PATH_FOR_META = PATH_FOR_TEST_CASE + "/meta";
    const std::string PATH_FOR_CTRL = PATH_FOR_TEST_CASE+  "/scan";
    const std::string PATH_FOR_FS = PATH_FOR_TEST_CASE+  "/fs"; // to simulate a filesyetem
    const std::string LOG_PATH = PATH_FOR_TEST_CASE + "/log";

    const std::string MOCK_SMB_ERR = "MOCK_SMB_ERR";
    const wchar_t MOCK_WCHAR[] = L"MOCK WCHAR ACL";
}

static std::map<struct smb2dir*, std::string> opendirMap {};
static std::vector<struct smb2dirent> smbDirents {};

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
    scanConfig.scanIO = IOEngine::LIBSMB2;
    scanConfig.scanCheckPointEnable = false;
    scanConfig.lastBackupTime = 0;

    scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
    scanConfig.scanHardlinkResultCb = GeneratedHardLinkCtrlFileCb;
    scanConfig.rfiCtrlCb = GeneratedRfiCtrlFileCb;
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

std::string RecoverSmbPathToPosixTestPath(const std::string &nfsPath)
{
    string posixPath = nfsPath;
    if (posixPath.empty() || posixPath == ".") {
        posixPath = "/";
    }
    if (posixPath.front() == '.') {
        posixPath = posixPath.substr(1);
    }
    if (posixPath.front() != '/') {
        posixPath = string("/") + posixPath;
    }
    if (posixPath != "/") {
        posixPath = PATH_FOR_FS + posixPath;
    } else {
        posixPath = PATH_FOR_FS;
    }

    INFOLOG("recover smb path %s to %s", nfsPath.c_str(), posixPath.c_str());
    return posixPath;
}


static bool Init_stub_success(void *obj)
{
    INFOLOG("enter SmbContextWrapper Init_stub_success");
    return true;
}

static bool SmbConnect_stub_success(void *obj)
{
    INFOLOG("enter SmbContextWrapper SmbConnect_stub_success");
    return true;
}

static std::string SmbGetError_stub(void *obj)
{
    INFOLOG("enter SmbContextWrapper SmbGetError_stub");
    return "SMB MOCK ERR";
}

static void SmbProcess_stub(void *obj)
{}

static void SmbCloseDir_stub(void *obj, struct smb2dir *smb2dir)
{
    INFOLOG("enter SmbContextWrapper SmbCloseDir_stub");
    DIR* posixDir = reinterpret_cast<DIR*>(smb2dir);
    closedir(posixDir);
}

static int SmbStat64_stub_success(void *obj, const char *path, struct smb2_stat_64 *st)
{
    INFOLOG("enter SmbContextWrapper SmbStat64_stub_success");
    return Module::SUCCESS;
}

static int SmbGetSd_stub_success(void *obj, const char *path, wchar_t **sdstr)
{
    INFOLOG("enter SmbContextWrapper SmbGetSd_stub_success");    
    wchar_t *tmp_wchar = new wchar_t[100];
    memcpy(tmp_wchar, MOCK_WCHAR, sizeof(MOCK_WCHAR));
    *sdstr = tmp_wchar;
    return Module::SUCCESS;
}

static const char* smb2_get_error_stub(struct smb2_context *smb)
{
    return MOCK_SMB_ERR.c_str();
}

static void smb2_destroy_context_stub(struct smb2_context *smbcontext)
{
    INFOLOG("enter SmbContextWrapper smb2_destroy_context_stub"); 
}

static string SmbGetClientGuid_stub()
{
    INFOLOG("enter SmbContextWrapper SmbGetClientGuid_stub");
    return "SMB_MOCK_CLIENT_GUID";
}

static int SmbGetInfoAsync_stub(void* obj, const char *path, struct SMB2_ALL_INFO *allInfo,
    smb2_command_cb cb, void *privateData)
{
    INFOLOG("enter SmbContextWrapper SmbGetInfoAsync_stub, path = %s", path);
    smb2_context* smb = nullptr;
    int status = 0;

    allInfo->Name = new uint8_t[10];
    char name[10] = "Demo";
    memcpy((void*)(allInfo->Name), name, sizeof(name));

    SmbGetSd_stub_success(nullptr, nullptr, &allInfo->SecurityDescriptor);
    cb(smb, status, allInfo, privateData);
    return Module::SUCCESS;
}

static struct smb2dirent *SmbReadDir_stub(void *obj, struct smb2dir *smb2dir)
{
    INFOLOG("enter SmbContextWrapper SmbReadDir_stub");
    DIR *dir = reinterpret_cast<DIR*>(smb2dir);
    struct dirent *posixDirent = readdir(dir);
    if (posixDirent != nullptr) {
        smbDirents.emplace_back();
        auto &newSmbDirent = smbDirents.back();
        struct smb2dirent *smbDirent = &newSmbDirent;
        smbDirent->name = posixDirent->d_name;
        if (posixDirent->d_type == DT_DIR) {
            INFOLOG("mock smb readdir, return dir %s", smbDirent->name);
            smbDirent->st.smb2_type == SMB2_TYPE_DIRECTORY;
        } else {
            INFOLOG("mock smb readdir, return file %s", smbDirent->name);
            smbDirent->st.smb2_type == SMB2_TYPE_FILE;
        }
        return smbDirent;
    }
    INFOLOG("mock smb readdir, read completed");
    return nullptr;
}

static void SmbOpendirMock(const char *path, smb2_command_cb cb, void *privateData, int status)
{
    smb2_context* smb = nullptr;

    string smbPath = path;
    string posixPath = RecoverSmbPathToPosixTestPath(smbPath);
    INFOLOG("mock smb opendir %s to posix opendir %s", smbPath.c_str(), posixPath.c_str());
    
    struct smb2dir *smbDir = reinterpret_cast<struct smb2dir*>(opendir(posixPath.c_str()));
    opendirMap[smbDir] = posixPath;
    cb(smb, status, smbDir, privateData);
}

static int SmbOpendirAsync_stub_success(void *obj, const char *path, smb2_command_cb cb, void *privateData)
{
    SmbOpendirMock(path, cb, privateData, 0);
    INFOLOG("enter SmbContextWrapper SmbOpendirAsync_stub_success");
    return Module::SUCCESS;
}


class SmbScannerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
private:
    Stub stub;
};

void SmbScannerTest::SetUp()
{
    SetupDirForTest();

    stub.set(ADDR(SmbContextWrapper, Init), Init_stub_success);
    stub.set(ADDR(SmbContextWrapper, SmbConnect), SmbConnect_stub_success);
    stub.set(ADDR(SmbContextWrapper, SmbGetError), SmbGetError_stub);
    stub.set(ADDR(SmbContextWrapper, SmbProcess), SmbProcess_stub);
    stub.set(ADDR(SmbContextWrapper, SmbCloseDir), SmbCloseDir_stub);
    stub.set(ADDR(SmbContextWrapper, SmbStat64), SmbStat64_stub_success);
    stub.set(ADDR(SmbContextWrapper, SmbGetSd), SmbGetSd_stub_success);
    stub.set(ADDR(SmbContextWrapper, SmbGetInfoAsync), SmbGetInfoAsync_stub);
    stub.set(ADDR(SmbContextWrapper, SmbReadDir), SmbReadDir_stub);
    stub.set(ADDR(SmbContextWrapper, SmbOpendirAsync), SmbOpendirAsync_stub_success);
    stub.set(ADDR(SmbContextWrapper, SmbGetClientGuid), SmbGetClientGuid_stub);
    stub.set(smb2_destroy_context, smb2_destroy_context_stub);
    stub.set(smb2_get_error, smb2_get_error_stub);
}

void SmbScannerTest::TearDown()
{
    FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE);
    opendirMap.clear();
    smbDirents.clear();
}

void SmbScannerTest::SetUpTestCase()
{}

void SmbScannerTest::TearDownTestCase()
{}

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

/*
 * 用例名称：smb扫描，正常场景
 * 前置条件：无
 * check点：正确生成meta/ctrl
 **/
TEST_F(SmbScannerTest, SmbScannerTest_No_Filter)
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
    // scanner->Enqueue(".");
    // ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    // EXPECT_TRUE(MonitorScanner(scanner));
    // scanner->Destroy();
    // scanner.reset();
}


/*
 * 用例名称：smb扫描，带过滤
 * 前置条件：无
 * check点：正确生成meta/ctrl
 **/
TEST_F(SmbScannerTest, SmbScannerTest_With_Filter)
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
