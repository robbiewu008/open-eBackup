/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Author: w30029850
 * Create: 2022-08-26.
 */

#include <list>
#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "Scanner.h"
#include "log/Log.h"
#include "ScannerTime.h"

#include "DeviceManager.h"
#include "SnapdiffService.h"
#include "ScannerUtils.h"

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
    constexpr auto MODULE = "NAS_SNAPDIFF_SERVICE_TEST";
    const std::string LOG_NAME = "NAS_SNAPDIFF_SERVICE_TEST.log";
    const std::string LOG_PATH = "/home/NAS_SNAPDIFF_SERVICE_TEST/log/";
    constexpr int ONE_GB = 1024 * 1024 * 1024;

    const std::string PATH_FOR_META = "/home/NAS_SNAPDIFF_SERVICE_TEST/meta";
    const std::string PATH_FOR_CTRL = "/home/NAS_SNAPDIFF_SERVICE_TEST/scan";

    constexpr auto MAX_QUEUE_LIMIT = 100;
}

static CTRL_FILE_RETCODE Open_stub(void* obj, CTRL_FILE_OPEN_MODE mode)
{
    std::cout << "Open stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::SUCCESS;
}

static CTRL_FILE_RETCODE Open_failed_stub(void* obj, CTRL_FILE_OPEN_MODE mode)
{
    std::cout << "Open failed stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::FAILED;
}

static CTRL_FILE_RETCODE Close_stub(void* obj, CTRL_FILE_OPEN_MODE mode)
{
    std::cout << "Close stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::SUCCESS;
}

static bool Init_stub(void* obj)
{
    std::cout << "Init stub invoked" << std::endl;
    return true;
}

static bool SmbConnect_stub(void *obj)
{
    std::cout << "SmbConnect stub invoked" << std::endl;
    return true;
}

static int SmbStat64_stub(void *obj, const char *path, struct smb2_stat_64 *st)
{
    std::cout << "SmbStat64 stub invoked" << std::endl;
    return Module::SUCCESS;
}

static int SmbDisconnect_stub(void *obj)
{
    std::cout << "SmbDisconnect stub invoked" << std::endl;
    return Module::SUCCESS;
}

static std::string GetSmbAcl_stub(const std::string &path)
{
    std::cout << "GetSmbAcl stub invoked" << std::endl;
    return "demo acl for smb";
}

static string SmbGetClientGuid_stub(void* obj)
{
    std::cout << "SmbGetClientGuid stub invoked" << std::endl;
    return "";
}

static int NfsMount_stub(void *obj)
{
    std::cout << "NfsMount stub invoked" << std::endl;
    return Module::SUCCESS;
}

static void NfsDestroyContext_stub(void *obj)
{
    std::cout << "NfsDestroyContext stub invoked" << std::endl;
    return;
}

static int NfsLstat64_stub(const char *path, struct nfs_stat_64 *st)
{
    std::cout << "NfsLstat64 stub invoked" << std::endl;
    return Module::SUCCESS;
}

static bool Rename_stub(const std::string &oldName, const std::string &newName)
{
    std::cout << "Rename stub invoked" << std::endl;
    return true;
}


static uint64_t XMetaParser_WriteXMeta_stub(std::vector<XMetaField> &entry)
{
    std::cout << "XMetaParser_WriteXMeta stub invoked" << std::endl;
    return 114;
}

static uint16_t MetaParser_WriteDirectoryMeta_stub(DirMeta &dirMeta)
{
    std::cout << "XMetaParser_WriteDirectoryMeta stub invoked" << std::endl;
    return 514;
}

static uint16_t MetaParser_WriteFileMeta_stub(FileMeta &fMeta)
{
    std::cout << "XMetaParser_WriteFileMeta stub invoked" << std::endl;
    return 1919;
}

static CTRL_FILE_RETCODE CopyCtrlParser_WriteDirEntry_stub(CopyCtrlDirEntry &dirEntry)
{
    std::cout << "CopyCtrlParser_WriteDirEntry stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::SUCCESS;
}

