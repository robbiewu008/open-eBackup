#include "CommonService.h"
#include "ScannerTime.h"
#include "ScanConsts.h"
#include "ScannerUtils.h"
#include "define/Types.h"
#include "MergeDcacheService.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "ControlFileUtils.h"
#include "ParserStructs.h"
#include "RfiCtrlParser.h"
#include "FileParser.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using namespace std;
using namespace FS_SCANNER;
using namespace Module;

namespace {
    constexpr auto MODULE = "COMMON_SERVICE";
    constexpr auto SCANNER_SUCCESS = Module::SUCCESS; // 成功标识
    constexpr auto SCANNER_FAILED = Module::FAILED; // 失败标识
}

class CommonServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::shared_ptr<CommonService> m_ins {};
    ScanConfig m_config;
};

static int Stub_GetEntries_INT(void *obj)
{
    return 0;
}

void CommonServiceTest::SetUp()
{
    int threadId = 111;
    m_config.copyId = "222";
    // config.password = "qaz";
    ScanInfo info;
    info.m_terminateFlag = false;
    info.m_finalDcacheFile = "file";
    shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
    m_ins = make_shared<CommonService>(threadId, m_config, info, statsMgr);
    stub.set(ADDR(DeleteCtrlParser, GetEntries), Stub_GetEntries_INT);
}

void CommonServiceTest::TearDown()
{
    stub.reset(ADDR(DeleteCtrlParser, GetEntries));
}

void CommonServiceTest::SetUpTestCase()
{}

void CommonServiceTest::TearDownTestCase()
{}

enum NAS_CTRL_FILE_RETCODE {
    NAS_CTRL_FILE_RET_FAILED = -1,
    NAS_CTRL_FILE_RET_SUCCESS = 0,
    NAS_CTRL_FILE_RET_MAX_LIMIT_REACHED = 1,
    NAS_CTRL_FILE_RET_READ_EOF = 2,
};

static void STUB_CommonService_VOID(void *obj)
{
    return;
}

static int Stub_InitMtimeFile_FAILED(void *obj)
{
    std::cout << "NfsMount stub invoked" << std::endl;
    return SCANNER_FAILED;
}

static int Stub_FlushMtimeFile_FAILED(void *obj)
{
    return 1;
}

static int Stub2_InitMtimeFile_FAILED(void* obj)
{
    return -1;
}

static void STUB_InsertHardLinkFileToMap_VOID(void* obj)
{
    return;
}

static void STUB_StopScan_VOID(void* obj)
{
    return;
}

static CTRL_FILE_RETCODE STUB_Open_FAILED(void* obj)
{
    return CTRL_FILE_RETCODE::FAILED;
}

static CTRL_FILE_RETCODE Stub_WriteDirEntry_FAILED(void *obj, DeleteCtrlEntry &deleteCtrlEntry)
{
    return CTRL_FILE_RETCODE::FAILED;
}

static CTRL_FILE_RETCODE Stub_WriteDirEntry_LIMIT_REACHED(int delFlag)
{
    return CTRL_FILE_RETCODE::LIMIT_REACHED;
}

static int Stub_WriteDeleteDirectoryEntry_FAILED(void *obj)
{
    return SCANNER_FAILED;
}

static string Stub_GetFileOrDirNameFromXMeta_STRING(void *obj)
{
    return "mmm";
}

static NAS_CTRL_FILE_RETCODE Stub_WriteFileEntry_FAILED(void *obj)
{
    return NAS_CTRL_FILE_RET_FAILED;
}
static void STUB_FlushDataToFile_VOID(void *obj)
{
    return;
}

static CTRL_FILE_RETCODE STUB_Close_FAILED(void *obj)
{
    return CTRL_FILE_RETCODE::FAILED;
}


static CTRL_FILE_RETCODE STUB_Close_SUCCESS(void *obj)
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

static string STUB_ControlFileUtils_String(void* obj, string m_directory, uint32_t m_fcacheFileCount)
{
    return "mmm";
}

static std::unique_ptr<Module::FileCacheParser> STUB_ControlFileUtils_NULLPTR(void *obj, std::string filename, Module::CTRL_FILE_OPEN_MODE mode,
        ScanConfig &config)
{
    return nullptr;
}

