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
#include "mockcpp/mockcpp.hpp"
#include "common/FSBackupUtils.h"
#include "MkdirRequest.h"
#include "LibnfsCopyWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class MkdirRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
};

void MkdirRequestTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_commonData.nfsContextContainer = &m_nfsContextContainer;
    m_commonData.syncNfsContextContainer = &m_nfsContextContainer;
    m_commonData.pktStats = make_shared<PacketStats>();
    m_commonData.writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_commonData.controlInfo = std::make_shared<BackupControlInfo>();
    m_commonData.commonObj = nullptr;
    m_commonData.IsResumeSendCb = LibnfsCopyWriter::IsResumeSendCb;
    m_commonData.ResumeSendCb = LibnfsCopyWriter::ResumeSendCb;
    m_commonData.hardlinkMap = make_shared<HardLinkMap>();
}

void MkdirRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void MkdirRequestTest::SetUpTestCase()
{}

void MkdirRequestTest::TearDownTestCase()
{}

static nfs_context* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfsContext;
}

static std::string String_Stub()
{
    std::string str = "";
    return str;
}
TEST_F(MkdirRequestTest, CreateCloseCbData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    NfsCommonData commonData {};
    int retryCnt = 0;
    std::shared_ptr<FileHandleCache> fileHandleCache {};
    struct nfsfh* nfsfh {};
    CreateMkdirCbData(fileHandle, commonData, retryCnt, fileHandleCache, nfsfh);
}

TEST_F(MkdirRequestTest, SendClose)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_dirName = "/d1";

    NfsMkdirCbData *cbData = nullptr;
    EXPECT_EQ(SendMkdir(fileHandle, cbData), MP_FAILED);

    cbData = new(nothrow) NfsMkdirCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &m_commonData;
    cbData->nfsfh = nullptr;

    MOCKER_CPP(&IsRootDir)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&CreateDirWithPath)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&HandleMkdirSyncReqStatus)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0))
            .then(returnValue(0));
    EXPECT_EQ(SendMkdir(fileHandle, cbData), MP_SUCCESS);

    MOCKER_CPP(&CreateDirWithPath)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&HandleParentDirNotPresent)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendMkdir(fileHandle, cbData), MP_SUCCESS);

    cbData->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&CreateDirWithFh)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendMkdir(fileHandle, cbData), MP_SUCCESS);
}

TEST_F(MkdirRequestTest, CreateDirWithPath)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    std::shared_ptr<NfsContextWrapper> nfs = m_commonData.syncNfsContextContainer->GetCurrContext();
    NfsMkdirCbData *cbData = nullptr;

    EXPECT_EQ(CreateDirWithPath(fileHandle, nfs, cbData), MP_FAILED);

    cbData = new(nothrow) NfsMkdirCbData();
    cbData->writeCommonData = &m_commonData;
    MOCKER_CPP(&NfsContextWrapper::NfsMkdirGetFhLock)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(CreateDirWithPath(fileHandle, nfs, cbData), MP_SUCCESS);

}

TEST_F(MkdirRequestTest, CreateDirWithFh)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    struct nfsfh* nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    std::shared_ptr<NfsContextWrapper> nfs = m_commonData.syncNfsContextContainer->GetCurrContext();
    NfsMkdirCbData *cbData = nullptr;

    EXPECT_EQ(CreateDirWithFh(fileHandle, nfsfh, nfs, cbData), MP_FAILED);

    cbData = new(nothrow) NfsMkdirCbData();
    cbData->writeCommonData = &m_commonData;
    MOCKER_CPP(&NfsContextWrapper::NfsMkdirGetFhWithParentFhLock)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(CreateDirWithFh(fileHandle, nfsfh, nfs, cbData), MP_SUCCESS);

}

TEST_F(MkdirRequestTest, HandleParentDirNotPresent)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_dirName = "d1";
    uint16_t retryCnt = 0;
    std::shared_ptr<NfsContextWrapper> nfs = m_commonData.syncNfsContextContainer->GetCurrContext();
    NfsMkdirCbData *cbData = nullptr;

    EXPECT_EQ(HandleParentDirNotPresent(fileHandle, retryCnt, nfs, cbData), MP_FAILED);

    NfsMkdirCbData *cbData1 = new(nothrow) NfsMkdirCbData();
    cbData1->writeCommonData = &m_commonData;
    cbData1->fileHandleCache = make_shared<FileHandleCache>();
    MOCKER_CPP(&FSBackupUtils::GetParentDir)
            .stubs()
            .will(invoke(String_Stub));
    MOCKER_CPP(&MakeDirRecursively)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(0))
            .then(returnValue(0));
    EXPECT_EQ(HandleParentDirNotPresent(fileHandle, retryCnt, nfs, cbData1), MP_FAILED);
    EXPECT_EQ(HandleParentDirNotPresent(fileHandle, retryCnt, nfs, cbData1), MP_FAILED);

    struct nfsfh* nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData1->fileHandleCache->Push(fileHandle.m_file->m_dirName, nfsfh);
    MOCKER_CPP(&CreateDirWithFh)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(HandleParentDirNotPresent(fileHandle, retryCnt, nfs, cbData1), MP_SUCCESS);
}

