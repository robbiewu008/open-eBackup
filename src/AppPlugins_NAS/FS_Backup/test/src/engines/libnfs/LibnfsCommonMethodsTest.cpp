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
#include "LibnfsCommonMethods.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;
using namespace Libnfscommonmethods;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LibnfsCommonMethodsTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    NfsCommonData commonData {};
    bool abort = false;
};

void LibnfsCommonMethodsTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    commonData.skipFailure = false;
    commonData.controlInfo = std::make_shared<BackupControlInfo>();
    commonData.abort = &abort;
    commonData.writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);

}

void LibnfsCommonMethodsTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibnfsCommonMethodsTest::SetUpTestCase()
{}

void LibnfsCommonMethodsTest::TearDownTestCase()
{}

static nfsfh* Get_Stub()
{
    struct nfsfh *nfsFh = nullptr;
    return nfsFh;
}

static int NfsUnlinkLock_Stub()
{
    int status = -BACKUP_ERR_ENOENT;
    return status;
}

static int NfsMount_Stub()
{
    int status = -EINTR;
    return status;
}

static uint64_t GetNfsReadMaxSizeLock_Stub()
{
    uint64_t val = 1;
    return val;
}

static uint64_t GetNfsWriteMaxSizeLock_Stub()
{
    uint64_t val = 1;
    return val;
}

static nfs_context* GetNfsContext_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfsContext;
}

TEST_F(LibnfsCommonMethodsTest, IsRetryMount)
{
    int status = -EPERM;
    IsRetryMount(status);

    status = -EFAULT;
    IsRetryMount(status);

    status = -EAGAIN;
    IsRetryMount(status);

    status = 0;
    IsRetryMount(status);
}

TEST_F(LibnfsCommonMethodsTest, DeleteAllFilesInsideRecursively)
{
    string filePath = "/1.txt";
    string rootPath = "rootPath";
    string nfsMntArgs = "auto-traverse-mounts=0";
    struct nfs_context *nfs = nullptr;
    struct nfsdirent *nfsdirent = nullptr;
    nfs = nfs_init_context();
    shared_ptr<NfsContextWrapper> nfsContextWrapper = make_shared<NfsContextWrapper>(rootPath, nfsMntArgs);
    MOCKER_CPP(&NfsContextWrapper::NfsOpendirLock)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&NfsContextWrapper::NfsReadDirLock)
            .stubs()
            .will(returnValue(nfsdirent))
            .then(returnValue(nfsdirent));
    MOCKER_CPP(&NfsContextWrapper::NfsCloseDirLock)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&NfsContextWrapper::NfsRmdirLock)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(DeleteAllFilesInsideRecursively(filePath, nfsContextWrapper), MP_SUCCESS);
}

TEST_F(LibnfsCommonMethodsTest, FreeDirFh)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "/1.txt";
    fileHandle.m_file->dstIOHandle.nfsFh = nullptr;
    FreeDirFh(fileHandle);

    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    memset_s(fileHandle.m_file->dstIOHandle.nfsFh, sizeof(struct nfsfh), 0, sizeof(struct nfsfh));
    fileHandle.m_file->dstIOHandle.nfsFh->fh.len = 64;
    fileHandle.m_file->dstIOHandle.nfsFh->fh.val = (char *) malloc(64);
    memset_s(fileHandle.m_file->dstIOHandle.nfsFh->fh.val, 64, 0, 64);
    FreeDirFh(fileHandle);
}

TEST_F(LibnfsCommonMethodsTest, FillFileHandleCacheWithInvalidDirectoryFh)
{
    string dirPath = "dir1";
    shared_ptr<FileHandleCache> fileHandleCache = make_shared<FileHandleCache>();
    FillFileHandleCacheWithInvalidDirectoryFh(dirPath, fileHandleCache);
}

TEST_F(LibnfsCommonMethodsTest, FreeNfsFh)
{
    struct nfsfh *nfsFh = (struct nfsfh *) malloc(sizeof(struct nfsfh));
    nfsFh->fh.val = nullptr;
    nfsFh->pagecache.entries = nullptr;
    FreeNfsFh(nfsFh);
}

TEST_F(LibnfsCommonMethodsTest, IsValidNfsFh)
{
    struct nfsfh *nfsFh = (struct nfsfh *) malloc(sizeof(struct nfsfh));
    nfsFh->fh.len = 1;
    EXPECT_EQ(IsValidNfsFh(nfsFh), MP_SUCCESS);

    nfsFh->fh.len = 0;
    EXPECT_EQ(IsValidNfsFh(nfsFh), MP_FAILED);
}

