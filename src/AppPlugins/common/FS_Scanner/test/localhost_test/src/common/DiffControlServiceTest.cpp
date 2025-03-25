#include "DiffControlService.h"
#include "ScannerUtils.h"
#include "ScanConsts.h"
#include "Scanner.h"
#include "ScanMgr.h"
#include "LibScanner.h"
#include "system/System.hpp"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

using namespace std;
using namespace Module;

namespace {
    constexpr auto MODULE = "POSIX_FOLDER_TRAVERSAL_TEST";
    const std::string LOG_NAME = "POSIX_FOLDER_TRAVERSAL_TEST.log";
    constexpr int ONE_GB = 1024 * 1024 * 1024;
    constexpr auto MAX_QUEUE_LIMIT = 100;
    constexpr int SCANNER_SUCCESS = 0;
    constexpr int SCANNER_FAILED = -1;

    /**
    * tmp/
    *   |--PosixScannerTest/
    *           |--meta/
    *           |--ctrl/
    *           |--rfi/
    *           |--fs/
    *           |--log/
    */
    const std::string PATH_FOR_TEST_CASE = "/tmp/DiffControlServiceTest";
    const std::string PATH_FOR_META = PATH_FOR_TEST_CASE + "/meta";
    const std::string PATH_FOR_CTRL = PATH_FOR_TEST_CASE+  "/ctrl";
    const std::string PATH_FOR_RFI = PATH_FOR_TEST_CASE+  "/rfi";
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
    if (!FS_SCANNER::CreateDir(PATH_FOR_FS)) {
        ERRLOG("failed to create dir %s", PATH_FOR_FS.c_str());
    }
    if (!FS_SCANNER::CreateDir(PATH_FOR_CTRL)) {
        ERRLOG("failed to create dir %s", PATH_FOR_CTRL.c_str());
    }
    if (!FS_SCANNER::CreateDir(PATH_FOR_RFI)) {
        ERRLOG("failed to create dir %s", PATH_FOR_RFI.c_str());
    }
    if (!FS_SCANNER::CreateDir(LOG_PATH)) {
        ERRLOG("failed to create dir %s", LOG_PATH.c_str());
    }
}

static void CreateFile(const std::string& path, const std::string output = "helloworld")
{
    std::ofstream file(path);
    file << output;
    file.close();
}

static void SetupFileTree(const std::map<std::string, std::vector<std::string>> &fs,
    const std::map<std::string, std::vector<std::string>> &rm, bool removeFlag = true)
{
    if (removeFlag) {
        FS_SCANNER::RemoveDir(PATH_FOR_FS);
    }
    if (!FS_SCANNER::PathExist(PATH_FOR_FS) && !FS_SCANNER::CreateDir(PATH_FOR_FS)) {
        ERRLOG("failed to create dir %s", PATH_FOR_FS.c_str());
    }
    
    for (const auto &entry: fs) {
        string realDirPath = PATH_FOR_FS + entry.first;
        if (!FS_SCANNER::PathExist(realDirPath) && !FS_SCANNER::CreateDir(realDirPath)) {
            ERRLOG("failed to create dir %s", realDirPath.c_str());
        }
        for (const std::string& filename: entry.second) {
            string realFilePath = realDirPath + "/" + filename;
            CreateFile(realFilePath);
        }
    }
    for (const auto &entry: rm) {
        string realDirPath = PATH_FOR_FS + entry.first;
        if (!FS_SCANNER::PathExist(realDirPath) && !FS_SCANNER::CreateDir(realDirPath)) {
            ERRLOG("failed to create dir %s", realDirPath.c_str());
        }
        for (const std::string& filename: entry.second) {
            if (filename.empty()) {
                FS_SCANNER::RemoveDir(realDirPath);
                break;
            }
            string realFilePath = realDirPath + "/" + filename;
            FS_SCANNER::RemoveFile(realFilePath);
        }
    }
}

static void CleanDirAndFile()
{
    FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE); // 删除DiffControlServiceTest
}

static void GeneratedCopyCtrlFileCb(void *usrData, std::string ctrlFile)
{
    cout << "Generated copy ctrl file: " << ctrlFile << endl;
}

static void GeneratedHardLinkCtrlFileCb(void *usrData, std::string ctrlFile)
{
    cout << "Generated hard link ctrl file: " << ctrlFile << endl;
}

static void GeneratedRfiCtrlFileCb(void *usrData, RfiCbStruct cbParam)
{
    cout << "generate rfi file : " << cbParam.rfiZipFileName << endl;
}

