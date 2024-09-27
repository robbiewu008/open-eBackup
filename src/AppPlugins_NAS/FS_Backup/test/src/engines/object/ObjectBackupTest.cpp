/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mockcpp/mockcpp.hpp"
#include "named_stub.h"

#include <memory>
#include <functional>
#include "log/Log.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/FSBackupUtils.h"

#include "ThreadPoolFactory.h"
#include "manager/CloudServiceManager.h"
#include "CloudServiceTest.h"

#include "BackupMgr.h"
#include "Utils.h"
#include "backup_layout/SqliteOps.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

/*
-------------test/conf/data------------------
data
├── randomtext01.txt
├── randomtext02.txt
└── randomtext03.txt
-------------test/conf/scan------------------
scan
├── ctrl
│   ├── control_0_1bc67b19a0b71000.txt
│   └── mtime_0_1bc67b19a4371002.txt
├── meta
│   ├── hcs-test-10
│   ├── latest
│   │   ├── dircache
│   │   ├── filecache_0
│   │   ├── meta_file_0
│   │   ├── metafile_count.txt
│   │   ├── scanner_status.txt
│   │   └── xmeta_file_0
│   └── previous
└── rfi

 */
namespace {
    std::string BUCKETNAME = "hcs-test-10";
    std::string CTRLFILE = "control_0_1bc67b19a0b71000.txt";
    std::string g_metaPath;
    std::string g_dataPath;
    std::string g_restorePath;
    std::string g_backupPath;
    std::string g_backupResult;
    BackupTestPara g_testBackupPara;
}

class ObjectBackupTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    LLTSTUB::Stub stub;
};

static std::unique_ptr<CloudServiceInterface> CreateInstStub(const StorageConfig& storageConfig)
{
    if (storageConfig.storageType == StorageType::HUAWEI) {
        return std::make_unique<CloudServiceTest>(storageConfig.verifyInfo, g_testBackupPara);
    } else {
        return nullptr;
    }
}

static void TestCreateDir(std::string& path)
{
    std::string cmd = "mkdir -p " + path;
    system(cmd.c_str());
}

static void TestDeleteDir(std::string& path)
{
    std::string cmd = "rm -rf " + path;
    system(cmd.c_str());
}

static size_t TestGetFileSize(std::string fileName)
{
    struct stat st {};
    if (::stat(fileName.c_str(), &st) != 0) {
        return 0;
    }
    return st.st_size;
}

void ObjectBackupTest::SetUp()
{
    g_dataPath = Module::CPath::GetInstance().GetConfPath() + "/data";
    g_metaPath = Module::CPath::GetInstance().GetConfPath() + "/scan";
    g_backupResult = Module::CPath::GetInstance().GetConfPath() + "/backup_out";
    g_backupPath = Module::CPath::GetInstance().GetConfPath() + "/backup";
    g_restorePath = Module::CPath::GetInstance().GetConfPath() + "/restore";
    g_testBackupPara.dataPath = g_dataPath;
    g_testBackupPara.dstPath = g_restorePath;

    TestCreateDir(g_backupPath);
    TestCreateDir(g_restorePath);
    stub.set(&CloudServiceManager::CreateInst, CreateInstStub);
}

void ObjectBackupTest::TearDown()
{
    stub.reset(&CloudServiceManager::CreateInst);
    CloudServiceTestReSet();
    TestDeleteDir(g_backupPath);
    TestDeleteDir(g_restorePath);
    memset_s(&g_testBackupPara, sizeof(g_testBackupPara), 0, sizeof(g_testBackupPara));
    GlobalMockObject::verify();
}

void ObjectBackupTest::SetUpTestCase()
{}

void ObjectBackupTest::TearDownTestCase()
{}

static std::shared_ptr<SQLiteDB> TestGetSqliteDB(const std::string &path)
{
    std::string sqlitePath = path + Module::PATH_SEPARATOR + "sqlite";
    std::string aliasPath = path + Module::PATH_SEPARATOR + "sqlitealias";

    FSBackupUtils::RecurseCreateDirectory(aliasPath);
    std::shared_ptr<Module::Snowflake> idGenerator = make_shared<Snowflake>();
    idGenerator->SetMachine(Module::GetMachineId());
    return make_shared<SQLiteDB>(sqlitePath, aliasPath, idGenerator);
}

