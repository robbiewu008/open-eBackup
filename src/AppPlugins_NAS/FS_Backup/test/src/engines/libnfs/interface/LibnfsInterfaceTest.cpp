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
#include "LibnfsInterface.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}
class LibnfsInterfaceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    FileHandle m_fileHandle {};
    NfsCommonData m_commonData {};
    std::shared_ptr<BlockBufferMap> m_blockBufferMap { nullptr };
    std::shared_ptr<FileHandleCache> m_fileHandleCache { nullptr };
};

void LibnfsInterfaceTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;
    m_backupParams.commonParams.skipFailure = false;

    m_fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    m_fileHandle.m_file->SetSrcState(FileDescState::INIT);
    m_fileHandle.m_file->SetDstState(FileDescState::INIT);
    m_blockBufferMap = std::make_shared<BlockBufferMap>();
    m_commonData.controlInfo = std::make_shared<BackupControlInfo>();

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_commonData.aggregateQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_commonData.writeWaitQueue = std::make_shared<BackupQueue<FileHandle>>(config);

    m_fileHandleCache = make_shared<FileHandleCache>();
}

void LibnfsInterfaceTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibnfsInterfaceTest::SetUpTestCase()
{}

void LibnfsInterfaceTest::TearDownTestCase()
{}


static nfsfh* Get_Stub1()
{
    struct nfsfh *nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    return nfsFh;
}

TEST_F(LibnfsInterfaceTest, SendNfsRequest)
{
    FileHandle fileHandle {};
    void *cbData = nullptr;
    LibnfsEvent event = LibnfsEvent::CREATE;
    MOCKER_CPP(&SendNfsRequest1)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest(fileHandle, cbData, event), MP_SUCCESS);

    event = LibnfsEvent::MKDIR;
    MOCKER_CPP(&SendNfsRequest2)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest(fileHandle, cbData, event), MP_SUCCESS);
}

TEST_F(LibnfsInterfaceTest, SendNfsRequest_1)
{
    FileHandle fileHandle {};
    void *cbData = nullptr;
    LibnfsEvent event = LibnfsEvent::OPEN;
    MOCKER_CPP(&SendOpen)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::READ;
    MOCKER_CPP(&SendRead)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::READLINK;
    MOCKER_CPP(&SendReadlink)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::SRC_CLOSE;
    MOCKER_CPP(&SendCloseSync)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::LSTAT;
    MOCKER_CPP(&SendLstat)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);
}

TEST_F(LibnfsInterfaceTest, SendNfsRequests1)
{
    FileHandle fileHandle {};
    void *cbData = nullptr;
    LibnfsEvent event = LibnfsEvent::CREATE;
    MOCKER_CPP(&SendCreate)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::WRITE;
    MOCKER_CPP(&SendWrite)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);
    //event = LibnfsEvent::WRITE_META;
    //MOCKER_CPP(&SendSetMeta)
    //        .stubs()
    //        .will(returnValue(0));
    //EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::MKNOD;
    MOCKER_CPP(&SendMknod)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::SYMLINK;
    MOCKER_CPP(&SendSymLink)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);

    event = LibnfsEvent::INVALID;
    EXPECT_EQ(SendNfsRequest1(fileHandle, cbData, event), MP_SUCCESS);
}