static void FillScanConfig(ScanConfig& scanConfig)
{
    HCP_Log(INFO, MODULE) << "Enter FillScanConfig" << HCPENDLOG;

    scanConfig.jobId = "123456";
    scanConfig.reqID = 123456;

    scanConfig.scanCheckPointEnable = false;

    scanConfig.scanType = ScanJobType::FULL;
    scanConfig.scanIO = IOEngine::POSIX;
    scanConfig.lastBackupTime = 0;

    /* config meta path */
    scanConfig.metaPath =  PATH_FOR_META;
    scanConfig.curDcachePath = PATH_FOR_META + "/latest";
    scanConfig.prevDcachePath = PATH_FOR_META + "/previous";
    scanConfig.metaPathForCtrlFiles = PATH_FOR_CTRL;
    scanConfig.indexPath = PATH_FOR_RFI;
    scanConfig.generatorIsFull = false;

    // /* 记录线程数 */
    scanConfig.maxCommonServiceInstance = 1;

    scanConfig.scanCtrlMaxDataSize = "1073741824000000";
    scanConfig.scanCtrlMinDataSize = "5368709120000";
    scanConfig.scanCtrlFileTimeSec = 10000;
    scanConfig.scanCtrlMaxEntriesFullBkup = 5000000;
    scanConfig.scanCtrlMaxEntriesIncBkup = 5000000;
    scanConfig.scanCtrlMinEntriesFullBkup = 5000000;
    scanConfig.scanCtrlMinEntriesIncBkup = 5000000;
    scanConfig.scanMetaFileSize = ONE_GB;

    scanConfig.maxWriteQueueSize = 10000;

    scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
    scanConfig.scanHardlinkResultCb = GeneratedHardLinkCtrlFileCb;
    scanConfig.rfiCtrlCb = GeneratedRfiCtrlFileCb;

    scanConfig.dFilter = ScanDirectoryFilter{FILTER_TYPE::DISABLED, {}};
    scanConfig.fFilter = ScanFileFilter{FILTER_TYPE::DISABLED, {}};

    HCP_Log(INFO, MODULE) << "EXIT FillScanConfig" << HCPENDLOG;
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

static void StartAndWaitScan()
{
    INFOLOG("-----------------ENTER StartAndWaitScan----------------");
    ScanConfig scanConfig {};
    FillScanConfig(scanConfig);
    std::shared_ptr<Scanner> scanner = ScanMgr::CreateScanInst(scanConfig);
    std::vector<std::string> enqueueList {
        "/home"
    };
    for (auto& path : enqueueList) {
        scanner->Enqueue(PATH_FOR_FS + path, PATH_FOR_FS);
    }
    ASSERT_EQ(scanner->Start(), SCANNER_STATUS::SUCCESS);
    EXPECT_TRUE(MonitorScanner(scanner));
    scanner->Destroy();
    INFOLOG("-----------------EXIT StartAndWaitScan----------------");
}

static void CreateFiles()
{
    FS_SCANNER::RemoveDir(PATH_FOR_META + "/previous");
    FS_SCANNER::Rename(PATH_FOR_META + "/latest", PATH_FOR_META + "/previous");
    SetupFileTree({
        {
            "/home", {}
        }, {
            "/home/user0", {"1.txt"}
        }, {
            "/home/user1", {"1.txt", "2.txt", "3.txt"}
        }, {
            "/home/user2", {"1.png", "2.png", "3.png"}
        }, {
            "/home/user3", {"1.jpg", "2.jpg", "3.jpg"}
        }, {
            "/home/user4", {}
        }, {
            "/opt", {"1.txt", "2.txt", "3.txt"}
        }
    }, {});
    // 创建硬链接
    string cmd = "ln " + PATH_FOR_FS + "/opt/1.txt " + PATH_FOR_FS + "/home/user4/1.txt";
    system(cmd.c_str());
    cmd = "ln " + PATH_FOR_FS + "/opt/2.txt " + PATH_FOR_FS + "/home/user4/2.txt";
    system(cmd.c_str());
    cmd = "ln " + PATH_FOR_FS + "/opt/3.txt " + PATH_FOR_FS + "/home/user4/3.txt";
    system(cmd.c_str());
    cmd = "ln " + PATH_FOR_FS + "/home/user3/3.jpg " + PATH_FOR_FS + "/home/user4/3.jpg";
    system(cmd.c_str());
}

/*
 *  /增加的文件前缀为r
 *  /hmoe/user0: 删除/user0/*
 *  /home/user1: 删除/user1
 *  /home/user2: 删：2.png; 增：r4.png
 *  /home/user3: 不变
 *  /home/user4: 硬链接，1.txt不变；删：链2.txt，源3.txt; 增：r4.txt
 *  其他场景待增加
 **/
static void CreateFilesForInc()
{
    FS_SCANNER::RemoveDir(PATH_FOR_META + "/previous");
    // 重命名作为上一次扫描结果
    FS_SCANNER::Rename(PATH_FOR_META + "/latest", PATH_FOR_META + "/previous");
    SetupFileTree({
        {
            "/home/user2", {"r4.png"}
        }, {
            "/opt", {"r4.txt"}
        }
    }, {
        {
            "/home/user0", {"1.txt"}
        }, {
            "/home/user1", {""}
        }, {
            "/home/user2", {"2.png"}
        }, {
            "/home/user4", {"2.txt"}
        }, {
            "/opt", {"3.txt"}
        }
    }, false);
    // 创建硬链接
    string cmd = "ln " + PATH_FOR_FS + "/opt/r4.txt " + PATH_FOR_FS + "/home/user4/r4.txt";
    system(cmd.c_str());
    cmd = "ln " + PATH_FOR_FS + "/home/user3/2.jpg " + PATH_FOR_FS + "/home/user4/r2.jpg";
    system(cmd.c_str());
    cmd = "ln " + PATH_FOR_FS + "/home/user3/3.jpg " + PATH_FOR_FS + "/home/user4/r3.jpg";
    system(cmd.c_str());
}

class DiffControlServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
private:
    std::shared_ptr<DiffControlService> InitDiffControlService(ScanConfig& config);
};

static int runShellCmdWithOutputStub(void* obj, const severity_level& severity, const string& moduleName,
        const size_t& requestID, const string& cmd, const vector<string> params,
        vector<string>& cmdoutput, vector<string>& stderroutput)
{
    system(cmd.c_str());
    return 0;
}

void DiffControlServiceTest::SetUp()
{
    stub.set(runShellCmdWithOutput, runShellCmdWithOutputStub);
}

void DiffControlServiceTest::TearDown()
{
    stub.reset(runShellCmdWithOutput);
}

// 最开始扫描不存在的目录
void DiffControlServiceTest::SetUpTestCase()
{
    SetupDirForTest();
    SetupFileTree({
        {
            "/home", {}
        }
    }, {});
    StartAndWaitScan();
}

void DiffControlServiceTest::TearDownTestCase()
{
    CleanDirAndFile();
    INFOLOG("EXIT DiffControlServiceTest");
}