static void FillCommonParams(BackupParams& backupParams)
{
    CommonParams commonParams {};
    commonParams.maxBufferCnt = 10;
    commonParams.maxBufferSize = 10 * 1024; // 10kb
    commonParams.maxErrorFiles = 100;
    commonParams.backupDataFormat = BackupDataFormat::NATIVE;
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    commonParams.jobId = "jobId";
    commonParams.subJobId = "subJobId";
    commonParams.skipFailure = true; // stop backup if any item backup failed.
    commonParams.blockSize = DEFAULT_BLOCK_SIZE;
    backupParams.commonParams = commonParams;

    ScanAdvanceParams scanAdvParams {};
    scanAdvParams.metaFilePath = g_metaPath + "/meta/latest";
    backupParams.scanAdvParams = scanAdvParams;
}

static void FillBackupParams(BackupParams& backupParams)
{
    backupParams.backupType = BackupType::BACKUP_FULL;
    backupParams.phase = BackupPhase::COPY_STAGE;
    backupParams.srcEngine = BackupIOEngine::OBJECTSTORAGE;
    backupParams.dstEngine = BackupIOEngine::POSIX;

    FillCommonParams(backupParams);
    backupParams.commonParams.writeAcl = false;
    backupParams.commonParams.writeMeta = false;
    backupParams.commonParams.writeExtendAttribute = false;

    ObjectBackupAdvanceParams srcParam {};
    ObsBucket bucket{};
    bucket.bucketName = BUCKETNAME;
    srcParam.buckets.push_back(bucket);
    srcParam.authArgs.storageType = StorageType::HUAWEI;
    srcParam.dataPath = g_backupPath;
    srcParam.threadNum = 1;
    backupParams.srcAdvParams = make_shared<ObjectBackupAdvanceParams>(srcParam);

    HostBackupAdvanceParams dstParam {};
    dstParam.dataPath = g_backupPath;
    dstParam.threadNum = 1;
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>(dstParam);
}

static void FillRestoreParams(BackupParams& backupParams)
{
    backupParams.backupType = BackupType::RESTORE;
    backupParams.phase = BackupPhase::COPY_STAGE;
    backupParams.srcEngine = BackupIOEngine::POSIX;
    backupParams.dstEngine = BackupIOEngine::OBJECTSTORAGE;

    FillCommonParams(backupParams);
    backupParams.commonParams.writeAcl = true;
    backupParams.commonParams.writeMeta = true;
    backupParams.commonParams.writeExtendAttribute = true;

    HostBackupAdvanceParams srcParam {};
    srcParam.dataPath = g_backupResult; // 与备份相反
    srcParam.threadNum = 1;
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>(srcParam);

    ObjectBackupAdvanceParams dstParam {};
    ObsBucket bucket{};
    bucket.bucketName = BUCKETNAME;
    dstParam.buckets.push_back(bucket);
    dstParam.authArgs.storageType = StorageType::HUAWEI;
    dstParam.dataPath = g_restorePath;
    dstParam.threadNum = 16;
    backupParams.dstAdvParams = make_shared<ObjectBackupAdvanceParams>(dstParam);
}

static bool MonitorBackup(unique_ptr<Backup>& backup)
{
    BackupPhaseStatus backupStatus;

    while (true) {
        backupStatus = backup->GetStatus();
        if (backupStatus == BackupPhaseStatus::COMPLETED) {
            break;
        }
        if (backupStatus != BackupPhaseStatus::INPROGRESS) {
            return false;
        }
        sleep(1);
    }

    return true;
}

