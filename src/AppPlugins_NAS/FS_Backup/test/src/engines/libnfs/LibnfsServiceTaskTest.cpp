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
#include <stdexcept>
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "LibnfsDeleteWriter.h"
#include "LibnfsDirMetaWriter.h"
#include "LibnfsServiceTask.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;
using namespace Libnfscommonmethods;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LibnfsServiceTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    LibnfsParams m_params {};
    FileHandle m_fileHandle {};
    std::shared_ptr<BackupControlInfo> m_controlInfo { nullptr };
    std::shared_ptr<PacketStats> m_pktStats = nullptr;
    NfsContextContainer m_nfsContextContainer;

    std::unique_ptr<LibnfsServiceTask> m_libnfsServiceTask = nullptr;
};

void LibnfsServiceTaskTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.commonParams.skipFailure = false;
    m_backupParams.commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;

    m_fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    m_fileHandle.m_retryCnt = 1;
    m_fileHandle.m_file->m_fileName = "/1.txt";

    bool abort = false;
    std::shared_ptr<LibnfsBackupAdvanceParams> m_advParams =
        dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams);
    std::vector<FileHandle> fileHandleList {};
    fileHandleList.push_back(m_fileHandle);
    m_params.jobStartTime = 0;
    m_params.deleteJobStartTime = 5;
    m_params.dstRootPath = m_advParams->dataPath;
    m_params.backupParams = m_backupParams;
    m_params.fileHandleCache = make_shared<FileHandleCache>();
    m_params.nfsCtx = m_nfsContextContainer.GetCurrContext();
    m_params.abort = &abort;
    m_params.fileHandleList = fileHandleList;
    
    m_controlInfo = std::make_shared<BackupControlInfo>();
    m_pktStats = make_shared<PacketStats>();

    m_libnfsServiceTask = make_unique<LibnfsServiceTask>(LibnfsEvent::DELETE, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
}

void LibnfsServiceTaskTest::TearDown()
{
    GlobalMockObject::verify(); // ??mock?????mock??
}

void LibnfsServiceTaskTest::SetUpTestCase()
{}

void LibnfsServiceTaskTest::TearDownTestCase()
{}


static nfsfh* Get_Stub()
{
    struct nfsfh *nfsFh = nullptr;
    return nfsFh;
}


static nfsfh* Get_Stub1()
{
    struct nfsfh *nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    return nfsFh;
}