std::shared_ptr<DiffControlService> DiffControlServiceTest::InitDiffControlService(ScanConfig& config)
{
    int threadId = 0;
    shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
    HardlinkManager hardLinkManager {};
    std::shared_ptr<Trie> trie = make_shared<Trie>();
    shared_ptr<MetaFileManager> metaFileManager = make_shared<MetaFileManager>();
    std::vector<std::string> emptyList {};
    std::shared_ptr<CtrlFilterManager> filterManager = std::make_shared<CtrlFilterManager>(emptyList, emptyList);
    DiffControlInfo info(statsMgr, trie, metaFileManager, filterManager);
    return std::make_shared<DiffControlService>(threadId, config, hardLinkManager, info);
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

static std::shared_ptr<DirCacheParser> CreateDcacheObj_Failed(std::string fname,
    CTRL_FILE_OPEN_MODE mode, ScanConfig &config)
{
    return nullptr;
}


static std::shared_ptr<MtimeCtrlParser> CreateDirectoryMtimeObj_failed(void* obj)
{
    return nullptr;
}

static std::shared_ptr<DeleteCtrlParser> CreateDeleteCtrlObj_failed(void* obj)
{
    return nullptr;
}

/*
 *   DiffScan需要创建文件树进行扫描以生成meta文件
 *   增量用例需严格按照顺序进行执行，以减少扫描次数
 *     1、原始为空目录，新增文件
 *     2、原始有文件，对原始文件进行了增删改
 *     3、原始有文件，之后全部删除
 **/

/*
 * ~~~~~~~~~~~~~~~~~~~~~增量分割线~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 用例名称：场景1：DiffScan，扫描类型为CONTROL_GEN，增量
 * 前置条件：调用Init初始化DiffControlService对象
 * check点：检查返回值
 **/
TEST_F(DiffControlServiceTest, DiffScan_CONTROL_GEN_INC) 
{
    // INFOLOG("-------------ENTER DiffControlServiceTest.DiffScan_CONTROL_GEN_INC------------");
    // CreateFiles();
    // StartAndWaitScan();
    // ScanConfig config {};
    // FillScanConfig(config);
    // config.scanType = ScanJobType::CONTROL_GEN;
    // std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    // string finalDcacheFile = PATH_FOR_META + "/latest/dircache";
    // cout << finalDcacheFile << endl;
    
    // bool isFull = false; // true：全量扫描，false：增量扫描
    // EXPECT_EQ(m_ins->Init(), SCANNER_SUCCESS);
    // int ret = m_ins->DiffScan(finalDcacheFile, isFull);
    // EXPECT_EQ(ret, SCANNER_SUCCESS);
    // INFOLOG("------------EXIT DiffControlServiceTest.DiffScan_CONTROL_GEN_INC------------");
}

/*
 * 用例名称：场景1：DiffScan，扫描类型为RFI_GEN，增量
 * 前置条件：上一个用例创建上一次meta仓
 * check点：检查返回值
 **/
TEST_F(DiffControlServiceTest, DiffScan_RFI_GEN_INC) 
{
    // INFOLOG("---------------ENTER DiffControlServiceTest.DiffScan_RFI_GEN_INC--------------");
    // int threadId = 0;
    // ScanConfig config {};
    // FillScanConfig(config);
    // config.scanType = ScanJobType::RFI_GEN;
    // std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    // string finalDcacheFile = PATH_FOR_META + "/latest/dircache";
    // cout << finalDcacheFile << endl;
    
    // bool isFull = false; // true：全量扫描，false：增量扫描
    // EXPECT_EQ(m_ins->Init(), SCANNER_SUCCESS);
    // int ret = m_ins->DiffScan(finalDcacheFile, isFull);
    // EXPECT_EQ(ret, SCANNER_SUCCESS);
    // INFOLOG("--------------EXIT DiffControlServiceTest.DiffScan_RFI_GEN_INC--------------");
}

/*
 * 用例名称：场景2：DiffScan，扫描类型为CONTROL_GEN，增量
 * 前置条件：调用Init初始化DiffControlService对象
 * check点：检查返回值
 **/
TEST_F(DiffControlServiceTest, DiffScan_CONTROL_GEN_INC_2) 
{
    // INFOLOG("--------------ENTER DiffControlServiceTest.DiffScan_CONTROL_GEN_INC_2--------------");
    // CreateFilesForInc();
    // StartAndWaitScan();
    // int threadId = 0;
    // ScanConfig config {};
    // FillScanConfig(config);
    // config.scanType = ScanJobType::CONTROL_GEN;
    // std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    // string finalDcacheFile = PATH_FOR_META + "/latest/dircache";
    // cout << finalDcacheFile << endl;
    
    // bool isFull = false; // true：全量扫描，false：增量扫描
    // EXPECT_EQ(m_ins->Init(), SCANNER_SUCCESS);
    // int ret = m_ins->DiffScan(finalDcacheFile, isFull);
    // EXPECT_EQ(ret, SCANNER_SUCCESS);
    // INFOLOG("--------------EXIT DiffControlServiceTest.DiffScan_CONTROL_GEN_INC_2--------------");
}

/*
 * 用例名称：场景3：DiffScan，扫描类型为CONTROL_GEN，增量
 * 前置条件：调用Init初始化DiffControlService对象
 * check点：检查返回值
 **/
TEST_F(DiffControlServiceTest, DiffScan_CONTROL_GEN_INC_3) 
{
    // INFOLOG("--------------ENTER DiffControlServiceTest.DiffScan_CONTROL_GEN_INC_3--------------");
    // FS_SCANNER::RemoveDir(PATH_FOR_META + "/previous");
    // // 重命名作为上一次扫描结果
    // FS_SCANNER::Rename(PATH_FOR_META + "/latest", PATH_FOR_META + "/previous");
    // FS_SCANNER::RemoveDir(PATH_FOR_FS + "/home");
    // StartAndWaitScan();
    // int threadId = 0;
    // ScanConfig config {};
    // FillScanConfig(config);
    // config.scanType = ScanJobType::CONTROL_GEN;
    // std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    // string finalDcacheFile = PATH_FOR_META + "/latest/dircache";
    // cout << finalDcacheFile << endl;
    
    // bool isFull = false; // true：全量扫描，false：增量扫描
    // EXPECT_EQ(m_ins->Init(), SCANNER_SUCCESS);
    // int ret = m_ins->DiffScan(finalDcacheFile, isFull);
    // EXPECT_EQ(ret, SCANNER_SUCCESS);
    // INFOLOG("--------------EXIT DiffControlServiceTest.DiffScan_CONTROL_GEN_INC_3--------------");
}

/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~全量分割线~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 用例名称：DiffScan，扫描类型为CONTROL_GEN，全量
 * 前置条件：调用Init初始化DiffControlService对象
 * check点：检查返回值
 **/
TEST_F(DiffControlServiceTest, DiffScan_CONTROL_GEN) 
{
    // INFOLOG("--------------ENTER DiffControlServiceTest.DiffScan_CONTROL_GEN--------------");
    // int threadId = 0;
    // ScanConfig config {};
    // FillScanConfig(config);
    // config.scanType = ScanJobType::CONTROL_GEN;
    // std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    // string finalDcacheFile = PATH_FOR_META + "/latest/dircache";
    // cout << finalDcacheFile << endl;
    
    // bool isFull = true; // true：全量扫描，false：增量扫描
    // EXPECT_EQ(m_ins->Init(), SCANNER_SUCCESS);
    // int ret = m_ins->DiffScan(finalDcacheFile, isFull);
    // EXPECT_EQ(ret, SCANNER_SUCCESS);

    // stub.set((std::shared_ptr<DirCacheParser>(ControlFileUtils::*)(std::string, CTRL_FILE_OPEN_MODE, ScanConfig&))
    //     ADDR(ControlFileUtils, CreateDcacheObj), CreateDcacheObj_Failed);
    // EXPECT_EQ(m_ins->DiffScan(finalDcacheFile, isFull), SCANNER_FAILED);
    // INFOLOG("--------------EXIT DiffControlServiceTest.DiffScan_CONTROL_GEN--------------");
}

/*
 * 用例名称：DiffScan，扫描类型为RFI_GEN，全量
 * 前置条件：调用Init初始化DiffControlService对象
 * check点：检查返回值
 **/
TEST_F(DiffControlServiceTest, DiffScan_RFI_GEN) 
{
    // INFOLOG("--------------ENTER DiffControlServiceTest.DiffScan_RFI_GEN--------------");
    // int threadId = 0;
    // ScanConfig config {};
    // FillScanConfig(config);
    // config.scanType = ScanJobType::RFI_GEN;
    // std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    // string finalDcacheFile = PATH_FOR_META + "/latest/dircache";
    // cout << finalDcacheFile << endl;
    
    // bool isFull = true; // true：全量扫描，false：增量扫描
    // EXPECT_EQ(m_ins->Init(), SCANNER_SUCCESS);
    // int ret = m_ins->DiffScan(finalDcacheFile, isFull);
    // EXPECT_EQ(ret, SCANNER_SUCCESS);
    // INFOLOG("--------------EXIT DiffControlServiceTest.DiffScan_RFI_GEN--------------");
}

static CTRL_FILE_RETCODE Stub_Open_SUCCESS()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

static CTRL_FILE_RETCODE Stub_Open_FAILED()
{
    return CTRL_FILE_RETCODE::FAILED;
}

/*
 * 用例名称：Init
 * 前置条件：无
 * check点：函数返回值
 */
TEST_F(DiffControlServiceTest, Init)
{
    int threadId = 0;
    ScanConfig config {};
    FillScanConfig(config);
    config.scanType = ScanJobType::RFI_GEN;
    std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);
    EXPECT_EQ(m_ins->Init(), SCANNER_SUCCESS);
    stub.set(ADDR(RfiCtrlParser, Open), Stub_Open_FAILED);
    EXPECT_EQ(m_ins->Init(), SCANNER_FAILED);
    m_ins->m_config.scanType = ScanJobType::CONTROL_GEN;
    m_ins->m_config.lastBackupTime = 123456;
    EXPECT_EQ(m_ins->Init(), SCANNER_FAILED);
    stub.reset(ADDR(RfiCtrlParser, Open));
    EXPECT_EQ(m_ins->Init(), SCANNER_SUCCESS);
    stub.set(ADDR(ControlFileUtils, CreateDirectoryMtimeObj), CreateDirectoryMtimeObj_failed);
    EXPECT_EQ(m_ins->Init(), SCANNER_FAILED);
    stub.reset(ADDR(ControlFileUtils, CreateDirectoryMtimeObj));
    stub.set(ADDR(ControlFileUtils, CreateDeleteCtrlObj), CreateDeleteCtrlObj_failed);
    EXPECT_EQ(m_ins->Init(), SCANNER_FAILED);
    stub.reset(ADDR(ControlFileUtils, CreateDeleteCtrlObj));
}