static void CheckBackupResult()
{
    std::string file1 = g_backupPath + "/" + BUCKETNAME + "/dir_0/8677186132288471040";
    std::string file2 = g_backupPath + "/" + BUCKETNAME + "/dir_0/8159136742678200320";
    std::string file3 = g_backupPath + "/" + BUCKETNAME + "/dir_0/10710610406529040384";
    EXPECT_TRUE(Module::CFile::FileExist(file1));
    EXPECT_TRUE(Module::CFile::FileExist(file2));
    EXPECT_TRUE(Module::CFile::FileExist(file3));
    EXPECT_TRUE(TestGetFileSize(file1) == TestGetFileSize(g_dataPath + "/randomtext01.txt"));
    EXPECT_TRUE(TestGetFileSize(file2) == TestGetFileSize(g_dataPath + "/randomtext02.txt"));
    EXPECT_TRUE(TestGetFileSize(file3) == TestGetFileSize(g_dataPath + "/randomtext03.txt"));
}

static void CheckRestoreResult()
{
    std::string file1 = g_restorePath + "/" + BUCKETNAME + "/randomtext01.txt";
    std::string file2 = g_restorePath + "/" + BUCKETNAME + "/randomtext02.txt";
    std::string file3 = g_restorePath + "/" + BUCKETNAME + "/randomtext03.txt";
    EXPECT_TRUE(Module::CFile::FileExist(file1));
    EXPECT_TRUE(Module::CFile::FileExist(file2));
    EXPECT_TRUE(Module::CFile::FileExist(file3));
    EXPECT_TRUE(TestGetFileSize(file1) == TestGetFileSize(g_dataPath + "/randomtext01.txt"));
    EXPECT_TRUE(TestGetFileSize(file2) == TestGetFileSize(g_dataPath + "/randomtext02.txt"));
    EXPECT_TRUE(TestGetFileSize(file3) == TestGetFileSize(g_dataPath + "/randomtext03.txt"));
}

/* 全量备份成功 */
TEST_F(ObjectBackupTest, TestBackupFull_Success)
{
    ThreadPoolFactory::InitThreadPool(32, 32);
    BackupParams params {};
    FillBackupParams(params);
    g_testBackupPara.maxFailNum = 1;
    g_testBackupPara.SetFlag(BackupTest::GET_OBJECT_ERR);
    unique_ptr<Backup> backup = BackupMgr::CreateBackupInst(params);
    EXPECT_TRUE(backup != nullptr);

    std::string ctrlFile = g_metaPath + "/ctrl/" + CTRLFILE;

    EXPECT_TRUE(backup->Enqueue(ctrlFile) == BackupRetCode::SUCCESS);
    EXPECT_TRUE(backup->Start() == BackupRetCode::SUCCESS);
    EXPECT_TRUE(MonitorBackup(backup));
    CheckBackupResult();
}

/* 全量备份失败 */
TEST_F(ObjectBackupTest, TestBackupFull_Fail)
{
    ThreadPoolFactory::InitThreadPool(32, 32);
    BackupParams params {};
    FillBackupParams(params);
    g_testBackupPara.maxFailNum = 3;
    g_testBackupPara.SetFlag(BackupTest::GET_OBJECT_ERR);
    g_testBackupPara.errorCode = OBS_STATUS_AccessDenied;
    unique_ptr<Backup> backup = BackupMgr::CreateBackupInst(params);
    EXPECT_TRUE(backup != nullptr);

    std::string ctrlFile = g_metaPath + "/ctrl/" + CTRLFILE;

    EXPECT_TRUE(backup->Enqueue(ctrlFile) == BackupRetCode::SUCCESS);
    EXPECT_TRUE(backup->Start() == BackupRetCode::SUCCESS);
    EXPECT_FALSE(MonitorBackup(backup));
}