static CTRL_FILE_RETCODE CopyCtrlParser_WriteFileEntry_stub(CopyCtrlFileEntry &fileEntry)
{
    std::cout << "CopyCtrlParser_WriteFileEntry stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::SUCCESS;
}


static CTRL_FILE_RETCODE MtimeCtrlParser_WriteEntry_stub(MtimeCtrlEntry &mtimeEntry)
{
    std::cout << "MtimeCtrlParser_WriteEntry stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::SUCCESS;
}

static CTRL_FILE_RETCODE DeleteCtrlParser_WriteDirEntry_stub(DeleteCtrlEntry &deleteEntry)
{
    std::cout << "DeleteCtrlParser_WriteDirEntry stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::SUCCESS;
}

static CTRL_FILE_RETCODE DeleteCtrlParser_WriteFileEntry_stub(std::string  &m_fileName)
{
    std::cout << "DeleteCtrlParser_WriteFileEntry stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::SUCCESS;
}

// write limit reach stub

static CTRL_FILE_RETCODE CopyCtrlParser_WriteDirEntry_LimitReach_stub(CopyCtrlDirEntry &dirEntry)
{
    std::cout << "CopyCtrlParser_WriteDirEntry_LimitReach stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::LIMIT_REACHED;
}

static CTRL_FILE_RETCODE CopyCtrlParser_WriteFileEntry_LimitReach_stub(CopyCtrlFileEntry &fileEntry)
{
    std::cout << "CopyCtrlParser_WriteFileEntry_LimitReach stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::LIMIT_REACHED;
}


static CTRL_FILE_RETCODE MtimeCtrlParser_WriteEntry_LimitReach_stub(MtimeCtrlEntry &mtimeEntry)
{
    std::cout << "MtimeCtrlParser_WriteEntry_LimitReach stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::LIMIT_REACHED;
}

static CTRL_FILE_RETCODE DeleteCtrlParser_WriteDirEntry_LimitReach_stub(DeleteCtrlEntry &deleteEntry)
{
    std::cout << "DeleteCtrlParser_WriteDirEntry_LimitReach stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::LIMIT_REACHED;
}

static CTRL_FILE_RETCODE DeleteCtrlParser_WriteFileEntry_LimitReach_stub(std::string  &m_fileName)
{
    std::cout << "DeleteCtrlParser_WriteFileEntry_LimitReach stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::LIMIT_REACHED;
}

// write failed stup

static CTRL_FILE_RETCODE CopyCtrlParser_WriteDirEntry_FAILED_stub(CopyCtrlDirEntry &dirEntry)
{
    std::cout << "CopyCtrlParser_WriteDirEntry_FAILED stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::FAILED;
}

static CTRL_FILE_RETCODE CopyCtrlParser_WriteFileEntry_FAILED_stub(CopyCtrlFileEntry &fileEntry)
{
    std::cout << "CopyCtrlParser_WriteFileEntry_FAILED stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::FAILED;
}


static CTRL_FILE_RETCODE MtimeCtrlParser_WriteEntry_FAILED_stub(MtimeCtrlEntry &mtimeEntry)
{
    std::cout << "MtimeCtrlParser_WriteEntry_FAILED stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::FAILED;
}

static CTRL_FILE_RETCODE DeleteCtrlParser_WriteDirEntry_FAILED_stub(DeleteCtrlEntry &deleteEntry)
{
    std::cout << "DeleteCtrlParser_WriteDirEntry_FAILED stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::FAILED;
}

static CTRL_FILE_RETCODE DeleteCtrlParser_WriteFileEntry_FAILED_stub(std::string  &m_fileName)
{
    std::cout << "DeleteCtrlParser_WriteFileEntry_FAILED stub invoked" << std::endl;
    return CTRL_FILE_RETCODE::FAILED;
}



static uint64_t GetCurrentOffset_stub()
{
    return 114;
}

static std::string GetFileName_stub()
{
    return "filename";
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

    scanConfig.dFilter = ScanDirectoryFilter{FILTER_TYPE::DISABLED, {}};
    scanConfig.fFilter = ScanFileFilter{FILTER_TYPE::EXCLUDE, {
        "/4.txt"
    }};
}