static int Stub_InitRfiFile_FAILED()
{
    return -1;
}

static int Stub_InitRfiFile_SUCCESS()
{
    return 0;
}

static int Stub_FAILED()
{
    return -1;
}

static int Stub_SUCCESS()
{
    return 0;
}

static CTRL_FILE_RETCODE Stub_WriteEntry_FAILED()
{
    return CTRL_FILE_RETCODE::FAILED;
}
static CTRL_FILE_RETCODE Stub_WriteEntry_LIMIT_REACHED()
{
    return CTRL_FILE_RETCODE::LIMIT_REACHED;
}
static CTRL_FILE_RETCODE Stub_WriteEntry_SUCCESS()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}
static int Stub_InitMtimeFile_FAILED()
{
    return -1;
}
static int Stub_InitMtimeFile_SUCCESS()
{
    return 0;
}

/*
 * 用例名称：WriteDirectoryMtimeCtrlFile
 * 前置条件：无
 * check点：写目录的Mtime控制文件
 **/
// TEST_F(DiffControlServiceTest, WriteDirectoryMtimeCtrlFile) 
// {
//     int threadId = 0;
//     ScanConfig config;
//     config.scanType = ScanJobType::CONTROL_GEN;
//     config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
//     config.prevDcachePath = ""; // 上次dircache位置
//     config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
//     config.scanCtrlMaxDataSize = "1";
//     shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
//     std::unique_ptr<DiffControlService> m_ins = std::make_unique<DiffControlService>(threadId, config, statsMgr);

//     stub.set(ADDR(MtimeCtrlParser, WriteEntry), Stub_WriteEntry_FAILED);
//     XMetaField xMeta;
//     xMeta.m_xMetaType == XMETA_TYPE::XMETA_TYPE_NAME;
//     xMeta.m_value = "nanjing";
//     DirMetaWrapper dmWrapper;
//     dmWrapper.m_xMeta.push_back(xMeta);
//     int ret = m_ins->WriteDirectoryMtimeCtrlFile(dmWrapper);
//     EXPECT_EQ(ret, -1);
//     stub.reset(ADDR(MtimeCtrlParser, WriteEntry));

//     stub.set(ADDR(MtimeCtrlParser, WriteEntry), Stub_WriteEntry_LIMIT_REACHED);
//     stub.set(ADDR(DiffControlService, CloseMtimeFile), Stub_Func_Void);
//     stub.set(ADDR(DiffControlService, InitMtimeFile), Stub_InitMtimeFile_FAILED);
//     ret = m_ins->WriteDirectoryMtimeCtrlFile(dmWrapper);
//     EXPECT_EQ(ret, -1);

//     stub.set(ADDR(DiffControlService, InitMtimeFile), Stub_InitMtimeFile_SUCCESS);
//     ret = m_ins->WriteDirectoryMtimeCtrlFile(dmWrapper);
//     EXPECT_EQ(ret, 0);
//     stub.reset(ADDR(MtimeCtrlParser, WriteEntry));
//     stub.reset(ADDR(DiffControlService, CloseMtimeFile));
//     stub.reset(ADDR(DiffControlService, InitMtimeFile));


//     stub.set(ADDR(MtimeCtrlParser, WriteEntry), Stub_WriteEntry_SUCCESS);
//     ret = m_ins->WriteDirectoryMtimeCtrlFile(dmWrapper);
//     EXPECT_EQ(ret, 0);
//     stub.reset(ADDR(MtimeCtrlParser, WriteEntry));
// }

static string Stub_GetCtrlFileName()
{
    return "aaa";
}
static int Stub_GetEntries_0()
{
    return 0;
}
static int Stub_GetEntries_1()
{
    return 1;
}

/*
 * 用例名称：CloseMtimeFile
 * 前置条件：无
 * check点：关闭Mtime控制文件
 **/
// TEST_F(DiffControlServiceTest, CloseMtimeFile) 
// {
//     int threadId = 0;
//     ScanConfig config;
//     config.scanType = ScanJobType::CONTROL_GEN;
//     config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
//     config.prevDcachePath = ""; // 上次dircache位置
//     config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
//     config.scanCtrlMaxDataSize = "1";
//     shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
//     std::unique_ptr<DiffControlService> m_ins = std::make_unique<DiffControlService>(threadId, config, statsMgr);

//     stub.set(ADDR(Module::MtimeCtrlParser, Close), Stub_Func_Void);
//     stub.set(ADDR(Module::MtimeCtrlParser, GetCtrlFileName), Stub_GetCtrlFileName);
//     stub.set(ADDR(Module::MtimeCtrlParser, GetEntries), Stub_GetEntries_0);
//     stub.set(FS_SCANNER::RemoveFile, Stub_Func_False);
//     int ret = m_ins->CloseMtimeFile();
//     EXPECT_EQ(ret, 0);

//     stub.set(FS_SCANNER::RemoveFile, Stub_Func_True);
//     ret = m_ins->CloseMtimeFile();
//     EXPECT_EQ(ret, 0);

//     stub.set(ADDR(Module::MtimeCtrlParser, GetEntries), Stub_GetEntries_0);
//     ret = m_ins->CloseMtimeFile();
//     EXPECT_EQ(ret, 0);

//     stub.reset(ADDR(Module::MtimeCtrlParser, Close));
//     stub.reset(ADDR(Module::MtimeCtrlParser, GetCtrlFileName));
//     stub.reset(ADDR(Module::MtimeCtrlParser, GetEntries));
//     stub.reset(FS_SCANNER::RemoveFile);  
// }

