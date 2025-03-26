#include "SnapdiffService.h"
#include <thread>
#include "log/Log.h"
#include "ScannerUtils.h"
#include "BufferQueue.h"

#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

using namespace std;
using namespace Module;
using namespace FS_SCANNER;

class SnapdiffServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<SnapdiffService> m_ins {nullptr};
};

void SnapdiffServiceTest::SetUp() {
    ScanConfig config;
    std::shared_ptr<BufferQueue<SnapdiffResultMap>> snapdiffBufferQueue;
    std::shared_ptr<StatisticsMgr> statsMgr;
    m_ins = std::make_unique<SnapdiffService>(config, snapdiffBufferQueue, statsMgr);
}
void SnapdiffServiceTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}
void SnapdiffServiceTest::SetUpTestCase() {}
void SnapdiffServiceTest::TearDownTestCase() {}

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
 * 用例名称：InitControlFilesAndMetaFile
 * 前置条件：无
 * check点：初始化文件
 **/
TEST_F(SnapdiffServiceTest, InitControlFilesAndMetaFile)
{

    MOCKER_CPP(&SnapdiffService::InitMetaFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&SnapdiffService::InitXMetaFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&SnapdiffService::InitCopyCtrlFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&SnapdiffService::InitHardLinkCtrlFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&SnapdiffService::InitMtimeCtrlFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&SnapdiffService::InitDeleteCtrlFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    m_ins->InitControlFilesAndMetaFile();
    m_ins->InitControlFilesAndMetaFile();
    m_ins->InitControlFilesAndMetaFile();
    m_ins->InitControlFilesAndMetaFile();
    m_ins->InitControlFilesAndMetaFile();
    m_ins->InitControlFilesAndMetaFile();
    EXPECT_EQ(m_ins->InitControlFilesAndMetaFile(), true);
}

/*
 * 用例名称：Start
 * 前置条件：无
 * check点：开始
 **/
TEST_F(SnapdiffServiceTest, Start)
{
    MOCKER_CPP(&SnapdiffService::InitNasSession)
            .stubs()
            .will(returnValue(false));
    EXPECT_EQ(m_ins->Start(), SCANNER_STATUS::FAILED);
}

/*
 * 用例名称：InitNasSession
 * 前置条件：无
 * check点：初始化Nas会话
 **/
TEST_F(SnapdiffServiceTest, InitNasSession)
{
    MOCKER_CPP(&SnapdiffService::InitNFSSession)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&SnapdiffService::InitSmbSession)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    m_ins->m_config.nasSnapdiffProtocol = NAS_PROTOCOL::NFS;
    EXPECT_EQ(m_ins->InitNasSession(), false);

    m_ins->m_config.nasSnapdiffProtocol = NAS_PROTOCOL::SMB;
    EXPECT_EQ(m_ins->InitNasSession(), false);

    EXPECT_EQ(m_ins->InitNasSession(), true);
}

/*
 * 用例名称：FetchDirChangeType
 * 前置条件：无
 * check点：获取目录修改类型
 **/
TEST_F(SnapdiffServiceTest, FetchDirChangeType)
{
    std::string dirPath = "/tmp";
    EXPECT_EQ(m_ins->FetchDirChangeType(dirPath), 0);

    m_ins->m_tmpDirChangeType["/tmp"] = SNAPDIFF_BACKUPENTRY_CHANGETYPE::NEW;
    EXPECT_EQ(m_ins->FetchDirChangeType(dirPath), 1);
}

/*
 * 用例名称：CutFilenameSuffix
 * 前置条件：无
 * check点：去掉文件名后缀
 **/
TEST_F(SnapdiffServiceTest, CutFilenameSuffix)
{
    std::string filePath = "/tmp/file";
    std::string suffix = ".txt";
    m_ins->CutFilenameSuffix(filePath, suffix);

    MOCKER_CPP(FS_SCANNER::Rename)
            .stubs()
            .will(returnValue(false));
    filePath = "/tmp/file.txt";
    m_ins->CutFilenameSuffix(filePath, suffix);

    char* str = "he";
    MOCKER_CPP(&Module::NfsContextWrapper::NfsLstat64)
            .stubs()
            .will(returnValue(1));
    MOCKER_CPP(&Module::NfsContextWrapper::NfsGetError)
            .stubs()
            .will(returnValue(str));
    MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
            .stubs()
            .will(ignoreReturnValue());
    SnapdiffBackupEntry backupEntry;
    backupEntry.m_path = "/";
    EXPECT_EQ(m_ins->FillDiffLibNfs(backupEntry), false);
}

/*
 * 用例名称：FillDiffLibSmb
 * 前置条件：无
 * check点：填充diff smb
 **/
TEST_F(SnapdiffServiceTest, FillDiffLibSmb)
{
    char* str = "he";
    MOCKER_CPP(&Module::SmbContextWrapper::SmbStat64)
            .stubs()
            .will(returnValue(1));
    SnapdiffBackupEntry backupEntry;
    backupEntry.m_path = "/";
    EXPECT_EQ(m_ins->FillDiffLibSmb(backupEntry), false);
}

/*
 * 用例名称：GetSmbAcl
 * 前置条件：无
 * check点：获取smb Acl
 **/
TEST_F(SnapdiffServiceTest, GetSmbAcl)
{
    char* str = "he";
    MOCKER_CPP(&Module::SmbContextWrapper::SmbGetSd)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(0));
    std::string path = "/tmp/file.txt";
    EXPECT_EQ(m_ins->GetSmbAcl(path), "");
}

/*
 * 用例名称：PostMetaFileWrite
 * 前置条件：无
 * check点：后置metafile写
 **/
TEST_F(SnapdiffServiceTest, PostMetaFileWrite)
{
    MOCKER_CPP(&Module::MetaParser::Close)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&SnapdiffService::InitMetaFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    m_ins->m_config.scanMetaFileSize = 0;
    m_ins->PostMetaFileWrite(5);
    m_ins->PostMetaFileWrite(5);
    EXPECT_EQ(m_ins->m_metaFileCount, 0);
}

/*
 * 用例名称：PostXMetaFileWrite
 * 前置条件：无
 * check点：后置xmetafile写
 **/
TEST_F(SnapdiffServiceTest, PostXMetaFileWrite)
{
    MOCKER_CPP(&Module::XMetaParser::Close)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&SnapdiffService::InitXMetaFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    m_ins->m_config.scanMetaFileSize = 0;
    m_ins->PostXMetaFileWrite(5);
    m_ins->PostXMetaFileWrite(5);
    EXPECT_EQ(m_ins->m_XmetaFileCount, 0);
}

/*
 * 用例名称：PostHardlinkFileWrite
 * 前置条件：无
 * check点：后置xmetafile写
 **/
TEST_F(SnapdiffServiceTest, PostHardlinkFileWrite)
{
    Module::CTRL_FILE_RETCODE retCode = CTRL_FILE_RETCODE::FAILED;
    m_ins->m_hardlinkCtrlFilePath = "/tmp";
    m_ins->PostHardlinkFileWrite(retCode);

    MOCKER_CPP(&Module::HardlinkCtrlParser::Close)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&SnapdiffService::CutFilenameSuffix)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&SnapdiffService::InitHardLinkCtrlFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    retCode = CTRL_FILE_RETCODE::LIMIT_REACHED;
    m_ins->PostHardlinkFileWrite(retCode);
    m_ins->PostHardlinkFileWrite(retCode);
    EXPECT_NE(m_ins->m_hardlinkCtrlFileCount, 0);
}