TEST_F(LibnfsCommonMethodsTest, NasServerCheck)
{
    NfsContextContainer nfsContextContainer {};
    std::shared_ptr<Module::NfsContextWrapper> nfsContext = nfsContextContainer.GetCurrContext();
    uint32_t serverCheckSleepTime = 1;
    uint32_t serverCheckRetry = 1;

    MOCKER_CPP(&NfsContextWrapper::NfsNullSyncLock)
            .stubs()
            .will(returnValue(MP_FAILED))
            .then(returnValue(MP_SUCCESS));
    EXPECT_EQ(NasServerCheck(nfsContext, serverCheckSleepTime, serverCheckRetry), MP_SUCCESS);
}

TEST_F(LibnfsCommonMethodsTest, GetRWSizeFromLibnfs)
{
    NfsContextContainer nfsContextContainer {};
    shared_ptr<NfsContextWrapper> rootNfs = nfsContextContainer.GetCurrContext();
    BackupParams backupParams {};

    backupParams.commonParams.blockSize = 10;
    
    MOCKER_CPP(&NfsContextWrapper::GetNfsReadMaxSizeLock)
            .stubs()
            .will(invoke(GetNfsReadMaxSizeLock_Stub));
    MOCKER_CPP(&NfsContextWrapper::GetNfsWriteMaxSizeLock)
            .stubs()
            .will(invoke(GetNfsWriteMaxSizeLock_Stub));
    GetRWSizeFromLibnfs(rootNfs, backupParams);
}

TEST_F(LibnfsCommonMethodsTest, FillNfsContextContainer)
{
    string rootPath = "/Dir";
    uint16_t contextCount = 1;
    NfsContextContainer nfsContextContainer {};
    BackupParams backupParams {};
    uint32_t serverCheckSleepTime = 1;

    MOCKER_CPP(&NfsContextWrapper::NfsMount)
            .stubs()
            .will(invoke(NfsMount_Stub))
            .then(returnValue(MP_FAILED));
    MOCKER_CPP(&Libnfscommonmethods::GetRWSizeFromLibnfs)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(FillNfsContextContainer(rootPath, contextCount, nfsContextContainer,
        backupParams, serverCheckSleepTime), false);   
}

TEST_F(LibnfsCommonMethodsTest, LibNfsDeleteDirectorySync)
{
    string dirName = "/d1";
    NfsContextContainer nfsContextContainer {};
    NfsContextContainer *syncNfsContextContainer = &nfsContextContainer;

    MOCKER_CPP(&Libnfscommonmethods::DeleteDirectory)
            .stubs()
            .will(returnValue(MP_FAILED));
    EXPECT_EQ(LibNfsDeleteDirectorySync(dirName, syncNfsContextContainer), MP_FAILED);
}

TEST_F(LibnfsCommonMethodsTest, DeleteDirectory)
{
    DeleteInfo deleteInfo {};
    deleteInfo.m_fileName = "/d1/1.txt";
    NfsContextContainer nfsContextContainer {};
    shared_ptr<NfsContextWrapper> nfs = nfsContextContainer.GetCurrContext();

    MOCKER_CPP(&NfsContextWrapper::NfsLstat64Lock)
            .stubs()
            .will(returnValue(MP_SUCCESS));
    MOCKER_CPP(&Libnfscommonmethods::DeleteFileDirectoryLibNfsRecursively)
            .stubs()
            .will(returnValue(MP_SUCCESS));
    EXPECT_EQ(DeleteDirectory(deleteInfo, nfs), MP_SUCCESS);

    deleteInfo.m_fileName = "";
    EXPECT_EQ(DeleteDirectory(deleteInfo, nfs), MP_SUCCESS);
}