/*
 * 用例名称：CloseDeleteCtrlFile
 * 前置条件：无
 * check点：关闭删除控制文件
 **/
// TEST_F(DiffControlServiceTest, CloseDeleteCtrlFile) 
// {
//     int threadId = 0;
//     ScanConfig config;
//     config.scanType = ScanJobType::CONTROL_GEN;
//     config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
//     config.prevDcachePath = ""; // 上次dircache位置
//     config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
//     config.scanCtrlMaxDataSize = "1";
//     shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
//     std::unique_ptr<DiffControlService> m_ins = std::make_unique<DiffControlService>(threadId, config, statsMgr);

//     stub.set(ADDR(Module::DeleteCtrlParser, Close), Stub_Func_Void);
//     stub.set(ADDR(Module::DeleteCtrlParser, GetCtrlFileName), Stub_GetCtrlFileName);
//     stub.set(ADDR(Module::DeleteCtrlParser, GetEntries), Stub_GetEntries_0);
//     stub.set(FS_SCANNER::RemoveFile, Stub_Func_False);
//     int ret = m_ins->CloseDeleteCtrlFile();
//     EXPECT_EQ(ret, 0);

//     stub.set(FS_SCANNER::RemoveFile, Stub_Func_True);
//     ret = m_ins->CloseDeleteCtrlFile();
//     EXPECT_EQ(ret, 0);

//     stub.set(ADDR(Module::DeleteCtrlParser, GetEntries), Stub_GetEntries_0);
//     ret = m_ins->CloseDeleteCtrlFile();
//     EXPECT_EQ(ret, 0);

//     stub.reset(ADDR(Module::DeleteCtrlParser, Close));
//     stub.reset(ADDR(Module::DeleteCtrlParser, GetCtrlFileName));
//     stub.reset(ADDR(Module::DeleteCtrlParser, GetEntries));
//     stub.reset(FS_SCANNER::RemoveFile);  
// }

static CTRL_FILE_RETCODE Stub_WriteDirEntry_FAILED()
{
    return CTRL_FILE_RETCODE::FAILED;
}
static CTRL_FILE_RETCODE Stub_WriteDirEntry_LIMIT_REACHED()
{
    return CTRL_FILE_RETCODE::LIMIT_REACHED;
}
static CTRL_FILE_RETCODE Stub_WriteDirEntry_SUCCESS()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}
static int Stub_InitDeletCtrlFile_FAILED()
{
    return -1;
}
static int Stub_InitDeletCtrlFile_SUCCESS()
{
    return 0;
}

/*
 * 用例名称：WriteDeleteDirectoryEntry
 * 前置条件：无
 * check点：写删除目录entry
 **/
// TEST_F(DiffControlServiceTest, WriteDeleteDirectoryEntry) 
// {
//     int threadId = 0;
//     ScanConfig config;
//     config.scanType = ScanJobType::CONTROL_GEN;
//     config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
//     config.prevDcachePath = ""; // 上次dircache位置
//     config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
//     config.scanCtrlMaxDataSize = "1";
//     shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
//     std::unique_ptr<DiffControlService> m_ins = std::make_unique<DiffControlService>(threadId, config, statsMgr);

//     stub.set(ADDR(Module::DeleteCtrlParser, WriteDirEntry), Stub_WriteDirEntry_FAILED);
//     string path = "/file.txt";
//     int delFlag = 1;
//     int ret = m_ins->WriteDeleteDirectoryEntry(path, delFlag);
//     EXPECT_EQ(ret, -1);
//     stub.reset(ADDR(Module::DeleteCtrlParser, WriteDirEntry));

//     stub.set(ADDR(Module::DeleteCtrlParser, WriteDirEntry), Stub_WriteDirEntry_LIMIT_REACHED);
//     stub.set(ADDR(DiffControlService, CloseDeleteCtrlFile), Stub_Func_Void);
//     stub.set(ADDR(DiffControlService, InitDeletCtrlFile), Stub_InitDeletCtrlFile_FAILED);
//     ret = m_ins->WriteDeleteDirectoryEntry(path, delFlag);
//     EXPECT_EQ(ret, -1);

//     stub.set(ADDR(DiffControlService, InitDeletCtrlFile), Stub_InitDeletCtrlFile_SUCCESS);
//     ret = m_ins->WriteDeleteDirectoryEntry(path, delFlag);
//     EXPECT_EQ(ret, 0);
//     stub.reset(ADDR(Module::DeleteCtrlParser, WriteDirEntry));
//     stub.reset(ADDR(DiffControlService, CloseDeleteCtrlFile));
//     stub.reset(ADDR(DiffControlService, InitDeletCtrlFile));


//     stub.set(ADDR(Module::DeleteCtrlParser, WriteDirEntry), Stub_WriteDirEntry_SUCCESS);
//     ret = m_ins->WriteDeleteDirectoryEntry(path, delFlag);
//     EXPECT_EQ(ret, 0);
//     stub.reset(ADDR(Module::DeleteCtrlParser, WriteDirEntry));
// }

static int Stub_GetEntries_FAILED()
{
    return -1;
}
static int Stub_GetEntries_SUCCESS()
{
    return 0;
}

static int Stub_WriteDeleteDirectoryEntry_FAILED()
{
    return -1;
}
static int Stub_WriteDeleteDirectoryEntry_SUCCESS()
{
    return 0;
}

// static int Stub_InitDeletCtrlFile_FAILED()
// {
//     return -1;
// }
// static int Stub_InitDeletCtrlFile_SUCCESS()
// {
//     return 0;
// }

static CTRL_FILE_RETCODE Stub_WriteFileEntry_FAILED()
{
    return CTRL_FILE_RETCODE::FAILED;
}
static CTRL_FILE_RETCODE Stub_WriteFileEntry_LIMIT_REACHED()
{
    return CTRL_FILE_RETCODE::LIMIT_REACHED;
}
static CTRL_FILE_RETCODE Stub_WriteFileEntry_SUCCESS()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}
/*
 * 用例名称：WriteDeleteFileEntry
 * 前置条件：无
 * check点：写删除文件entry
 **/
//  TEST_F(DiffControlServiceTest, WriteDeleteFileEntry)
// {
//     int threadId = 0;
//     ScanConfig config;
//     config.scanType = ScanJobType::CONTROL_GEN;
//     config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
//     config.prevDcachePath = ""; // 上次dircache位置
//     config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
//     config.scanCtrlMaxDataSize = "1";
//     shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
//     std::unique_ptr<DiffControlService> m_ins = std::make_unique<DiffControlService>(threadId, config, statsMgr);

//     stub.set(ADDR(Module::DeleteCtrlParser, GetEntries), Stub_GetEntries_SUCCESS);
//     stub.set(ADDR(DiffControlService, WriteDeleteDirectoryEntry), Stub_WriteDeleteDirectoryEntry_FAILED);
//     string path = "/a/b";
//     string fileName = "a.txt";
//     int ret = m_ins->WriteDeleteFileEntry(fileName, path);
//     EXPECT_EQ(ret, -1);
//     stub.reset(ADDR(Module::DeleteCtrlParser, GetEntries));
//     stub.reset(ADDR(DiffControlService, WriteDeleteDirectoryEntry));