static string STUB_GetXMetaFileName_String(void *obj)
{
    return "aa";
}

/*
 * 用例名称：Init
 * 前置条件：无
 * check点：初始化参数
 **/
// TEST_F(CommonServiceTest, Init)
// {
//     stub.set(ADDR(CommonService, InitMetaFile), Stub2_InitMtimeFile_FAILED);
//     int ret = m_ins->Init();
//     EXPECT_EQ(ret, -1);
// }

 static string Stub_GetFileOrDirNameFromXMeta_String(void* obj)
 {
     return "";
 }

static CTRL_FILE_RETCODE Stub_WriteEntry_FAILED(void* obj)
{
    return CTRL_FILE_RETCODE::FAILED;
}

static CTRL_FILE_RETCODE Stub_WriteEntry_LIMIT_REACHED()
{
    return CTRL_FILE_RETCODE::LIMIT_REACHED;
}

static int Stub_CloseMtimeFile_Int(void* obj)
{
    return 1;
}

static CTRL_FILE_RETCODE Stub_WriteEntry_SUCCESS()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

/*
 * 用例名称：WriteDirectoryMtimeCtrlFile
 * 前置条件：无
 * check点：写入CtrlFile
 **/
TEST_F(CommonServiceTest, WriteDirectoryMtimeCtrlFile)
{
    DirMetaWrapper dmWrapper;
    dmWrapper.m_meta.m_inode = 1;
    XMetaField xMetaField;
    xMetaField.m_value = "aa";
    dmWrapper.m_xMeta.push_back(xMetaField);
    stub.set(ADDR(FS_SCANNER, GetFileOrDirNameFromXMeta), Stub_GetFileOrDirNameFromXMeta_String);
    stub.set(ADDR(Module::MtimeCtrlParser, WriteEntry), Stub_WriteEntry_FAILED);
    int ret = m_ins->WriteDirectoryMtimeCtrlFile(dmWrapper);
    EXPECT_EQ(ret, -1);
    stub.reset(ADDR(Module::MtimeCtrlParser, WriteEntry));

    stub.set(ADDR(Module::MtimeCtrlParser, WriteEntry), Stub_WriteEntry_LIMIT_REACHED);
    stub.set(ADDR(CommonService, CloseMtimeFile), Stub_CloseMtimeFile_Int);
    stub.set(ADDR(CommonService, InitMtimeFile), Stub_InitMtimeFile_FAILED);
    ret = m_ins->WriteDirectoryMtimeCtrlFile(dmWrapper);
    EXPECT_EQ(ret, -1);
    stub.reset(ADDR(Module::MtimeCtrlParser, WriteEntry));
    stub.reset(ADDR(CommonService, CloseMtimeFile));
    stub.reset(ADDR(CommonService, InitMtimeFile));

}

/*
 * 用例名称：FlushMtimeFile
 * 前置条件：无
 * check点：将内存数据刷入磁盘
 **/
TEST_F(CommonServiceTest, FlushMtimeFile)
{
    stub.set(ADDR(CommonService, CloseMtimeFile), Stub_FlushMtimeFile_FAILED);
    stub.set(ADDR(CommonService, InitMtimeFile), Stub_InitMtimeFile_FAILED);
    int ret = m_ins->FlushMtimeFile();
    EXPECT_EQ(ret, -1);
    stub.reset(ADDR(CommonService, InitMtimeFile));
    stub.reset(ADDR(CommonService, CloseMtimeFile));
}

static CTRL_FILE_RETCODE Stub_WriteDirEntry_SUCCESS()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}
/*
 * 用例名称：WriteDeleteDirectoryEntry
 * 前置条件：无
 * check点：写入目标目录，生成新文件
 **/