TEST_F(LibnfsServiceTaskTest, Exec)
{
    MOCKER_CPP(&LibnfsServiceTask::HandleDelete)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsServiceTask->Exec();

    MOCKER_CPP(&LibnfsServiceTask::HandleWriteMeta)
            .stubs()
            .will(ignoreReturnValue());

    auto task = make_shared<LibnfsServiceTask>(LibnfsEvent::WRITE_META, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
    EXPECT_NO_THROW(task->Exec());

    auto task1 = make_shared<LibnfsServiceTask>(LibnfsEvent::INVALID, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
    EXPECT_NO_THROW(task1->Exec());
}

TEST_F(LibnfsServiceTaskTest, HandleWriteMeta)
{
    MOCKER_CPP(&FileHandleCache::Get)
            .stubs()
            .will(invoke(Get_Stub))
            .then(invoke(Get_Stub))
            .then(invoke(Get_Stub))
            .then(invoke(Get_Stub1))
            .then(invoke(Get_Stub1));
    MOCKER_CPP(&LibnfsServiceTask::LookupRecursively)
            .stubs()
            .will(returnValue(MP_FAILED))
            .then(returnValue(MP_SUCCESS));
    EXPECT_NO_THROW(m_libnfsServiceTask->HandleWriteMeta());

    //m_libnfsServiceTask->HandleWriteMeta();

    MOCKER_CPP(&NfsContextWrapper::NfsChmodChownUtimeLock)
            .stubs()
            .will(returnValue(MP_FAILED))
            .then(returnValue(MP_SUCCESS));
    MOCKER_CPP(&LibnfsServiceTask::HandleDirSetMetaFailure)
            .stubs()
            .will(ignoreReturnValue());
    //m_libnfsServiceTask->HandleWriteMeta();

    //m_libnfsServiceTask->HandleWriteMeta();
}

TEST_F(LibnfsServiceTaskTest, HandleDelete)
{
    m_libnfsServiceTask->HandleDelete();

    MOCKER_CPP(&lstat)
            .stubs()
            .will(returnValue(MP_SUCCESS));
    EXPECT_NO_THROW(m_libnfsServiceTask->HandleDelete());

    m_fileHandle.m_file->m_fileName = "";
    auto task = make_shared<LibnfsServiceTask>(LibnfsEvent::WRITE_META, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
    
    EXPECT_NO_THROW(task->HandleDelete());

    m_fileHandle.m_file->m_fileName = "./f1.txt";
    auto task1 = make_shared<LibnfsServiceTask>(LibnfsEvent::WRITE_META, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
    
    EXPECT_NO_THROW(task1->HandleDelete());
}

TEST_F(LibnfsServiceTaskTest, HandleLstatStatus)
{
    std::string fullPath = "/1.txt";
    struct stat st {};
    st.st_ctime = 1;
    st.st_nlink = 1;
    int delStatIsDir = 1;

    MOCKER_CPP(&LibnfsServiceTask::DetermineDirectoryOrFile)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    EXPECT_EQ(m_libnfsServiceTask->HandleLstatStatus(fullPath, st, delStatIsDir), MP_FAILED);

    st.st_nlink = 2;
    st.st_ctime = 20;
    MOCKER_CPP(&LibnfsServiceTask::CompareTypeOfDeleteEntryAndBackupCopy)
            .stubs()
            .will(returnValue(MP_FAILED))
            .then(returnValue(MP_SUCCESS));
    EXPECT_EQ(m_libnfsServiceTask->HandleLstatStatus(fullPath, st, delStatIsDir), MP_FAILED);

    EXPECT_EQ(m_libnfsServiceTask->HandleLstatStatus(fullPath, st, delStatIsDir), MP_SUCCESS);

    
    m_backupParams.commonParams.restoreReplacePolicy = RestoreReplacePolicy::IGNORE_EXIST;
    m_params.backupParams = m_backupParams;

    auto task = make_shared<LibnfsServiceTask>(LibnfsEvent::WRITE_META, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
    
    EXPECT_EQ(task->HandleLstatStatus(fullPath, st, delStatIsDir), MP_FAILED);
}

TEST_F(LibnfsServiceTaskTest, CompareTypeOfDeleteEntryAndBackupCopy)
{
    int delStatIsDir = 0;
    m_fileHandle.m_file->SetFlag(IS_DIR);

    auto task = make_shared<LibnfsServiceTask>(LibnfsEvent::WRITE_META, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);

    EXPECT_EQ(task->CompareTypeOfDeleteEntryAndBackupCopy(delStatIsDir), MP_FAILED);

    m_fileHandle.m_file->ClearFlag(IS_DIR);

    auto task1 = make_shared<LibnfsServiceTask>(LibnfsEvent::WRITE_META, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
    delStatIsDir = 1;

    EXPECT_EQ(task1->CompareTypeOfDeleteEntryAndBackupCopy(delStatIsDir), MP_FAILED);

    delStatIsDir = 0;
    EXPECT_EQ(task1->CompareTypeOfDeleteEntryAndBackupCopy(delStatIsDir), MP_SUCCESS);
}

TEST_F(LibnfsServiceTaskTest, DeleteFilesAndDirectory)
{
    std::string fileName = "/d1";
    int delStatIsDir = 1;

    MOCKER_CPP(&LibnfsServiceTask::DeleteDirectory)
            .stubs()
            .will(returnValue(MP_SUCCESS));
    EXPECT_NO_THROW(m_libnfsServiceTask->DeleteFilesAndDirectory(fileName, delStatIsDir));

    fileName = "/1.txt";
    delStatIsDir = 0;

    MOCKER_CPP(&LibnfsServiceTask::DeleteFile)
            .stubs()
            .will(returnValue(MP_SUCCESS));
    EXPECT_NO_THROW(m_libnfsServiceTask->DeleteFilesAndDirectory(fileName, delStatIsDir));
}

TEST_F(LibnfsServiceTaskTest, DeleteDirectory)
{
    std::string fPath = "/d1";
    EXPECT_EQ(m_libnfsServiceTask->DeleteDirectory(fPath), -1);

    std::string fPath1 = "./d";
    EXPECT_EQ(m_libnfsServiceTask->DeleteDirectory(fPath1), -1);
}

TEST_F(LibnfsServiceTaskTest, DeleteFile)
{
    std::string fPath = "/1.txt";
    EXPECT_EQ(m_libnfsServiceTask->DeleteFile(fPath), -1);

    std::string fPath1 = "./d.txt";
    EXPECT_EQ(m_libnfsServiceTask->DeleteFile(fPath1), -1);
}

TEST_F(LibnfsServiceTaskTest, UnlinkCb)
{
    const char fpath {};
    const struct stat sb {};
    int typeflag = 0;
    struct FTW ftwbuf {};
    EXPECT_NO_THROW(m_libnfsServiceTask->UnlinkCb(&fpath, &sb, typeflag, &ftwbuf));
}

TEST_F(LibnfsServiceTaskTest, DetermineDirectoryOrFile)
{
    struct stat st {};
    int delStatIsDir = 0;
    EXPECT_NO_THROW(m_libnfsServiceTask->DetermineDirectoryOrFile(st, delStatIsDir));
}

static string Get_ParentDir_Stub()
{
    std::string dir = "/";
    return dir;
}

static string Get_ParentDir_Stub1()
{
    std::string dir = "";
    return dir;
}

TEST_F(LibnfsServiceTaskTest, IsDirMetaSetRequired)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    MOCKER_CPP(&FSBackupUtils::GetParentDir)
            .stubs()
            .will(invoke(Get_ParentDir_Stub))
            .then(invoke(Get_ParentDir_Stub1))
            .then(invoke(Get_ParentDir_Stub1));

    MOCKER_CPP(&FileHandleCache::Get)
            .stubs()
            .will(invoke(Get_Stub))
            .then(invoke(Get_Stub))
            .then(invoke(Get_Stub1));
    //EXPECT_EQ(m_libnfsServiceTask->IsDirMetaSetRequired(fileHandle), false);

    fileHandle.m_file->m_fileName = "";
    MOCKER_CPP(&NfsContextWrapper::NfsLstat64Lock)
            .stubs()
            .will(returnValue(MP_FAILED));
    MOCKER_CPP(&LibnfsServiceTask::HandleDirLstatStatus)
            .stubs()
            .will(returnValue(MP_FAILED))
            .then(returnValue(MP_SUCCESS));
    EXPECT_EQ(m_libnfsServiceTask->IsDirMetaSetRequired(fileHandle), false);

    MOCKER_CPP(&NfsContextWrapper::NfsLstat64WithParentFhLock)
            .stubs()
            .will(returnValue(MP_FAILED));
    EXPECT_EQ(m_libnfsServiceTask->IsDirMetaSetRequired(fileHandle), true);
}

TEST_F(LibnfsServiceTaskTest, HandleDirLstatStatus)
{
    int status = -BACKUP_ERR_ENOENT;
    struct nfs_stat_64 st {};
    EXPECT_EQ(m_libnfsServiceTask->HandleDirLstatStatus(status, m_fileHandle, st), MP_SUCCESS);

    status = -EINTR;
    MOCKER_CPP(&LibnfsServiceTask::HandleDirLstatRetry)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsServiceTask->HandleDirLstatStatus(status, m_fileHandle, st), MP_FAILED);

    status = -1;
    EXPECT_EQ(m_libnfsServiceTask->HandleDirLstatStatus(status, m_fileHandle, st), MP_FAILED);

    status = 0;
    MOCKER_CPP(&LibnfsServiceTask::IsDirAlreadyExistedInTargetFS)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsServiceTask->HandleDirLstatStatus(status, m_fileHandle, st), MP_FAILED);

    EXPECT_EQ(m_libnfsServiceTask->HandleDirLstatStatus(status, m_fileHandle, st), MP_SUCCESS);
}

TEST_F(LibnfsServiceTaskTest, IsDirAlreadyExistedInTargetFS)
{
    struct nfs_stat_64 st {};
    st.nfs_ctime = 0;
    EXPECT_EQ(m_libnfsServiceTask->IsDirAlreadyExistedInTargetFS(m_fileHandle, st), false);

    m_params.jobStartTime = 10;
    auto task1 = make_shared<LibnfsServiceTask>(LibnfsEvent::INVALID, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);

    EXPECT_EQ(task1->IsDirAlreadyExistedInTargetFS(m_fileHandle, st), true);
}

TEST_F(LibnfsServiceTaskTest, HandleDirLstatRetry)
{
    EXPECT_NO_THROW(m_libnfsServiceTask->HandleDirLstatRetry(m_fileHandle));
}

TEST_F(LibnfsServiceTaskTest, HandleDirSetMetaFailure)
{
    int status = -BACKUP_ERR_ENOENT;
    EXPECT_NO_THROW(m_libnfsServiceTask->HandleDirSetMetaFailure(status, m_fileHandle));

    status = -EINTR;
    MOCKER_CPP(&LibnfsServiceTask::HandleDirMetaRetry)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsServiceTask->HandleDirSetMetaFailure(status, m_fileHandle));

    status = -1;
    EXPECT_NO_THROW(m_libnfsServiceTask->HandleDirSetMetaFailure(status, m_fileHandle));
}

TEST_F(LibnfsServiceTaskTest, LookupRecursively)
{
    std::string targetFilePath = "/d1";
    MOCKER_CPP(&LibnfsServiceTask::LookupDir)
            .stubs()
            .will(returnValue(MP_SUCCESS));
    EXPECT_EQ(m_libnfsServiceTask->LookupRecursively(targetFilePath), MP_SUCCESS);
}

static nfs_context* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfsContext;
}

TEST_F(LibnfsServiceTaskTest, LookupDir)
{
    std::string dirPath = "/d1";
    std::string parentDirPath = "/";
    bool lookupFailed = true;

    MOCKER_CPP(&LibnfsServiceTask::FillFileHandleCacheWithInvalidDirectoryFh)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsServiceTask->LookupDir(dirPath, parentDirPath, lookupFailed), MP_FAILED);

    lookupFailed = false;
    MOCKER_CPP(&LibnfsServiceTask::LookupSync)
            .stubs()
            .will(returnValue(MP_FAILED))
            .then(returnValue(MP_SUCCESS));
    EXPECT_EQ(m_libnfsServiceTask->LookupDir(dirPath, parentDirPath, lookupFailed), MP_SUCCESS);

    bool abort = true;
    m_params.abort = &abort;
    MOCKER_CPP(&LibnfsServiceTask::FillFileHandleCacheWithInvalidDirectoryFh)
            .stubs()
            .will(ignoreReturnValue());

    auto task1 = make_shared<LibnfsServiceTask>(LibnfsEvent::INVALID, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
    EXPECT_EQ(task1->LookupDir(dirPath, parentDirPath, lookupFailed), MP_FAILED);
}

TEST_F(LibnfsServiceTaskTest, LookupSync)
{
    std::string dirPath = "/d1";
    std::string parentDirPath = "/";
    uint16_t retryCnt = 0;

    MOCKER_CPP(&FileHandleCache::Get)
            .stubs()
            .will(invoke(Get_Stub1));
    EXPECT_EQ(m_libnfsServiceTask->LookupSync(dirPath, parentDirPath, retryCnt), MP_SUCCESS);

    //MOCKER_CPP(&Libnfscommonmethods::IsValidNfsFh)
    //        .stubs()
    //        .will(returnValue(MP_FAILED))
    //        .then(returnValue(MP_SUCCESS));
    //EXPECT_EQ(m_libnfsServiceTask->LookupSync(dirPath, parentDirPath, retryCnt), MP_FAILED);

    //MOCKER_CPP(&NfsContextWrapper::NfsLookupGetFhWithParentFhLock)
    //        .stubs()
    //        .will(returnValue(MP_SUCCESS));
    //MOCKER_CPP(&LibnfsServiceTask::HandleLookupSyncReqStatus)
    //        .stubs()
    //        .will(returnValue(MP_SUCCESS))
    //        .then(returnValue(MP_SUCCESS));
    //EXPECT_EQ(m_libnfsServiceTask->LookupSync(dirPath, parentDirPath, retryCnt), MP_SUCCESS);
//
    //MOCKER_CPP(&NfsContextWrapper::NfsLookupGetFhLock)
    //        .stubs()
    //        .will(returnValue(MP_SUCCESS));
    //EXPECT_EQ(m_libnfsServiceTask->LookupSync(dirPath, parentDirPath, retryCnt), MP_SUCCESS);
    
}

TEST_F(LibnfsServiceTaskTest, HandleLookupSyncReqStatus)
{
    int status = -EINTR;
    uint16_t retryCnt = 0;
    struct nfsfh *nfsfh = nullptr;
    std::string fileName = "/1.txt";
    shared_ptr<NfsContextWrapper> nfs = m_params.nfsCtx;

    MOCKER_CPP(&NfsContextWrapper::GetNfsContext)
            .stubs()
            .will(invoke(NfsGetError_Stub))
            .then(invoke(NfsGetError_Stub));
    int ret = m_libnfsServiceTask->HandleLookupSyncReqStatus(status, retryCnt, nfsfh, fileName, nfs);
    EXPECT_EQ(ret, MP_FAILED);

    status = -1;
    ret = m_libnfsServiceTask->HandleLookupSyncReqStatus(status, retryCnt, nfsfh, fileName, nfs);
    EXPECT_EQ(ret, MP_FAILED);

    status = 0;
    ret = m_libnfsServiceTask->HandleLookupSyncReqStatus(status, retryCnt, nfsfh, fileName, nfs);
    EXPECT_EQ(ret, MP_FAILED);

    nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&FileHandleCache::Push)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    ret = m_libnfsServiceTask->HandleLookupSyncReqStatus(status, retryCnt, nfsfh, fileName, nfs);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsServiceTaskTest, FillFileHandleCacheWithInvalidDirectoryFh)
{
    std::string dirPath = "/d1";
    MOCKER_CPP(&FileHandleCache::Get)
            .stubs()
            .will(invoke(Get_Stub));
    MOCKER_CPP(&FileHandleCache::Push)
            .stubs()
            .will(returnValue(false));
    EXPECT_NO_THROW(m_libnfsServiceTask->FillFileHandleCacheWithInvalidDirectoryFh(dirPath));
}

