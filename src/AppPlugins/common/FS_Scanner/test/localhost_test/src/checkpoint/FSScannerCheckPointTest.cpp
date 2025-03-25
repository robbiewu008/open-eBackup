#include "FSScannerCheckPoint.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

using namespace std;
using namespace Module;

class FSScannerCheckPointTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<FSScannerCheckPoint> m_ins {nullptr};
};

void FSScannerCheckPointTest::SetUp() 
{
    ScanConfig config;
    config.metaPath = "xxx";
    ScanInfo info;
    m_ins = std::make_unique<FSScannerCheckPoint>(config, info);
}
void FSScannerCheckPointTest::TearDown() 
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}
void FSScannerCheckPointTest::SetUpTestCase() {}
void FSScannerCheckPointTest::TearDownTestCase() {}

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

/*
 * 用例名称：CreateTempCheckPointDirectory
 * 前置条件：无
 * check点：创建临时目录
 **/
TEST_F(FSScannerCheckPointTest, CreateTempCheckPointDirectory)
{
    MOCKER_CPP(&FS_SCANNER::CheckAndCreateDirectory)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&FS_SCANNER::CheckParentDirIsReachable)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));    
    ScanConfig config;
    ScanInfo info;
    std::unique_ptr<FSScannerCheckPoint> m_ins2 = std::make_unique<FSScannerCheckPoint>(config, info);
    EXPECT_EQ(m_ins2->CreateTempCheckPointDirectory(), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins2->CreateTempCheckPointDirectory(), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins2->CreateTempCheckPointDirectory(), CHECKPOINT_STATUS::SUCCESS);
}

/*
 * 用例名称：WriteCheckPointEntryToFile
 * 前置条件：无
 * check点：写入文件
 **/
TEST_F(FSScannerCheckPointTest, WriteCheckPointEntryToFile)
{
    std::string chkPntFileName = "/a/b/file.txt";
    m_ins->m_pCheckPointParser = std::make_unique<Module::CheckPointParser>(chkPntFileName);
    MOCKER_CPP(&Module::CheckPointParser::WriteChkPntEntry)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(1));
    MOCKER_CPP(&Module::CheckPointParser::Close)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));    
    MOCKER_CPP(&FSScannerCheckPoint::CreateCheckPointFile)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    string chkPntEntry = "a";
    EXPECT_EQ(m_ins->WriteCheckPointEntryToFile(chkPntEntry), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins->WriteCheckPointEntryToFile(chkPntEntry), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins->WriteCheckPointEntryToFile(chkPntEntry), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins->WriteCheckPointEntryToFile(chkPntEntry), CHECKPOINT_STATUS::SUCCESS);
}

/*
 * 用例名称：EndCheckPoint
 * 前置条件：无
 * check点：结束检查
 **/
TEST_F(FSScannerCheckPointTest, EndCheckPoint)
{
    EXPECT_EQ(m_ins->EndCheckPoint(), CHECKPOINT_STATUS::FAILED);

    MOCKER_CPP(&Module::CheckPointParser::Close)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));    
    MOCKER_CPP(&FS_SCANNER::RemoveDir)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&FS_SCANNER::Rename)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true)); 
    // ScanConfig config; // m_ins 形参为引用 奇怪问题
    ScanConfig config;
    ScanInfo info;
    std::unique_ptr<FSScannerCheckPoint> m_ins2 = std::make_unique<FSScannerCheckPoint>(config, info);
    std::string chkPntFileName = "/a/b/file.txt";
    m_ins2->m_pCheckPointParser = std::make_unique<Module::CheckPointParser>(chkPntFileName);    
    EXPECT_EQ(m_ins2->EndCheckPoint(), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins2->EndCheckPoint(), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins2->EndCheckPoint(), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins2->EndCheckPoint(), CHECKPOINT_STATUS::SUCCESS);
}

/*
 * 用例名称：ReadCheckPointDirectory
 * 前置条件：无
 * check点：读目录
 **/
TEST_F(FSScannerCheckPointTest, ReadCheckPointDirectory)
{
    MOCKER_CPP(&FSScannerCheckPoint::ReadLatestDirectory)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));   
    MOCKER_CPP(&FS_SCANNER::PathExist)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&FS_SCANNER::GetFileListInDirectory)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true)); 
    ScanConfig config;
    ScanInfo info;
    std::unique_ptr<FSScannerCheckPoint> m_ins2 = std::make_unique<FSScannerCheckPoint>(config, info);
    std::string chkPntFileName = "/a/b/file.txt";
    vector<string> checkPointFiles;   
//     EXPECT_EQ(m_ins2->ReadCheckPointDirectory(checkPointFiles), CHECKPOINT_STATUS::FAILED);
//     EXPECT_EQ(m_ins2->ReadCheckPointDirectory(checkPointFiles), CHECKPOINT_STATUS::FAILED);
//     EXPECT_EQ(m_ins2->ReadCheckPointDirectory(checkPointFiles), CHECKPOINT_STATUS::SUCCESS);
}

/*
 * 用例名称：CloseAndRenameHardLinkFile
 * 前置条件：无
 * check点：关闭并重命名硬链接文件
 **/