static void FillScanConfigForSnapdiff(ScanConfig &scanConfig)
{
    scanConfig.scanType = ScanJobType::SNAPDIFFNAS_GEN;
    scanConfig.scanIO = IOEngine::SNAPDIFFNAS;
    scanConfig.nasSnapdiffProtocol = NAS_PROTOCOL::SMB;
    scanConfig.scanCheckPointEnable = false;
    scanConfig.lastBackupTime = 0;

    scanConfig.baseSnapshotName = "TEST_SNAPSHOT_1";
    scanConfig.incSnapshotName = "TEST_SNAPSHOT_2";

    scanConfig.deviceResourceName = "/nasfs/"; // illegal name, just for test
    scanConfig.deviceUrl = "11.4.5.14";
    scanConfig.devicePort = "8088";
    scanConfig.deviceUsername = "admin";
    scanConfig.devicePassword = "Huawei@123";
    scanConfig.deviceCert = "";
    scanConfig.devicePoolID = 0;
}

class NasSnapdiffServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    std::shared_ptr<StatisticsMgr> statsMgr {};
    std::shared_ptr<BufferQueue<SnapdiffResultMap>> snapdiffBuffer;
    ScanConfig scanConfig {};
    std::string enqueuePath {"/"};
    Stub stub;
};

void NasSnapdiffServiceTest::SetUp()
{
    stub.set(ADDR(FileParser, Open), Open_stub);
    stub.set(ADDR(FileParser, Close), Close_stub);

    stub.set(ADDR(XMetaParser, Open), Open_stub);
    stub.set(ADDR(XMetaParser, Close), Close_stub);
    stub.set(ADDR(XMetaParser, WriteXMeta), XMetaParser_WriteXMeta_stub);
    stub.set(ADDR(XMetaParser, GetCurrentOffset), GetCurrentOffset_stub);
    stub.set(ADDR(XMetaParser, GetFileName), GetFileName_stub);

    stub.set(ADDR(MetaParser, Open), Open_stub);
    stub.set(ADDR(MetaParser, Close), Close_stub);
    stub.set(ADDR(MetaParser, WriteDirectoryMeta), MetaParser_WriteDirectoryMeta_stub);
    stub.set(ADDR(MetaParser, WriteFileMeta), MetaParser_WriteFileMeta_stub);
    stub.set(ADDR(MetaParser, GetCurrentOffset), GetCurrentOffset_stub);
    stub.set(ADDR(MetaParser, GetFileName), GetFileName_stub);
    
    stub.set(ADDR(CopyCtrlParser, Open), Open_stub);
    stub.set(ADDR(CopyCtrlParser, Close), Close_stub);
    stub.set(ADDR(CopyCtrlParser, WriteDirEntry), CopyCtrlParser_WriteDirEntry_stub);
    stub.set(ADDR(CopyCtrlParser, WriteFileEntry), CopyCtrlParser_WriteFileEntry_stub);
    
    stub.set(ADDR(MtimeCtrlParser, Open), Open_stub);
    stub.set(ADDR(MtimeCtrlParser, Close), Close_stub);
    stub.set(ADDR(MtimeCtrlParser, WriteEntry), MtimeCtrlParser_WriteEntry_stub);

    stub.set(ADDR(DeleteCtrlParser, Open), Open_stub);
    stub.set(ADDR(DeleteCtrlParser, Close), Close_stub);
    stub.set(ADDR(DeleteCtrlParser, WriteDirEntry), DeleteCtrlParser_WriteDirEntry_stub);
    stub.set(ADDR(DeleteCtrlParser, WriteFileEntry), DeleteCtrlParser_WriteFileEntry_stub);

    stub.set(ADDR(SmbContextWrapper, Init), Init_stub);
    stub.set(ADDR(SmbContextWrapper, SmbConnect), SmbConnect_stub);
    stub.set(ADDR(SmbContextWrapper, SmbStat64), SmbStat64_stub);
    stub.set(ADDR(SmbContextWrapper, SmbDisconnect), SmbDisconnect_stub);
    stub.set(ADDR(SnapdiffService, GetSmbAcl), GetSmbAcl_stub);
    stub.set(ADDR(SmbContextWrapper, SmbGetClientGuid), SmbGetClientGuid_stub);

    stub.set(ADDR(NfsContextWrapper, NfsMount), NfsMount_stub);
    stub.set(ADDR(NfsContextWrapper, NfsDestroyContext), NfsDestroyContext_stub);
    stub.set(ADDR(NfsContextWrapper, NfsLstat64), NfsLstat64_stub);

    stub.set(FS_SCANNER::Rename, Rename_stub);

    FillCommonScanConfig(scanConfig);
    FillScanConfigForSnapdiff(scanConfig);
    statsMgr = make_shared<StatisticsMgr>();
    snapdiffBuffer = make_shared<BufferQueue<SnapdiffResultMap>>(MAX_QUEUE_LIMIT);

    snapdiffBuffer->BlockingPush(SnapdiffResultMap {
        {"/home",std::list<Module::SnapdiffMetadataInfo> {
            {
                0,
                SNAPDIFF_BACKUPENTRY_CHANGETYPE::DELETE,
                "/home/1.txt",
                SNAPDIFF_METADATA_INFO_F_TYPE::FILE_TYPE, 
                0,0,0,0,0,0,0,0,0,0
            }, {
                0,
                SNAPDIFF_BACKUPENTRY_CHANGETYPE::DELETE,
                "/home/mars",
                SNAPDIFF_METADATA_INFO_F_TYPE::DIR_TYPE,
                0,0,0,0,0,0,0,0,0,0
            },{
                0,
                SNAPDIFF_BACKUPENTRY_CHANGETYPE::NEW,
                "/home/2.txt",
                SNAPDIFF_METADATA_INFO_F_TYPE::FILE_TYPE, 
                0,0,0,0,0,0,0,0,0,0
            }, {
                0,
                SNAPDIFF_BACKUPENTRY_CHANGETYPE::NEW,
                "/home/jupyter",
                SNAPDIFF_METADATA_INFO_F_TYPE::DIR_TYPE,
                0,0,0,0,0,0,0,0,0,0
            },{
                0,
                SNAPDIFF_BACKUPENTRY_CHANGETYPE::META,
                "/home/3.txt",
                SNAPDIFF_METADATA_INFO_F_TYPE::FILE_TYPE, 
                0,0,0,0,0,0,0,0,0,0
            }, {
                0,
                SNAPDIFF_BACKUPENTRY_CHANGETYPE::META,
                "/home/earth",
                SNAPDIFF_METADATA_INFO_F_TYPE::DIR_TYPE,
                0,0,0,0,0,0,0,0,0,0
            }
        }},
    });

    snapdiffBuffer->SetPushFinished();
}