TEST_F(CommonServiceTest, WriteDeleteDirectoryEntry)
{
    string path = "/home/ll";
    int delFlag = 1;

    DeleteCtrlEntry deleteCtrlEntry {};
    deleteCtrlEntry.m_absPath = path;
    deleteCtrlEntry.m_isDel = false;
    deleteCtrlEntry.m_isDir = true;
    
    std::unique_ptr<Module::DeleteCtrlParser> m_pDeleteCtrlParser;

    stub.set(ADDR(DeleteCtrlParser, WriteDirEntry), Stub_WriteDirEntry_FAILED);
    int ret = m_ins->WriteDeleteDirectoryEntry(path, delFlag);
    EXPECT_EQ(ret,-1);
    stub.reset(ADDR(DeleteCtrlParser, WriteDirEntry));

    stub.set(ADDR(DeleteCtrlParser, WriteDirEntry), Stub_WriteDirEntry_LIMIT_REACHED);
    stub.set(ADDR(CommonService, CloseDeleteCtrlFile), Stub_CloseMtimeFile_Int);
    stub.set(ADDR(CommonService, InitDeletCtrlFile), STUB_Close_FAILED);
    ret = m_ins->WriteDeleteDirectoryEntry(path, delFlag);
    EXPECT_EQ(ret,-1);
}

/*
 * 用例名称：WriteDeleteFileEntry
 * 前置条件：无
 * check点：写入新文件
 **/
TEST_F(CommonServiceTest, WriteDeleteFileEntry)
{
    string fileName = "mm.doc";
    string path = "/c:zm";
    int data = 0;
    m_ins->isDelDirEntryAdded = false;
    string ctlFileName = "qw";
    m_ins->m_pDeleteCtrlParser = std::make_unique<Module::DeleteCtrlParser>(ctlFileName);
    int ret = m_ins->WriteDeleteFileEntry(fileName, path);                      
    EXPECT_EQ(ret,-1);             
}

/*
 * 用例名称：Exit
 * 前置条件：无
 * check点：结束运行
 **/
TEST_F(CommonServiceTest, Exit)
{
    m_ins->Exit();
    EXPECT_EQ(m_ins->m_exitThreadFlag, true);
}

/*
 * 用例名称：SetTaskStatus
 * 前置条件：无
 * check点：设置任务状态
 **/
TEST_F(CommonServiceTest, SetTaskStatus) 
{
    bool writeJob = false; 
    m_ins->SetTaskStatus(writeJob);
    EXPECT_EQ(m_ins->m_writeJobTask, false);
}

/*
 * 用例名称：AddDirScanObjList
 * 前置条件：无
 * check点：增加目标扫描文件
 **/
TEST_F(CommonServiceTest, AddDirScanObjList)  
{
    vector<DirectoryScan> dirObjList;
    DirectoryScan d1;
    d1.m_isResumeCalled = 111;
    d1.m_isDirScanCompleted = false;
    d1.m_filterFlag = 000;

    DirectoryScan d2;
    d2.m_isResumeCalled = 222;
    d2.m_isDirScanCompleted = false;
    d2.m_filterFlag = 999;
    dirObjList.push_back(d1);
    dirObjList.push_back(d2);

    m_ins->AddDirScanObjList(dirObjList);
    EXPECT_EQ(d2.m_isResumeCalled, 222);
}

/*
 * 用例名称：HandleHardlinkModifiedFile
 * 前置条件：无
 * check点：处理硬链接修改文件
 **/
TEST_F(CommonServiceTest, HandleHardlinkModifiedFile)  
{
    FileMetaWrapper fmWrapper;
    fmWrapper.m_meta.type = 0;
    fmWrapper.m_meta.m_size = 0;

    XMetaField xMetaField;
    xMetaField.m_value = "kkk";

    string m_value = "kkk";
    fmWrapper.m_xMeta.push_back(xMetaField); 

    FileCache fc; 
    fc.m_compareFlag = 0;
    fc.m_fileMetaHash.crc = 1;
    fc.m_fileId = 2;
    fc.m_inode = 1;
    fc.m_metaLength = 1;
    fc.m_mdataOffset = 1;
    string dirPath = "asd";
    uint32_t aclFlag = 1122;
    string metafileName = "qwe";

    string ctlFileName = "qwQ";
    m_ins->m_pDeleteCtrlParser = std::make_unique<Module::DeleteCtrlParser>(ctlFileName);
   
    HardlinkFileCache hfc;
    hfc.m_inode = 0;
    hfc.m_dirName = "xm";
    stub.set(ADDR(RfiCtrlParser, GetFileOrDirNameFromXMeta), Stub_GetFileOrDirNameFromXMeta_STRING);
    int ret = m_ins->HandleHardlinkModifiedFile(fmWrapper, fc, dirPath, aclFlag, metafileName);
    EXPECT_EQ(ret,-1);
} 