TEST_F(LibnfsCommonMethodsTest, DeleteFileDirectoryLibNfsRecursively)
{
    string filePath = "/d1/file.txt";
    int isDir = 1;
    NfsContextContainer nfsContextContainer {};
    shared_ptr<NfsContextWrapper> dstNfs = nfsContextContainer.GetCurrContext();

    MOCKER_CPP(&Libnfscommonmethods::DeleteAllFilesInsideRecursively)
            .stubs()
            .will(returnValue(MP_SUCCESS));
    EXPECT_EQ(DeleteFileDirectoryLibNfsRecursively(filePath, isDir, dstNfs), MP_SUCCESS);

    isDir = 0;
    MOCKER_CPP(&NfsContextWrapper::NfsUnlinkLock)
            .stubs()
            .will(returnValue(MP_SUCCESS))
            .then(invoke(NfsUnlinkLock_Stub))
            .then(returnValue(MP_FAILED));
    EXPECT_EQ(DeleteFileDirectoryLibNfsRecursively(filePath, isDir, dstNfs), MP_SUCCESS);

    EXPECT_EQ(DeleteFileDirectoryLibNfsRecursively(filePath, isDir, dstNfs), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::GetNfsContext)
            .stubs()
            .will(invoke(GetNfsContext_Stub));
    EXPECT_EQ(DeleteFileDirectoryLibNfsRecursively(filePath, isDir, dstNfs), MP_FAILED);
}

TEST_F(LibnfsCommonMethodsTest, RemoveHardLinkMapEntryIfFileCreationFailed)
{
    HardLinkMapRemoveParams hardLinkMapRemoveParams {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_dirName = "d1";
    hardLinkMapRemoveParams.hardLinkMap = make_shared<HardLinkMap>();
    hardLinkMapRemoveParams.fileHandle = fileHandle;

    MOCKER_CPP(&HardLinkMap::Exist)
            .stubs()
            .will(returnValue(true));
    RemoveHardLinkMapEntryIfFileCreationFailed(hardLinkMapRemoveParams);
}

TEST_F(LibnfsCommonMethodsTest, ConstructReadBlock)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "/1.txt";
    fileHandle.m_file->m_size = 1024;
    uint64_t blockSize = 1024;
    ConstructReadBlock(fileHandle, blockSize);
}

TEST_F(LibnfsCommonMethodsTest, FloatToString)
{
    float val = 0.1;
    uint8_t precisson = 0;
    FloatToString(val, precisson);
}

TEST_F(LibnfsCommonMethodsTest, FormatCapacity)
{
    uint64_t capacity = 256;
    FormatCapacity(capacity);

    capacity = 2 * 1024;
    FormatCapacity(capacity);

    capacity = 2 * 1024 * 1024;
    FormatCapacity(capacity);

    capacity = 2 * 1024 * 1024 * 1024;
    FormatCapacity(capacity);

    capacity = 2 * 1024 * 1024 * 1024 * 1024;
    FormatCapacity(capacity);

    capacity = 2 * 1024 * 1024 * 1024 * 1024 * 1024;
    FormatCapacity(capacity);
}

TEST_F(LibnfsCommonMethodsTest, ProcessDirParentFh)
{
    std::string dirPath = "d2";
    std::string parentDirPath = "d1";
    uint16_t retryCnt = 0;

    struct nfsfh* nfsfh = nullptr;
    EXPECT_EQ(ProcessDirParentFh(dirPath, parentDirPath, nfsfh, retryCnt), MP_SUCCESS);

    nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&Libnfscommonmethods::IsValidNfsFh)
            .stubs()
            .will(returnValue(1));
    EXPECT_EQ(ProcessDirParentFh(dirPath, parentDirPath, nfsfh, retryCnt), MP_FAILED);
}

TEST_F(LibnfsCommonMethodsTest, ProcessParentFh)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->ClearFlag(IS_DIR);

    struct nfsfh* nfsfh = nullptr;
    fileHandle.m_file->m_dirName = "d2";
    EXPECT_EQ(ProcessParentFh(fileHandle, commonData, nfsfh), MP_FAILED);

    nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&Libnfscommonmethods::IsValidNfsFh)
            .stubs()
            .will(returnValue(1));
    MOCKER_CPP(&Libnfscommonmethods::HandleParentDirCreationFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(ProcessParentFh(fileHandle, commonData, nfsfh), MP_FAILED);
}

TEST_F(LibnfsCommonMethodsTest, HandleParentDirCreationFailure)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    MOCKER_CPP(&Libnfscommonmethods::RemoveHardLinkMapEntryIfFileCreationFailed)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    HandleParentDirCreationFailure(fileHandle, commonData);
}

TEST_F(LibnfsCommonMethodsTest, IsAbort)
{
    *(commonData.abort) = false;
    commonData.controlInfo->m_failed = false;
    commonData.controlInfo->m_controlReaderFailed = true;
    bool ret = IsAbort(commonData);
    EXPECT_EQ(ret, true);

    *(commonData.abort) = false;
    commonData.controlInfo->m_failed = false;
    commonData.controlInfo->m_controlReaderFailed = false;
    ret = IsAbort(commonData);
    EXPECT_EQ(ret, false);
}