/* 增量备份删除 */
TEST_F(ObjectBackupTest, TestBackupDelete)
{
    BackupParams params {};
    FillBackupParams(params);
    params.backupType = BackupType::BACKUP_INC;
    params.phase = BackupPhase::DELETE_STAGE;
    params.commonParams.metaPath = g_backupResult;
    auto dstAdvParams = dynamic_pointer_cast<HostBackupAdvanceParams>(params.dstAdvParams);
    dstAdvParams->dataPath = g_backupResult;
    unique_ptr<Backup> backup = BackupMgr::CreateBackupInst(params);
    EXPECT_TRUE(backup != nullptr);

    std::string ctrlFile = g_metaPath + "/ctrl/" + "delete_control_1_522e392813b066.txt";
    EXPECT_TRUE(backup->Enqueue(ctrlFile) == BackupRetCode::SUCCESS);
    EXPECT_TRUE(backup->Start() == BackupRetCode::SUCCESS);
    EXPECT_TRUE(MonitorBackup(backup));

    // 检查数据文件删除成功
    std::string file1 = g_backupResult + "/" + BUCKETNAME + "/dir_0/8677186132288471040";
    std::string file2 = g_backupResult + "/" + BUCKETNAME + "/dir_0/8159136742678200320";
    std::string file3 = g_backupResult + "/" + BUCKETNAME + "/dir_0/10710610406529040384";
    EXPECT_FALSE(Module::CFile::FileExist(file1));
    EXPECT_FALSE(Module::CFile::FileExist(file2));
    EXPECT_TRUE(Module::CFile::FileExist(file3));
    EXPECT_TRUE(TestGetFileSize(file3) == TestGetFileSize(g_dataPath + "/randomtext03.txt"));

    // 检查sqlite中记录删除成功
    std::shared_ptr<SQLiteDB> sqliteDb = TestGetSqliteDB(g_backupResult);
    std::string dbFile = "/" + BUCKETNAME + "/dir_0/copymetadata.sqlite";
    IndexDetails indexInfo {};
    std::vector<IndexDetails> vecIndexInfo {};

    std::string fileName1 = "8159136742678200320";
    int32_t ret = indexInfo.QueryIndexInfoByName(sqliteDb, dbFile, vecIndexInfo, fileName1, "f");
    EXPECT_TRUE(ret == Module::SUCCESS);
    EXPECT_TRUE(vecIndexInfo.empty());

    std::string fileName2 = "8677186132288471040";
    ret = indexInfo.QueryIndexInfoByName(sqliteDb, dbFile, vecIndexInfo, fileName2, "f");
    EXPECT_TRUE(ret == Module::SUCCESS);
    EXPECT_TRUE(vecIndexInfo.empty());

    std::string fileName3 = "10710610406529040384";
    ret = indexInfo.QueryIndexInfoByName(sqliteDb, dbFile, vecIndexInfo, fileName3, "f");
    EXPECT_TRUE(ret == Module::SUCCESS);
    EXPECT_FALSE(vecIndexInfo.empty());

    std::string cmd = "cd " + Module::CPath::GetInstance().GetConfPath() + " && tar zxf backup.tar.gz && cd -";
    system(cmd.c_str());
}

/* 全量恢复成功 */
TEST_F(ObjectBackupTest, TestBackupRestore_Success)
{
    ThreadPoolFactory::InitThreadPool(32, 32);
    BackupParams params {};
    FillRestoreParams(params);
    g_testBackupPara.maxFailNum = 1;
    g_testBackupPara.SetFlag(BackupTest::PUT_OBJECT_PART_ERR);
    unique_ptr<Backup> backup = BackupMgr::CreateBackupInst(params);
    EXPECT_TRUE(backup != nullptr);

    std::string ctrlFile = g_metaPath + "/ctrl/" + CTRLFILE;

    EXPECT_TRUE(backup->Enqueue(ctrlFile) == BackupRetCode::SUCCESS);
    EXPECT_TRUE(backup->Start() == BackupRetCode::SUCCESS);
    EXPECT_TRUE(MonitorBackup(backup));
    CheckRestoreResult();
}

/* 全量恢复失败 */
TEST_F(ObjectBackupTest, TestBackupRestore_Fail)
{
    ThreadPoolFactory::InitThreadPool(32, 32);
    BackupParams params {};
    FillRestoreParams(params);
    g_testBackupPara.maxFailNum = 1;
    g_testBackupPara.SetFlag(BackupTest::PUT_OBJECT_PART_ERR);
    g_testBackupPara.errorCode = OBS_STATUS_RequestTimeTooSkewed;
    unique_ptr<Backup> backup = BackupMgr::CreateBackupInst(params);
    EXPECT_TRUE(backup != nullptr);

    std::string ctrlFile = g_metaPath + "/ctrl/" + CTRLFILE;

    EXPECT_TRUE(backup->Enqueue(ctrlFile) == BackupRetCode::SUCCESS);
    EXPECT_TRUE(backup->Start() == BackupRetCode::SUCCESS);
    EXPECT_FALSE(MonitorBackup(backup));
}