static string STUB_GetDeleteCtrlFileName_String(void* obj)
{
    return "";
}

static unique_ptr<DeleteCtrlParser> STUB_CreateDeleteCtrlObj_nullptr(void* obj)
{
    return nullptr;
}
/*
 * 用例名称：InitDeletCtrlFile
 * 前置条件：无
 * check点：初始化CtrlFile
 **/
 TEST_F(CommonServiceTest, InitDeletCtrlFile)
 {
     stub.set(ADDR(CommonService, GetDeleteCtrlFileName), STUB_GetDeleteCtrlFileName_String);
     stub.set(ADDR(ControlFileUtils, CreateDeleteCtrlObj), STUB_CreateDeleteCtrlObj_nullptr);
     int ret = m_ins->InitDeletCtrlFile();
     EXPECT_EQ(ret, -1);
     stub.reset(ADDR(CommonService, GetDeleteCtrlFileName));
     stub.reset(ADDR(ControlFileUtils, CreateDeleteCtrlObj));

 }
/*
 * 用例名称：IsWriteCompleted
 * 前置条件：无
 * check点：判断是否写入成功
 **/
 TEST_F(CommonServiceTest, IsWriteCompleted)
 {
     queue<DirectoryScan> m_directoryScanQueue;
     DirectoryScan directoryScan;
     directoryScan.m_isResumeCalled = 1;
     directoryScan.m_isDirScanCompleted = true;
    //  m_directoryScanQueue.push(directoryScan);
     m_ins->m_directoryScanQueue.push(directoryScan);
     m_ins->m_writeCompletedFlag = true;
     bool ret = m_ins->IsWriteCompleted();
     EXPECT_EQ(ret, false);
 }

/*
 * 用例名称：CreateNewMetaFile
 * 前置条件：无
 * check点：构造新MetaFile
 **/
TEST_F(CommonServiceTest, CreateNewMetaFile)  
{
    stub.set(ADDR(CommonService, FlushDataToFile), STUB_FlushDataToFile_VOID);

    string metaFileName = "meta";
    string filename = "file";
    m_ins->m_pMetaParser = std::make_unique<Module::MetaParser>(metaFileName);

    stub.set(ADDR(Module::MetaParser, Close), STUB_Close_FAILED);
    m_ins->m_scanInfo.StopScan(SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE); 

    bool ret = m_ins->CreateNewMetaFile();
    EXPECT_EQ(ret, false);

    stub.reset(ADDR(CommonService, FlushDataToFile));
    stub.reset(ADDR(Module::MetaParser, Close));
}

/*
 * 用例名称：CreateNewFcacheFile
 * 前置条件：无
 * check点：构造新FcacheFile
 **/
TEST_F(CommonServiceTest, CreateNewFcacheFile)  
{
    stub.set(ADDR(ControlFileUtils, GetFileCacheFileName), STUB_ControlFileUtils_String);

    string fcacheFileName = "kkk";

    m_ins->m_pFileCacheParser = std::make_unique<Module::FileCacheParser>(fcacheFileName);
    stub.set(ADDR(Module::MetaParser, Close), STUB_Close_FAILED);
    m_ins->m_scanInfo.StopScan(SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE); 

    bool ret = m_ins->CreateNewFcacheFile();
    EXPECT_EQ(fcacheFileName, "kkk");

}

/*
 * 用例名称：CreateNewXMetaFile
 * 前置条件：无
 * check点：构建新XMetaFile
 **/
TEST_F(CommonServiceTest, CreateNewXMetaFile) 
{
    stub.set(ADDR(CommonService, FlushDataToFile), STUB_FlushDataToFile_VOID);
    m_ins->m_directory = "ll";

    string metafileName = "qwe";
    m_ins->m_pMetaParser = std::make_unique<Module::MetaParser>(metafileName);

    stub.set(ADDR(Module::MetaParser, Close), STUB_Close_FAILED);
    m_ins->m_scanInfo.StopScan(SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE); 

    bool ret = m_ins->CreateNewXMetaFile();
    EXPECT_EQ(ret, false);
    stub.reset(ADDR(CommonService, FlushDataToFile));
    stub.reset(ADDR(Module::MetaParser, Close));
}