TEST_F(FSScannerCheckPointTest, CloseAndRenameHardLinkFile)
{ 
    MOCKER_CPP(&Module::HardlinkCtrlParser::Close)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    MOCKER_CPP(&Module::HardlinkCtrlParser::GetEntries)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(1))
            .then(returnValue(0));
    MOCKER_CPP(&FS_SCANNER::Rename)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true)); 
    MOCKER_CPP(&FS_SCANNER::RemoveFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true)); 
    EXPECT_EQ(m_ins->CloseAndRenameHardLinkFile(), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins->CloseAndRenameHardLinkFile(), CHECKPOINT_STATUS::SUCCESS);
    EXPECT_EQ(m_ins->CloseAndRenameHardLinkFile(), CHECKPOINT_STATUS::SUCCESS);
    EXPECT_EQ(m_ins->CloseAndRenameHardLinkFile(), CHECKPOINT_STATUS::SUCCESS);
    EXPECT_EQ(m_ins->CloseAndRenameHardLinkFile(), CHECKPOINT_STATUS::SUCCESS);
}

/*
 * 用例名称：CreateAndOpenHardLinkFile
 * 前置条件：无
 * check点：创建并打开硬链接文件
 **/
TEST_F(FSScannerCheckPointTest, CreateAndOpenHardLinkFile)
{ 
    MOCKER_CPP(&Module::HardlinkCtrlParser::Open)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    ScanConfig config;
    ScanInfo info;
    std::unique_ptr<FSScannerCheckPoint> m_ins2 = std::make_unique<FSScannerCheckPoint>(config, info);
    EXPECT_EQ(m_ins2->CreateAndOpenHardLinkFile(), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins2->CreateAndOpenHardLinkFile(), CHECKPOINT_STATUS::SUCCESS);
}

/*
 * 用例名称：RemoveCheckPointDir
 * 前置条件：无
 * check点：移除目录
 **/
TEST_F(FSScannerCheckPointTest, RemoveCheckPointDir)
{ 
    MOCKER_CPP(&FS_SCANNER::RemoveDir)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    ScanConfig config;
    ScanInfo info;
    std::unique_ptr<FSScannerCheckPoint> m_ins2 = std::make_unique<FSScannerCheckPoint>(config, info);
    EXPECT_EQ(m_ins2->RemoveCheckPointDir(), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins2->RemoveCheckPointDir(), CHECKPOINT_STATUS::SUCCESS);
}

/*
 * 用例名称：ReadCheckPointFile
 * 前置条件：无
 * check点：读文件
 **/
TEST_F(FSScannerCheckPointTest, ReadCheckPointFile)
{ 
    MOCKER_CPP(&Module::CheckPointParser::Open)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    vector<string> v;
    MOCKER_CPP(&Module::CheckPointParser::ReadAllChkPntEntries)
            .stubs()
            .will(returnValue(v));
    MOCKER_CPP(&Module::CheckPointParser::Close)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    string chkPntFileName = "file.txt";
    vector<CheckPointData> chkPntDataList;
    EXPECT_EQ(m_ins->ReadCheckPointFile(chkPntFileName, chkPntDataList), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins->ReadCheckPointFile(chkPntFileName, chkPntDataList), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins->ReadCheckPointFile(chkPntFileName, chkPntDataList), CHECKPOINT_STATUS::SUCCESS);
}

/*
 * 用例名称：ReadLatestDirectory
 * 前置条件：无
 * check点：读最新目录
 **/
// TEST_F(FSScannerCheckPointTest, ReadLatestDirectory)
// { 
//     MOCKER_CPP(&FS_SCANNER::PathExist)
//             .stubs()
//             .will(returnValue(false))
//             .then(returnValue(true));
//     ScanConfig config;
//     ScanInfo info;
//     std::unique_ptr<FSScannerCheckPoint> m_ins2 = std::make_unique<FSScannerCheckPoint>(config, info);
//     EXPECT_EQ(m_ins2->ReadLatestDirectory(), CHECKPOINT_STATUS::FAILED);
//     EXPECT_EQ(m_ins2->ReadLatestDirectory(), CHECKPOINT_STATUS::FAILED);
// }

/*
 * 用例名称：CreateCheckPointFile
 * 前置条件：无
 * check点：创建文件
 **/
TEST_F(FSScannerCheckPointTest, CreateCheckPointFile)
{ 
    string str = "1";
    MOCKER_CPP(&FS_SCANNER::GetScanTypeStr)
            .stubs()
            .will(returnValue(str));
    string str2 = "xxx";
    MOCKER_CPP(&FSScannerCheckPoint::GetCheckPointFileName)
            .stubs()
            .will(returnValue(str2)); 
    MOCKER_CPP(&Module::CheckPointParser::Open)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    ScanConfig config;
    config.lastBackupTime = 1;
    config.scanCtrlMaxDataSize = "1";
    ScanInfo info;
    std::unique_ptr<FSScannerCheckPoint> m_ins2 = std::make_unique<FSScannerCheckPoint>(config, info);
    EXPECT_EQ(m_ins2->CreateCheckPointFile(), CHECKPOINT_STATUS::FAILED);
    EXPECT_EQ(m_ins2->CreateCheckPointFile(), CHECKPOINT_STATUS::SUCCESS);
}