//     stub.set(ADDR(Module::DeleteCtrlParser, GetEntries), Stub_GetEntries_SUCCESS);
//     stub.set(ADDR(DiffControlService, WriteDeleteDirectoryEntry), Stub_WriteDeleteDirectoryEntry_SUCCESS);
//     stub.set(ADDR(Module::DeleteCtrlParser, WriteFileEntry), Stub_WriteFileEntry_FAILED);
//     ret = m_ins->WriteDeleteFileEntry(fileName, path);
//     EXPECT_EQ(ret, -1);
//     stub.reset(ADDR(DiffControlService, WriteDeleteDirectoryEntry));
//     stub.reset(ADDR(Module::DeleteCtrlParser, WriteFileEntry));

//     stub.set(ADDR(DiffControlService, WriteDeleteDirectoryEntry), Stub_WriteDeleteDirectoryEntry_SUCCESS);
//     stub.set(ADDR(Module::DeleteCtrlParser, WriteFileEntry), Stub_WriteFileEntry_LIMIT_REACHED);
//     stub.set(ADDR(DiffControlService, CloseDeleteCtrlFile), Stub_Func_Void);
//     stub.set(ADDR(DiffControlService, InitDeletCtrlFile), Stub_InitDeletCtrlFile_FAILED);
//     ret = m_ins->WriteDeleteFileEntry(fileName, path);
//     EXPECT_EQ(ret, -1);
//     stub.reset(ADDR(DiffControlService, WriteDeleteDirectoryEntry));
//     stub.reset(ADDR(Module::DeleteCtrlParser, WriteFileEntry));
//     stub.reset(ADDR(DiffControlService, CloseDeleteCtrlFile));
//     stub.reset(ADDR(DiffControlService, InitDeletCtrlFile));

//     stub.set(ADDR(DiffControlService, WriteDeleteDirectoryEntry), Stub_WriteDeleteDirectoryEntry_SUCCESS);
//     stub.set(ADDR(Module::DeleteCtrlParser, WriteFileEntry), Stub_WriteFileEntry_SUCCESS);
//     stub.set(ADDR(DiffControlService, CloseDeleteCtrlFile), Stub_Func_Void);
//     ret = m_ins->WriteDeleteFileEntry(fileName, path);
//     EXPECT_EQ(ret, 0);
//     stub.reset(ADDR(DiffControlService, WriteDeleteDirectoryEntry));
//     stub.reset(ADDR(Module::DeleteCtrlParser, WriteFileEntry));
//     stub.reset(ADDR(DiffControlService, CloseDeleteCtrlFile));
//     stub.reset(ADDR(Module::DeleteCtrlParser, GetEntries));
// }

static string Stub_GetFileOrDirNameFromXMeta()
{
    return "aaa";
}

static int Stub_WriteDeleteFileEntry_SUCCESS()
{
    return 0;
}

static int Stub_WriteDeleteFileEntry_FAILED()
{
    return -1;
}
/*
 * 用例名称：HandleHardlinkModifiedFile
 * 前置条件：无
 * check点：硬链接修改文件
 **/
//  TEST_F(DiffControlServiceTest, HandleHardlinkModifiedFile)
// {
//     int threadId = 0;
//     ScanConfig config;
//     config.scanType = ScanJobType::CONTROL_GEN;
//     config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
//     config.prevDcachePath = ""; // 上次dircache位置
//     config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
//     config.scanCtrlMaxDataSize = "1";
//     shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
//     std::unique_ptr<DiffControlService> m_ins = std::make_unique<DiffControlService>(threadId, config, statsMgr);

//     stub.set(FS_SCANNER::GetFileOrDirNameFromXMeta, Stub_GetFileOrDirNameFromXMeta);
//     stub.set(ADDR(DiffControlService, WriteDeleteFileEntry), Stub_WriteDeleteFileEntry_FAILED);
//     FileMetaWrapper fmWrapper;
//     FileCache fc;
//     string dirPath;
//     uint32_t aclFlag;
//     string metafileName;
//     int ret = m_ins->HandleHardlinkModifiedFile(fmWrapper, fc, dirPath, aclFlag, metafileName);
//     EXPECT_EQ(ret, -1);
//     stub.reset(FS_SCANNER::GetFileOrDirNameFromXMeta);
//     stub.reset(ADDR(DiffControlService, WriteDeleteFileEntry));

//     stub.set(FS_SCANNER::GetFileOrDirNameFromXMeta, Stub_GetFileOrDirNameFromXMeta);
//     stub.set(ADDR(DiffControlService, WriteDeleteFileEntry), Stub_WriteDeleteFileEntry_SUCCESS);
//     ret = m_ins->HandleHardlinkModifiedFile(fmWrapper, fc, dirPath, aclFlag, metafileName);
//     EXPECT_EQ(ret, 0);
//     stub.reset(FS_SCANNER::GetFileOrDirNameFromXMeta);
//     stub.reset(ADDR(DiffControlService, WriteDeleteFileEntry));
// }

static CTRL_FILE_RETCODE Stub_ReadFileMeta_FAILED(FileMeta &fMeta, uint64_t offset)
{
    return CTRL_FILE_RETCODE::FAILED;
}
static CTRL_FILE_RETCODE Stub_ReadFileMeta_SUCCESS(FileMeta &fMeta, uint64_t offset)
{
    return CTRL_FILE_RETCODE::SUCCESS;
}
/*
 * 用例名称：ReadFileMetaV20
 * 前置条件：无
 * check点：读文件metaV20
 **/
 TEST_F(DiffControlServiceTest, ReadFileMetaV20) 
{
    int threadId = 0;
    ScanConfig config;
    config.scanType = ScanJobType::CONTROL_GEN;
    config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
    config.prevDcachePath = ""; // 上次dircache位置
    config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
    config.scanCtrlMaxDataSize = "1";
    std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    // stub.set((CTRL_FILE_RETCODE(Module::MetaParser::*)(FileMeta &fMeta, uint64_t offset))ADDR(Module::MetaParser, ReadFileMeta), Stub_ReadFileMeta_FAILED);
    // FileMetaWrapper fmWrapper;
    // FileCache fc1;
    // fc1.m_fileId = 0;
    // fc1.m_mdataOffset = 0;
    // fc1.m_metaLength = 0;
    // std::tuple<uint16_t&, uint64_t&, uint16_t&> metaInfo = make_tuple(ref(fc1.m_fileId), ref(fc1.m_mdataOffset), ref(fc1.m_metaLength));;
    // bool isCurrent = true;
    // bool ret = m_ins->ReadFileMetaV20(fmWrapper, metaInfo, isCurrent); // m_incScnr.m_metaPrevFiles参数解决不了
    // EXPECT_EQ(ret, false);
    // stub.reset((CTRL_FILE_RETCODE(Module::MetaParser::*)(FileMeta &fMeta, uint64_t offset))ADDR(Module::MetaParser, ReadFileMeta));
}