/*
 * 用例名称：GetHardlinkMaps
 * 前置条件：无
 * check点：获得HardlinkMaps
 **/
TEST_F(CommonServiceTest, GetHardlinkMaps)
{
    map<uint64_t, vector<HardlinkFileCache>> hardLinkMap;
    map<string, uint32_t> hardlinkFilesCntOfDirPathMap;
    HardlinkFileCache hardlinkFileCache;
    vector<HardlinkFileCache> hard;
    hardlinkFileCache.m_inode = 1;
    hard.push_back(hardlinkFileCache);
    hardLinkMap.insert(pair<uint64_t, vector<HardlinkFileCache>>(1,hard));
    hardlinkFilesCntOfDirPathMap.insert(map<string, uint32_t>::value_type("aa", 1));
    m_ins->m_hardlinkManager.m_hardlinkMap.insert(map<uint64_t, vector<HardlinkFileCache>>::value_type(1, hard));
    m_ins->m_hardlinkManager.m_hardlinkFilesCntOfDirPathMap.insert(map<string, uint32_t>::value_type("aa", 1));

    m_ins->GetHardlinkMaps(hardLinkMap, hardlinkFilesCntOfDirPathMap);
    EXPECT_EQ(hardlinkFileCache.m_inode, 1);
}

/*
 * 用例名称：InitHardLinkControlFile
 * 前置条件：无
 * check点：初始化InitHardLinkControlFile
 **/
TEST_F(CommonServiceTest, InitHardLinkControlFile)
{
    m_config.lastBackupTime = 1;
    int ret = m_ins->InitHardLinkControlFile();
    EXPECT_EQ(ret, 0);

    m_config.lastBackupTime = 0;
    stub.set(ADDR(Module::HardlinkCtrlParser, Open), STUB_Open_FAILED);
    stub.set(ADDR(ScanInfo, StopScan), STUB_StopScan_VOID);
    ret = m_ins->InitHardLinkControlFile();
    EXPECT_EQ(ret, -1);
}

/*
 * 用例名称：InitControlFile
 * 前置条件：无
 * check点：初始化ControlFile
 **/
TEST_F(CommonServiceTest, InitControlFile)
{
    m_config.lastBackupTime = 1;
    int ret = m_ins->InitControlFile();
    EXPECT_EQ(ret, 0);

    m_config.lastBackupTime = 0;
    stub.set(ADDR(Module::CopyCtrlParser, Open), STUB_Open_FAILED);
    stub.set(ADDR(ScanInfo, StopScan), STUB_StopScan_VOID);
    ret = m_ins->InitControlFile();
    EXPECT_EQ(ret, -1);
}

/*
 * 用例名称：RestoreHardLinkFCacheMap
 * 前置条件：无
 * check点：恢复FCache
 **/
TEST_F(CommonServiceTest, RestoreHardLinkFCacheMap)
{
    queue<HardlinkFileCache> hardLinkFCacheQue;
    HardlinkFileCache hardlinkFileCache;
    hardlinkFileCache.m_inode = 1;
    hardlinkFileCache.m_fileName = "/file";
    hardlinkFileCache.m_dirName = "/dir";
    hardLinkFCacheQue.push(hardlinkFileCache);
    
    stub.set(ADDR(HardlinkManager, InsertHardLinkFileToMap), STUB_InsertHardLinkFileToMap_VOID);
    m_ins->RestoreHardLinkFCacheMap(hardLinkFCacheQue);
    EXPECT_EQ(hardlinkFileCache.m_inode, 1);
}

/*
 * 用例名称：RestoreHardLinkFileCntOfDirPathMap
 * 前置条件：无
 * check点：向目标路径拷贝HardLinkFileCnt
 **/
TEST_F(CommonServiceTest, RestoreHardLinkFileCntOfDirPathMap)
{
    vector<pair<string, uint32_t>> hardlinkFilesCntList;
    hardlinkFilesCntList.push_back(make_pair("aa", 1));
    hardlinkFilesCntList.push_back(make_pair("bb", 2));

    EXPECT_NO_THROW(m_ins->RestoreHardLinkFileCntOfDirPathMap(hardlinkFilesCntList));
}