void NasSnapdiffServiceTest::TearDown()
{}

void NasSnapdiffServiceTest::SetUpTestCase()
{}

void NasSnapdiffServiceTest::TearDownTestCase()
{}

TEST_F(NasSnapdiffServiceTest, TEST_SNAPDIFF_SERVICE_SMB_COMMON)
{
    SnapdiffService snapdiffService(scanConfig, snapdiffBuffer, statsMgr);
    EXPECT_TRUE(snapdiffService.InitControlFilesAndMetaFile());
    EXPECT_EQ(snapdiffService.Start(), SCANNER_STATUS::SUCCESS);
}


TEST_F(NasSnapdiffServiceTest, TEST_SNAPDIFF_SERVICE_NFS_COMMON)
{
    scanConfig.nasSnapdiffProtocol = NAS_PROTOCOL::NFS;

    SnapdiffService snapdiffService(scanConfig, snapdiffBuffer, statsMgr);
    EXPECT_TRUE(snapdiffService.InitControlFilesAndMetaFile());
    EXPECT_EQ(snapdiffService.Start(), SCANNER_STATUS::SUCCESS);
}


TEST_F(NasSnapdiffServiceTest, TEST_SNAPDIFF_SERVICE_WRITE_LIMIT_REACH)
{
    stub.set(ADDR(CopyCtrlParser, WriteDirEntry), CopyCtrlParser_WriteDirEntry_LimitReach_stub);
    stub.set(ADDR(CopyCtrlParser, WriteFileEntry), CopyCtrlParser_WriteFileEntry_LimitReach_stub);
    stub.set(ADDR(MtimeCtrlParser, WriteEntry), MtimeCtrlParser_WriteEntry_LimitReach_stub);
    stub.set(ADDR(DeleteCtrlParser, WriteDirEntry), DeleteCtrlParser_WriteDirEntry_LimitReach_stub);
    stub.set(ADDR(DeleteCtrlParser, WriteFileEntry), DeleteCtrlParser_WriteFileEntry_LimitReach_stub);

    scanConfig.scanMetaFileSize = 1; // force xmeta and meta writer to reach limit

    SnapdiffService snapdiffService(scanConfig, snapdiffBuffer, statsMgr);
    EXPECT_TRUE(snapdiffService.InitControlFilesAndMetaFile());
    EXPECT_EQ(snapdiffService.Start(), SCANNER_STATUS::SUCCESS);
}