static int Stub_ValidateFcacheEntries_1()
{
    return 1;
}

static int Stub_ValidateFcacheEntries_0()
{
    return 0;
}

/*
 * 用例名称：WriteFcacheEntries
 * 前置条件：无
 * check点：写fcache entry
 **/
 TEST_F(DiffControlServiceTest, WriteFcacheEntries) 
{
    int threadId = 0;
    ScanConfig config;
    config.scanType = ScanJobType::RFI_GEN;
    config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
    config.prevDcachePath = ""; // 上次dircache位置
    config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
    config.scanCtrlMaxDataSize = "1";
    std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    stub.set(ADDR(DiffControlService, WriteDirectoryToRfiFile), Stub_Func_Void);
    DirCache dcache1;
    dcache1.m_totalFiles = 0;
    DirMetaWrapper dmWrapper;
    CopyCtrlDirEntry dirEntry;
    string opt = "n";
    m_ins->WriteFcacheEntries(dcache1, dmWrapper, dirEntry, opt);
    EXPECT_EQ(m_ins->m_config.scanType, ScanJobType::RFI_GEN);
    stub.reset(ADDR(DiffControlService, WriteDirectoryToRfiFile));
}

 TEST_F(DiffControlServiceTest, WriteFcacheEntries_2) 
{
    int threadId = 0;
    ScanConfig config;
    config.scanType = ScanJobType::INC;
    config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
    config.prevDcachePath = ""; // 上次dircache位置
    config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
    config.scanCtrlMaxDataSize = "1";
    std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    stub.set(ADDR(DiffControlService, ValidateFcacheEntries), Stub_ValidateFcacheEntries_1);
    DirCache dcache1;
    dcache1.m_totalFiles = 1;
    DirMetaWrapper dmWrapper;
    CopyCtrlDirEntry dirEntry;
    string opt = "n";
    m_ins->WriteFcacheEntries(dcache1, dmWrapper, dirEntry, opt);
    EXPECT_EQ(m_ins->m_config.scanType, ScanJobType::INC);
    stub.reset(ADDR(DiffControlService, ValidateFcacheEntries));

    stub.set(ADDR(DiffControlService, ValidateFcacheEntries), Stub_ValidateFcacheEntries_0);
    dcache1.m_totalFiles = 1;
    m_ins->WriteFcacheEntries(dcache1, dmWrapper, dirEntry, opt);
    EXPECT_EQ(m_ins->m_config.scanType, ScanJobType::INC);
    stub.reset(ADDR(DiffControlService, ValidateFcacheEntries));
}

 TEST_F(DiffControlServiceTest, WriteFcacheEntries_3) 
{
    int threadId = 0;
    ScanConfig config;
    config.scanType = ScanJobType::RFI_GEN;
    config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
    config.prevDcachePath = ""; // 上次dircache位置
    config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
    config.scanCtrlMaxDataSize = "1";
    std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    stub.set(ADDR(DiffControlService, ValidateFcacheEntries), Stub_ValidateFcacheEntries_1);
    DirCache dcache1;
    dcache1.m_totalFiles = 1;
    DirMetaWrapper dmWrapper;
    CopyCtrlDirEntry dirEntry;
    string opt = "d";
    m_ins->WriteFcacheEntries(dcache1, dmWrapper, dirEntry, opt);
    EXPECT_EQ(m_ins->m_config.scanType, ScanJobType::RFI_GEN);
    stub.reset(ADDR(DiffControlService, ValidateFcacheEntries));

    stub.set(ADDR(DiffControlService, WriteDirectoryToRfiFile), Stub_Func_Void);
    dcache1.m_totalFiles = 0;
    m_ins->WriteFcacheEntries(dcache1, dmWrapper, dirEntry, opt);
    EXPECT_EQ(m_ins->m_config.scanType, ScanJobType::RFI_GEN);
    stub.reset(ADDR(DiffControlService, WriteDirectoryToRfiFile));
}

 TEST_F(DiffControlServiceTest, WriteFcacheEntries_4) 
{
    int threadId = 0;
    ScanConfig config;
    config.scanType = ScanJobType::INC;
    config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
    config.prevDcachePath = ""; // 上次dircache位置
    config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
    config.scanCtrlMaxDataSize = "1";
    std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    stub.set(ADDR(DiffControlService, WriteDirectoryToControFile), Stub_ValidateFcacheEntries_1);
    DirCache dcache1;
    dcache1.m_totalFiles = 1;
    DirMetaWrapper dmWrapper;
    CopyCtrlDirEntry dirEntry;
    string opt = "d";
    m_ins->WriteFcacheEntries(dcache1, dmWrapper, dirEntry, opt);
    EXPECT_EQ(m_ins->m_config.scanType, ScanJobType::INC);
    stub.reset(ADDR(DiffControlService, WriteDirectoryToControFile));
}

/*
 * 用例名称：WriteFileEntryToControlBuffer
 * 前置条件：无
 * check点：写fcache entry
 **/
//  TEST_F(DiffControlServiceTest, WriteFileEntryToControlBuffer) 
// {
//     int threadId = 0;
//     ScanConfig config;
//     config.scanType = ScanJobType::RFI_GEN;
//     config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
//     config.prevDcachePath = ""; // 上次dircache位置
//     config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
//     config.scanCtrlMaxDataSize = "1";
//     shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
//     std::unique_ptr<DiffControlService> m_ins = std::make_unique<DiffControlService>(threadId, config, statsMgr);

//     stub.set(FS_SCANNER::GetFileOrDirNameFromXMeta, Stub_GetFileOrDirNameFromXMeta);
//     stub.set(ADDR(Module::CopyCtrlParser, WriteFileEntry), Stub_WriteFileEntry_SUCCESS);
//     stub.set(ADDR(StatisticsMgr, IncrCommStatsByType), Stub_Func_Void);
//     FileMetaWrapper fmWrapper;
//     string modType;
//     uint32_t aclFlag;
//     FileCache fc;
//     int ret = m_ins->WriteFileEntryToControlBuffer(fmWrapper, modType, aclFlag, fc);
//     EXPECT_EQ(ret, 0);
//     stub.reset(FS_SCANNER::GetFileOrDirNameFromXMeta);
//     stub.reset(ADDR(Module::CopyCtrlParser, WriteFileEntry));
//     stub.reset(ADDR(StatisticsMgr, IncrCommStatsByType));
// }

static bool Stub_CheckCtrlFileTimeElapse_False()
{
    return false;
}
static bool Stub_CheckCtrlFileTimeElapse_True()
{
    return true;
}
/*
 * 用例名称：WriteDirectoryToControFile
 * 前置条件：无
 * check点：写目录到ctrlfile
 **/
