#include "gtest/gtest.h"
#include "LibScanner.h"
#include "ScanConsts.h"
#include "ScannerImpl.h"
#include "stub.h"

using namespace std;
using namespace Module;
using namespace FS_SCANNER;

namespace {
    /**
    * tmp/
    *   |--PosixUtilsTest/
    *           |--meta/
    *           |--scan/
    *           |--fs/
    *           |--log/
    */
    const std::string PATH_FOR_TEST_CASE = "/tmp/LibScannerTest";
    const std::string PATH_FOR_META = PATH_FOR_TEST_CASE + "/meta";
    const std::string PATH_FOR_CTRL = PATH_FOR_TEST_CASE+  "/scan";
    const std::string PATH_FOR_FS = PATH_FOR_TEST_CASE+  "/fs"; // to simulate a filesyetem
    const std::string LOG_PATH = PATH_FOR_TEST_CASE + "/log";
}

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

class LibScannerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<ControlGenerator> m_ins {nullptr};
};

void LibScannerTest::SetUp()
{
    
}

void LibScannerTest::TearDown()
{}

void LibScannerTest::SetUpTestCase()
{}

void LibScannerTest::TearDownTestCase()
{
    FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE);
}

static SCANNER_STATUS Stub_SUCCESS(void* obj)
{
    return SCANNER_STATUS::SUCCESS;
}

static SCANNER_STATUS Stub_FAILED(void* obj)
{
    return SCANNER_STATUS::FAILED;
}

TEST_F(LibScannerTest, StartScannerBatch)
{
    ScanConf scanConf {};
    scanConf.jobId = "job123";
    scanConf.metaPath = "/home/test/meta";
    scanConf.metaPathForCtrlFiles = "/home/test/meta/controlfile";
    void* scannerHandle = CreateScannerInst(scanConf);
    char* dirPathList = "/home/postgres/data/aaaa;/home/postgres/data/bbbbb";
    typedef int (*fptr)(ScannerImpl*);
    fptr ScannerImpl_Start = (fptr)(&ScannerImpl::Start);
    fptr ScannerImpl_Enqueue = (fptr)(&ScannerImpl::Enqueue);
    stub.set(ScannerImpl_Start, Stub_SUCCESS);
    stub.set(ScannerImpl_Enqueue, Stub_SUCCESS);
    // 正常场景
    EXPECT_TRUE(StartScannerBatch(scannerHandle, dirPathList));
    stub.set(ScannerImpl_Start, Stub_FAILED);
    // 扫描开始返回失败
    EXPECT_FALSE(StartScannerBatch(scannerHandle, dirPathList));
    dirPathList = "";
    // 扫描列表为空
    EXPECT_FALSE(StartScannerBatch(scannerHandle, dirPathList));
    free(scannerHandle);
    scannerHandle = nullptr;
    // scannerHandle为空
    EXPECT_FALSE(StartScannerBatch(scannerHandle, dirPathList));
}

/*
 * 用例名称：InitLog
 * 前置条件：无
 * check点：初始化日志
 **/
TEST_F(LibScannerTest, InitLog)
{
    char* fullLogPath = "/tmp";
    int argc = 1;
    InitLog(fullLogPath, argc);
}

/*
 * 用例名称：StartScanner
 * 前置条件：无
 * check点：开始扫描
 **/
TEST_F(LibScannerTest, StartScanner)
{
    DestroyScannerInst(nullptr);
    GetStatistics(nullptr);

    const char *dirPath = (PATH_FOR_FS + "/").c_str();  
    bool ret = StartScanner(nullptr, dirPath);
    EXPECT_EQ(ret, false);

    const std::string jobId = "JOBID";
    ScanConf scanConf {};
    scanConf.jobId = const_cast<char*>(jobId.c_str());
    scanConf.metaPath = const_cast<char*>(PATH_FOR_META.c_str());
    scanConf.metaPathForCtrlFiles = const_cast<char*>(PATH_FOR_CTRL.c_str());
    ret = StartScanner(CreateScannerInst(scanConf), dirPath);
    EXPECT_EQ(ret, true);

    typedef int (*fptr)(ScannerImpl*);
    fptr ScannerImpl_Start = (fptr)(&ScannerImpl::Start);
    stub.set(ScannerImpl_Start, Stub_FAILED);
    ret = StartScanner(CreateScannerInst(scanConf), dirPath);
    EXPECT_EQ(ret, false);
}

static string Stub_Query(std::string jobId)
{
    return "/tmp/LibScannerTest/fs/home/user1";
}
static string Stub_Query_Empty(std::string jobId)
{
    return "";
}
static SCANNER_STATUS Stub_GetStatus_ABORTED()
{
    return SCANNER_STATUS::ABORTED;
}
static SCANNER_STATUS Stub_Abort_ABORTED()
{
    return SCANNER_STATUS::SUCCESS;
}
/*
 * 用例名称：MonitorScanner
 * 前置条件：无
 * check点：监控扫描
 **/
TEST_F(LibScannerTest, MonitorScanner)
{

    char *jobId = "JOBID";
    MonitorScanner(nullptr, jobId);

    SetupDirForTest(); // 创建目录用于Test
    SetupFileTree({ // 创建文件树
        {
            "/home", {}
        }, {
            "/home/user1", {"Abort.signal"}
        }
    });

    typedef int (*fptr)(ScannerImpl*);
    fptr ScannerImpl_GetStatus = (fptr)(&ScannerImpl::GetStatus);
    stub.set(ScannerImpl_GetStatus, Stub_GetStatus_ABORTED);
    stub.set(ADDR(ScanTaskInfo, Query), Stub_Query);
    ScanConf scanConf {};
    scanConf.jobId = jobId;
    scanConf.metaPath = const_cast<char*>(PATH_FOR_META.c_str());
    scanConf.metaPathForCtrlFiles = const_cast<char*>(PATH_FOR_CTRL.c_str());
    
    bool ret = MonitorScanner(CreateScannerInst(scanConf), jobId);
    EXPECT_EQ(ret, false);
    stub.reset(ADDR(ScanTaskInfo, Query));

    fptr ScannerImpl_Abort = (fptr)(&ScannerImpl::Abort);
    stub.set(ScannerImpl_Abort, Stub_Abort_ABORTED);
    ret = MonitorScanner(CreateScannerInst(scanConf), jobId);
    EXPECT_EQ(ret, false);
    stub.reset(ADDR(ScanTaskInfo, Query));
    stub.reset(ScannerImpl_Abort);

    stub.set(ADDR(ScanTaskInfo, Query), Stub_Query_Empty);
    ret = MonitorScanner(CreateScannerInst(scanConf), jobId);
    EXPECT_EQ(ret, false);
    stub.reset(ADDR(ScanTaskInfo, Query)); 
}