TEST_F(NasSnapdiffServiceTest, TEST_SNAPDIFF_SERVICE_WRITE_FAILED)
{
    stub.set(ADDR(CopyCtrlParser, WriteDirEntry), CopyCtrlParser_WriteDirEntry_FAILED_stub);
    stub.set(ADDR(CopyCtrlParser, WriteFileEntry), CopyCtrlParser_WriteFileEntry_FAILED_stub);
    stub.set(ADDR(MtimeCtrlParser, WriteEntry), MtimeCtrlParser_WriteEntry_FAILED_stub);
    stub.set(ADDR(DeleteCtrlParser, WriteDirEntry), DeleteCtrlParser_WriteDirEntry_FAILED_stub);
    stub.set(ADDR(DeleteCtrlParser, WriteFileEntry), DeleteCtrlParser_WriteFileEntry_FAILED_stub);

    SnapdiffService snapdiffService(scanConfig, snapdiffBuffer, statsMgr);
    EXPECT_TRUE(snapdiffService.InitControlFilesAndMetaFile());
    EXPECT_EQ(snapdiffService.Start(), SCANNER_STATUS::SUCCESS);
}

TEST_F(NasSnapdiffServiceTest, TEST_SNAPDIFF_SERVICE_INITMETA_FAILED)
{
    stub.set(ADDR(MetaParser, Open), Open_failed_stub);

    SnapdiffService snapdiffService(scanConfig, snapdiffBuffer, statsMgr);
    EXPECT_FALSE(snapdiffService.InitControlFilesAndMetaFile());
}

TEST_F(NasSnapdiffServiceTest, TEST_SNAPDIFF_SERVICE_INITXMETA_FAILED)
{
    stub.set(ADDR(XMetaParser, Open), Open_failed_stub);

    SnapdiffService snapdiffService(scanConfig, snapdiffBuffer, statsMgr);
    EXPECT_FALSE(snapdiffService.InitControlFilesAndMetaFile());
}

TEST_F(NasSnapdiffServiceTest, TEST_SNAPDIFF_SERVICE_INITCOPYCTRL_FAILED)
{
    stub.set(ADDR(CopyCtrlParser, Open), Open_failed_stub);

    SnapdiffService snapdiffService(scanConfig, snapdiffBuffer, statsMgr);
    EXPECT_FALSE(snapdiffService.InitControlFilesAndMetaFile());
}

TEST_F(NasSnapdiffServiceTest, TEST_SNAPDIFF_SERVICE_INITMTIMECTRL_FAILED)
{
    stub.set(ADDR(MtimeCtrlParser, Open), Open_failed_stub);

    SnapdiffService snapdiffService(scanConfig, snapdiffBuffer, statsMgr);
    EXPECT_FALSE(snapdiffService.InitControlFilesAndMetaFile());
}


TEST_F(NasSnapdiffServiceTest, TEST_SNAPDIFF_SERVICE_INITDELETECTRL_FAILED)
{
    stub.set(ADDR(DeleteCtrlParser, Open), Open_failed_stub);

    SnapdiffService snapdiffService(scanConfig, snapdiffBuffer, statsMgr);
    EXPECT_FALSE(snapdiffService.InitControlFilesAndMetaFile());
}