/* 全量恢复:只覆盖旧文件策略时，用户新修改的文件不覆盖 */
TEST_F(ObjectBackupTest, TestBackupRestore_OverWriteOlder)
{
    ThreadPoolFactory::InitThreadPool(32, 32);

    // 恢复前修改文件
    std::string filePath = g_restorePath + "/" + BUCKETNAME;
    TestCreateDir(filePath);

    std::string fileName1 = filePath + "/randomtext01.txt";
    std::string cmd1 = "echo aaaaa > " + fileName1;
    system(cmd1.c_str());
    struct stat oldAttr1 {};
    EXPECT_TRUE(::stat(fileName1.c_str(), &oldAttr1) == 0);

    std::string fileName2 = filePath + "/randomtext02.txt";
    std::string cmd2 = "echo bbbbb > " + fileName2;
    system(cmd2.c_str());
    struct stat oldAttr2 {};
    EXPECT_TRUE(::stat(fileName2.c_str(), &oldAttr2) == 0);

    BackupParams params {};
    FillRestoreParams(params);
    params.commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE_OLDER;
    unique_ptr<Backup> backup = BackupMgr::CreateBackupInst(params);
    EXPECT_TRUE(backup != nullptr);

    std::string ctrlFile = g_metaPath + "/ctrl/" + CTRLFILE;

    EXPECT_TRUE(backup->Enqueue(ctrlFile) == BackupRetCode::SUCCESS);
    EXPECT_TRUE(backup->Start() == BackupRetCode::SUCCESS);
    EXPECT_TRUE(MonitorBackup(backup));

    // randomtext01.txt 不应该被恢复，大小及修改时间应该不变
    struct stat newAttr1 {};
    EXPECT_TRUE(::stat(fileName1.c_str(), &newAttr1) == 0);
    EXPECT_TRUE(newAttr1.st_mtime == oldAttr1.st_mtime);
    EXPECT_TRUE(newAttr1.st_size == oldAttr1.st_size);

    // randomtext02.txt 不应该被恢复，大小及修改时间应该不变
    struct stat newAttr2 {};
    EXPECT_TRUE(::stat(fileName2.c_str(), &newAttr2) == 0);
    EXPECT_TRUE(newAttr2.st_mtime == oldAttr2.st_mtime);
    EXPECT_TRUE(newAttr2.st_size == oldAttr2.st_size);

    std::string fileName3 = g_restorePath + "/" + BUCKETNAME + "/randomtext03.txt";
    EXPECT_TRUE(Module::CFile::FileExist(fileName3));
    EXPECT_TRUE(TestGetFileSize(fileName3) == TestGetFileSize(g_dataPath + "/randomtext03.txt"));
}

/* 非聚合备份，产生sqlite文件 */
TEST_F(ObjectBackupTest, TestBackupGenSqlite)
{
    ThreadPoolFactory::InitThreadPool(32, 32);
    BackupParams params {};
    FillBackupParams(params);
    params.commonParams.genSqlite = true;
    params.commonParams.useSubJobSqlite = true;
    params.commonParams.metaPath = g_backupPath;

    unique_ptr<Backup> backup = BackupMgr::CreateBackupInst(params);
    EXPECT_TRUE(backup != nullptr);

    std::string ctrlFile = g_metaPath + "/ctrl/" + CTRLFILE;

    EXPECT_TRUE(backup->Enqueue(ctrlFile) == BackupRetCode::SUCCESS);
    EXPECT_TRUE(backup->Start() == BackupRetCode::SUCCESS);
    EXPECT_TRUE(MonitorBackup(backup));

    bool isComplete = false;
    EXPECT_TRUE(BackupMgr::MergedbFile(g_backupPath, isComplete) == Module::SUCCESS);
    EXPECT_TRUE(isComplete);

    std::string sqliteFile = g_backupPath + "/sqlite/" + BUCKETNAME + "/dir_0/copymetadata.sqlite";
    EXPECT_TRUE(Module::CFile::FileExist(sqliteFile));
    CheckBackupResult();
}