TEST_F(LibnfsServiceTaskTest, HandleDirMetaRetry)
{
    auto task = make_shared<LibnfsServiceTask>(LibnfsEvent::WRITE_META, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);

    EXPECT_NO_THROW(task->HandleDirMetaRetry(m_fileHandle));

    //WriterParams dirWriterParams {};
    //LibnfsDirMetaWriter libnfsDirMetaWriter(dirWriterParams);

    m_fileHandle.m_retryCnt = 11;
    //m_params.writeObj = &libnfsDirMetaWriter;
    auto task1 = make_shared<LibnfsServiceTask>(LibnfsEvent::WRITE_META, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);

    //task1->HandleDirMetaRetry(m_fileHandle);
}

TEST_F(LibnfsServiceTaskTest, HandleLstatRetry)
{
    m_libnfsServiceTask->HandleLstatRetry();

    WriterParams deleteWriterParams {};
    deleteWriterParams.backupParams = m_backupParams;
    LibnfsDeleteWriter libnfsDeleteWriter(deleteWriterParams);

    m_params.writeObj = &libnfsDeleteWriter;
    auto task = make_shared<LibnfsServiceTask>(LibnfsEvent::DELETE, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
    EXPECT_NO_THROW(task->HandleLstatRetry());

    m_fileHandle.m_retryCnt = 11;
    auto task1 = make_shared<LibnfsServiceTask>(LibnfsEvent::DELETE, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
    
    EXPECT_NO_THROW(task1->HandleLstatRetry());

    m_fileHandle.m_file->SetFlag(IS_DIR);
    auto task2 = make_shared<LibnfsServiceTask>(LibnfsEvent::DELETE, m_controlInfo,
        m_fileHandle, m_params, m_pktStats);
    
    EXPECT_NO_THROW(task2->HandleLstatRetry());
}