TEST_F(LibnfsInterfaceTest, SendNfsRequest_2)
{
    FileHandle fileHandle {};
    void *cbData = nullptr;
    LibnfsEvent event = LibnfsEvent::LINK_UTIME;
    MOCKER_CPP(&SendLinkUtime)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest2(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::HARDLINK;
    MOCKER_CPP(&SendHardLink)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest2(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::MKDIR;
    MOCKER_CPP(&SendMkdir)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest2(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::DIR_DELETE;
    MOCKER_CPP(&SendDirDelete)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest2(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::LINK_DELETE;
    MOCKER_CPP(&SendLinkDeleteSync)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest2(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::LINK_DELETE_FOR_RESTORE;
    MOCKER_CPP(&SendLinkDelete)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest2(fileHandle, cbData, event), MP_SUCCESS);
    event = LibnfsEvent::DST_CLOSE;
    MOCKER_CPP(&SendClose)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendNfsRequest2(fileHandle, cbData, event), MP_SUCCESS);

    event = LibnfsEvent::INVALID;
    EXPECT_EQ(SendNfsRequest2(fileHandle, cbData, event), MP_SUCCESS);
}

TEST_F(LibnfsInterfaceTest, SendOpenRequest)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendReaderNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendOpenRequest(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(SendOpenRequest(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendReadRequest)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendReaderNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendReadRequest(m_fileHandle, m_commonData, m_blockBufferMap), MP_SUCCESS);
    EXPECT_EQ(SendReadRequest(m_fileHandle, m_commonData, m_blockBufferMap), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendReadLinkRequest)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendReaderNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendReadLinkRequest(m_fileHandle, m_commonData, m_blockBufferMap), MP_SUCCESS);
    EXPECT_EQ(SendReadLinkRequest(m_fileHandle, m_commonData, m_blockBufferMap), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendSrcCloseRequest)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendReaderNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendSrcCloseRequest(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(SendSrcCloseRequest(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendCreateRequest)
{
    struct nfsfh *nfsfh = nullptr;
    uint32_t openFlag = 0;

    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendCreateRequest(m_fileHandle, nfsfh, openFlag, m_commonData, m_backupParams), MP_SUCCESS);
    EXPECT_EQ(SendCreateRequest(m_fileHandle, nfsfh, openFlag, m_commonData, m_backupParams), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendWriteRequest)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendWriteRequest(m_fileHandle, m_commonData, m_blockBufferMap), MP_SUCCESS);
    EXPECT_EQ(SendWriteRequest(m_fileHandle, m_commonData, m_blockBufferMap), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendSetMetaRequest)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendSetMetaRequest(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(SendSetMetaRequest(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendSymlinkUtimeRequest)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendSymlinkUtimeRequest(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(SendSymlinkUtimeRequest(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendLstatRequest)
{
    struct nfsfh *nfsfh = nullptr;

    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendLstatRequest(m_fileHandle, nfsfh, m_commonData, m_backupParams), MP_SUCCESS);
    EXPECT_EQ(SendLstatRequest(m_fileHandle, nfsfh, m_commonData, m_backupParams), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendSymlinkRequest)
{
    struct nfsfh *nfsfh = nullptr;

    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendSymlinkRequest(m_fileHandle, nfsfh, m_commonData, m_blockBufferMap), MP_SUCCESS);
    EXPECT_EQ(SendSymlinkRequest(m_fileHandle, nfsfh, m_commonData, m_blockBufferMap), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendMknodRequest)
{
    struct nfsfh *nfsfh = nullptr;

    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendMknodRequest(m_fileHandle, nfsfh, m_commonData), MP_SUCCESS);
    EXPECT_EQ(SendMknodRequest(m_fileHandle, nfsfh, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendDstCloseRequest)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendDstCloseRequest(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(SendDstCloseRequest(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendLinkDeleteRequest)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendLinkDeleteRequest(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(SendLinkDeleteRequest(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendLinkDeleteRequestForRestore)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendLinkDeleteRequestForRestore(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(SendLinkDeleteRequestForRestore(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendDirDeleteRequest)
{
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendDirDeleteRequest(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(SendDirDeleteRequest(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, SendHardlinkRequest)
{
    struct nfsfh *nfsfh = nullptr;
    string targetPath {};

    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleSendWriterNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendHardlinkRequest(m_fileHandle, targetPath, nfsfh, m_commonData, m_backupParams), MP_SUCCESS);
    EXPECT_EQ(SendHardlinkRequest(m_fileHandle, targetPath, nfsfh, m_commonData, m_backupParams), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, ReadLink)
{
    MOCKER_CPP(&SendReadLinkRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(ReadLink(m_fileHandle, m_commonData, m_blockBufferMap), MP_SUCCESS);
    EXPECT_EQ(ReadLink(m_fileHandle, m_commonData, m_blockBufferMap), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, WriteSymLinkMeta)
{
    MOCKER_CPP(&SendSymlinkUtimeRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(WriteSymLinkMeta(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(WriteSymLinkMeta(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, LstatFile)
{
    m_fileHandle.m_file->m_dirName = "dir1";

    MOCKER_CPP(&FileHandleCache::Get)
            .stubs()
            .will(invoke(Get_Stub1))
            .then(invoke(Get_Stub1));
    MOCKER_CPP(&Libnfscommonmethods::ProcessParentFh)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&SendLstatRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(LstatFile(m_fileHandle, m_commonData, m_backupParams, m_fileHandleCache), MP_SUCCESS);
    EXPECT_EQ(LstatFile(m_fileHandle, m_commonData, m_backupParams, m_fileHandleCache), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, CreateSymlink)
{
    m_fileHandle.m_file->m_dirName = "d2";
    MOCKER_CPP(&FileHandleCache::Get)
            .stubs()
            .will(invoke(Get_Stub1))
            .then(invoke(Get_Stub1));
    MOCKER_CPP(&Libnfscommonmethods::ProcessParentFh)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&SendSymlinkRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(CreateSymlink(m_fileHandle, m_commonData, m_fileHandleCache,
        m_blockBufferMap), MP_SUCCESS);
    EXPECT_EQ(CreateSymlink(m_fileHandle, m_commonData, m_fileHandleCache,
        m_blockBufferMap), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, CreateSpecialFile)
{
    m_fileHandle.m_file->m_dirName = "d2";
    MOCKER_CPP(&FileHandleCache::Get)
            .stubs()
            .will(invoke(Get_Stub1))
            .then(invoke(Get_Stub1));
    MOCKER_CPP(&Libnfscommonmethods::ProcessParentFh)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&SendMknodRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(CreateSpecialFile(m_fileHandle, m_commonData, m_fileHandleCache), MP_SUCCESS);
    EXPECT_EQ(CreateSpecialFile(m_fileHandle, m_commonData, m_fileHandleCache), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, LinkDelete)
{
    m_fileHandle.m_block.m_size = 5;

    m_fileHandle.m_file->SetDstState(FileDescState::LINK_DEL_FAILED);
    EXPECT_EQ(LinkDelete(m_fileHandle, m_commonData, m_blockBufferMap), MP_SUCCESS);

    m_fileHandle.m_file->SetDstState(FileDescState::INIT);
    EXPECT_EQ(LinkDelete(m_fileHandle, m_commonData, m_blockBufferMap), MP_SUCCESS);

    m_fileHandle.m_block.m_size = 0;
    m_fileHandle.m_block.m_seq = 0;
    MOCKER_CPP(&SendLinkDeleteRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(LinkDelete(m_fileHandle, m_commonData, m_blockBufferMap), MP_SUCCESS);
    EXPECT_EQ(LinkDelete(m_fileHandle, m_commonData, m_blockBufferMap), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, LinkDeleteForRestore)
{
    m_fileHandle.m_block.m_size = 5;

    EXPECT_EQ(LinkDeleteForRestore(m_fileHandle, m_commonData), MP_SUCCESS);

    m_fileHandle.m_block.m_size = 0;
    m_fileHandle.m_block.m_seq = 0;
    MOCKER_CPP(&SendLinkDeleteRequestForRestore)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(LinkDeleteForRestore(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(LinkDeleteForRestore(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, DirectoryDelete)
{
    m_fileHandle.m_block.m_size = 5;

    EXPECT_EQ(DirectoryDelete(m_fileHandle, m_commonData), MP_SUCCESS);

    m_fileHandle.m_block.m_size = 0;
    m_fileHandle.m_block.m_seq = 0;
    MOCKER_CPP(&SendDirDeleteRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(DirectoryDelete(m_fileHandle, m_commonData), MP_SUCCESS);
    EXPECT_EQ(DirectoryDelete(m_fileHandle, m_commonData), MP_FAILED);
}

TEST_F(LibnfsInterfaceTest, MakeDirectory)
{
    m_fileHandle.m_file->m_dirName = "d1";
    m_backupParams.commonParams.skipFailure = false;

    /* MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsCopyWriter->MakeDirectory(fileHandle), MP_SUCCESS);

    MOCKER_CPP(&FileHandleCache::Get)
            .stubs()
            .will(invoke(Get_Stub1))
            .then(invoke(Get_Stub1));
    MOCKER_CPP(&Libnfscommonmethods::IsValidNfsFh)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->MakeDirectory(fileHandle), MP_SUCCESS);

    MOCKER_CPP(&Libnfscommonmethods::FillFileHandleCacheWithInvalidDirectoryFh)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    EXPECT_EQ(m_libnfsCopyWriter->MakeDirectory(fileHandle), MP_SUCCESS);

    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->MakeDirectory(fileHandle), MP_SUCCESS); */
}

TEST_F(LibnfsInterfaceTest, HandleZeroSizeFileRead)
{
    m_backupParams.commonParams.skipFailure = false;
    MOCKER_CPP(&PushToAggregator)
            .stubs()
            .will(ignoreReturnValue());
    HandleZeroSizeFileRead(m_fileHandle, m_commonData, m_blockBufferMap);

    m_backupParams.commonParams.skipFailure = false;
    HandleZeroSizeFileRead(m_fileHandle, m_commonData, m_blockBufferMap);
}

TEST_F(LibnfsInterfaceTest, HandleSendReaderNfsRequestFailure)
{
    HandleSendReaderNfsRequestFailure(m_fileHandle, m_commonData);
}

TEST_F(LibnfsInterfaceTest, HandleSendWriterNfsRequestFailure)
{
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(returnValue(FileDescState::INIT));

    m_backupParams.commonParams.skipFailure = false;
    HandleSendWriterNfsRequestFailure(m_fileHandle, m_commonData);
}

TEST_F(LibnfsInterfaceTest, PushToAggregator)
{
    m_backupParams.commonParams.writeDisable = false;
    PushToAggregator(m_fileHandle, m_commonData);
}