//  TEST_F(DiffControlServiceTest, WriteDirectoryToControFile) 
// {
//     int threadId = 0;
//     ScanConfig config;
//     config.scanType = ScanJobType::RFI_GEN;
//     config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
//     config.prevDcachePath = ""; // 上次dircache位置
//     config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
//     config.scanCtrlMaxDataSize = "1";
//     shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
//     std::unique_ptr<DiffControlService> m_ins = std::make_unique<DiffControlService>(threadId, config, statsMgr);

//     stub.set(ADDR(Module::CopyCtrlParser, WriteDirEntry), Stub_WriteDirEntry_LIMIT_REACHED);
//     stub.set(ADDR(Module::CopyCtrlParser, CheckCtrlFileTimeElapse), Stub_CheckCtrlFileTimeElapse_False);
//     stub.set(ADDR(DiffControlService, WriteToControlFile), Stub_Func_Void);
//     CopyCtrlDirEntry dirEntry;
//     m_ins->WriteDirectoryToControFile(dirEntry);
//     EXPECT_EQ(m_ins->m_status, SCANNER_STATUS::CTRL_DIFF_IN_PROGRESS);
//     stub.reset(ADDR(Module::CopyCtrlParser, WriteDirEntry));
//     stub.reset(ADDR(Module::CopyCtrlParser, CheckCtrlFileTimeElapse));
//     stub.reset(ADDR(DiffControlService, WriteToControlFile));

//     stub.set(ADDR(Module::CopyCtrlParser, WriteDirEntry), Stub_WriteDirEntry_FAILED);
//     stub.set(ADDR(Module::CopyCtrlParser, CheckCtrlFileTimeElapse), Stub_CheckCtrlFileTimeElapse_False);
//     stub.set(ADDR(DiffControlService, WriteToControlFile), Stub_Func_Void);
//     m_ins->WriteDirectoryToControFile(dirEntry);
//     EXPECT_EQ(m_ins->m_status, SCANNER_STATUS::FAILED);
//     stub.reset(ADDR(Module::CopyCtrlParser, WriteDirEntry));
//     stub.reset(ADDR(Module::CopyCtrlParser, CheckCtrlFileTimeElapse));
//     stub.reset(ADDR(DiffControlService, WriteToControlFile));

// }

static CTRL_FILE_RETCODE Stub_Close_FAILED(FileMeta &fMeta, uint64_t offset)
{
    return CTRL_FILE_RETCODE::FAILED;
}
static CTRL_FILE_RETCODE Stub_Close_SUCCESS(FileMeta &fMeta, uint64_t offset)
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

static string Stub_GetControlFileName()
{
    return "aaa";
}
/*
 * 用例名称：WriteToControlFile
 * 前置条件：无
 * check点：写到ctrlfile
 **/
 TEST_F(DiffControlServiceTest, WriteToControlFile) 
{
    int threadId = 0;
    ScanConfig config;
    config.scanType = ScanJobType::RFI_GEN;
    config.curDcachePath = "/tmp/DiffControlServiceTest/fs/home/user1"; // 当前dircache位置
    config.prevDcachePath = ""; // 上次dircache位置
    config.metaPathForCtrlFiles = "/tmp/DiffControlServiceTest/fs/home/user1"; // ctrlfile 位置
    config.scanCtrlMaxDataSize = "1";
    std::shared_ptr<DiffControlService> m_ins = InitDiffControlService(config);

    stub.set(ADDR(Module::CopyCtrlParser, Close), Stub_Close_FAILED);
    m_ins->WriteToControlFile();
    EXPECT_EQ(m_ins->m_status, SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE);
    stub.reset(ADDR(Module::CopyCtrlParser, WriteDirEntry));

    stub.set(ADDR(Module::CopyCtrlParser, Close), Stub_Close_SUCCESS);
    stub.set(ADDR(Module::CopyCtrlParser, GetEntries), Stub_GetEntries_0);
    stub.set(FS_SCANNER::RemoveFile, Stub_Func_False);
    stub.set(ADDR(ControlFileUtils, GetControlFileName), Stub_GetControlFileName);
    stub.set(ADDR(Module::CopyCtrlParser, Open), Stub_Open_SUCCESS);
    m_ins->WriteToControlFile();
    EXPECT_EQ(m_ins->m_status, SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE);
    stub.reset(ADDR(Module::CopyCtrlParser, WriteDirEntry));
    stub.reset(ADDR(Module::CopyCtrlParser, GetEntries));
    stub.reset(FS_SCANNER::RemoveFile);
    stub.reset(ADDR(ControlFileUtils, GetControlFileName));
    stub.reset(ADDR(Module::CopyCtrlParser, Open));
}

/*
 * 用例名称：SequentialInsertionToTrie
 * 前置条件：无
 * check点：插入前缀树
 **/
TEST_F(DiffControlServiceTest, SequentialInsertionToTrie) 
{
    std::shared_ptr<Trie> trie = make_shared<Trie>();
    trie->Insert("/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt");
    trie->Insert("/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp");
    trie->Insert("/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp/dir1");
    trie->Insert("/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp/dir1/test2");
    trie->Insert("/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp/dir1/test1");

    int count = 0;
    for (auto & path : trie->GetAllParentPaths()) {
        count++;
    }
    EXPECT_EQ(count, 1);
    EXPECT_EQ((trie->GetAllParentPaths())[0], "/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context");
}

 /*
 * 用例名称：ReverseInsertionToTrie
 * 前置条件：无
 * check点：插入前缀树
 **/
TEST_F(DiffControlServiceTest, ReverseInsertionToTrie) 
{
    std::shared_ptr<Trie> trie = make_shared<Trie>();
    trie->Insert("/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp/dir1/test2");
    trie->Insert("/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp/dir1/test1");
    trie->Insert("/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp/dir1");
    trie->Insert("/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp");
    trie->Insert("/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt");
    
    int count = 0;
    for (auto & path : trie->GetAllParentPaths()) {
        count++;
    }
    EXPECT_EQ(count, 1);
    EXPECT_EQ((trie->GetAllParentPaths())[0], "/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context");
}

/*
 * 用例名称：MultithreadedInsertionToTrie
 * 前置条件：无
 * check点：多线程插入前缀树
 **/
TEST_F(DiffControlServiceTest, MultithreadedInsertionToTrie) 
{
    const int numThreads = 5;
    std::vector<std::thread> threads;
    std::shared_ptr<Trie> trie = make_shared<Trie>();
    std::vector<std::string> paths = {
        "/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp/dir1/test2",
        "/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp/dir1/test1",
        "/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp/dir1",
        "/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt/qzp",
        "/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context/opt"
    };

    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread([trie, i, paths](){
            trie->Insert(paths[i]);
        }));
    }

    for (auto& t : threads) {
        t.join(); 
    }

    int count = 0;
    for (auto & path : trie->GetAllParentPaths()) {
        count++;
    }
    EXPECT_EQ(count, 1);
    EXPECT_EQ((trie->GetAllParentPaths())[0], "/source_policy_b5af0ac6672244ef95f6f416d8e9d310_Context");
}