TEST_F(MkdirRequestTest, HandleMkdirSyncReqStatus)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_dirName = "d1";
    uint16_t retryCnt = 0;
    int status = 0;
    std::shared_ptr<NfsContextWrapper> nfs = m_commonData.syncNfsContextContainer->GetCurrContext();
    NfsMkdirCbData *cbData = nullptr;

    EXPECT_EQ(HandleMkdirSyncReqStatus(status, fileHandle, retryCnt, nfs, cbData), MP_FAILED);

    NfsMkdirCbData *cbData1 = new(nothrow) NfsMkdirCbData();
    cbData1->writeCommonData = &m_commonData;
    cbData1->fileHandleCache = make_shared<FileHandleCache>();
    status = -BACKUP_ERR_EEXIST;
    MOCKER_CPP(&HandleDirExist)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(HandleMkdirSyncReqStatus(status, fileHandle, retryCnt, nfs, cbData1), MP_SUCCESS);
    EXPECT_EQ(HandleMkdirSyncReqStatus(status, fileHandle, retryCnt, nfs, cbData1), MP_FAILED);

    status = -EACCES;
    MOCKER_CPP(&IsCriticalError)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    MOCKER_CPP(&HandleDstNoSpaceAndNoAccessError)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(HandleMkdirSyncReqStatus(status, fileHandle, retryCnt, nfs, cbData1), MP_FAILED);

    status = -EINTR;
    MOCKER_CPP(&NfsContextWrapper::GetNfsContext)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(HandleMkdirSyncReqStatus(status, fileHandle, retryCnt, nfs, cbData1), MP_FAILED);

    status = -1;
    MOCKER_CPP(&NfsContextWrapper::GetNfsContext)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(HandleMkdirSyncReqStatus(status, fileHandle, retryCnt, nfs, cbData1), MP_FAILED);

    status = 0;
    fileHandle.m_file->dstIOHandle.nfsFh = nullptr;
    EXPECT_EQ(HandleMkdirSyncReqStatus(status, fileHandle, retryCnt, nfs, cbData1), MP_SUCCESS);

    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData1->fileHandleCache->Push(fileHandle.m_file->m_dirName, fileHandle.m_file->dstIOHandle.nfsFh);
    MOCKER_CPP(&FileHandleCache::Push)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&Libnfscommonmethods::FreeDirFh)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(HandleMkdirSyncReqStatus(status, fileHandle, retryCnt, nfs, cbData1), MP_SUCCESS);
}

TEST_F(MkdirRequestTest, HandleDirExist)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_dirName = "d1";
    uint16_t retryCnt = 0;
    std::shared_ptr<NfsContextWrapper> nfs = m_commonData.syncNfsContextContainer->GetCurrContext();
    NfsMkdirCbData *cbData = nullptr;

    EXPECT_EQ(HandleDirExist(fileHandle, retryCnt, nfs, cbData), MP_FAILED);

    NfsMkdirCbData *cbData1 = new(nothrow) NfsMkdirCbData();
    cbData1->writeCommonData = &m_commonData;
    cbData1->fileHandleCache = make_shared<FileHandleCache>();
    MOCKER_CPP(&NfsContextWrapper::NfsLstat64Lock)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&HandleFileExistWithSameNameAsDirectory)
            .stubs()
            .will(returnValue(1));
    EXPECT_EQ(HandleDirExist(fileHandle, retryCnt, nfs, cbData1), MP_FAILED);

    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData1->fileHandleCache->Push(fileHandle.m_file->m_dirName, fileHandle.m_file->dstIOHandle.nfsFh);
    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&Libnfscommonmethods::IsValidNfsFh)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&NfsContextWrapper::NfsLstat64WithParentFhLock)
            .stubs()
            .will(returnValue(1));
    MOCKER_CPP(&FileHandleCache::Push)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&Libnfscommonmethods::FreeDirFh)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(HandleDirExist(fileHandle, retryCnt, nfs, cbData1), MP_SUCCESS);

    EXPECT_EQ(HandleDirExist(fileHandle, retryCnt, nfs, cbData1), MP_FAILED);
}

TEST_F(MkdirRequestTest, IsRootDir)
{
    std::string path {};
    EXPECT_EQ(IsRootDir(path), true);

    path = "d1";
    EXPECT_EQ(IsRootDir(path), false);
}

TEST_F(MkdirRequestTest, IsCriticalError)
{
    int status = -EACCES;
    EXPECT_EQ(IsCriticalError(status), true);

    status = -1;
    EXPECT_EQ(IsCriticalError(status), false);
}
