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
#include <stdio.h>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include "config_reader/ConfigIniReader.h"
#include "common/FSBackupUtils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "llt_stub/addr_pri.h"
#include "libsmb_ctx/SmbContextWrapper.h"
#include "interface/LibsmbWriterSyncInterface.h"

using namespace std;
using namespace Module;
namespace {
    const std::string MOCK_SMB_ERR = "MOCK_SMB_ERR";
}

class LibsmbWriterSyncInterfaceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    BackupQueueConfig config;
};

void LibsmbWriterSyncInterfaceTest::SetUp()
{
    fileHandle.m_file = fileDesc;
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    fileHandle.m_file->m_dirName = "/opt/oss";
    fileHandle.m_file->m_fileAttr = 1;
    fileHandle.m_file->m_btime = 1;
    fileHandle.m_file->m_atime = 1;
    fileHandle.m_file->m_mtime = 1;
    fileHandle.m_file->m_ctime = 1;
    fileHandle.m_file->SetFlag(IS_DIR);
    fileHandle.m_block.m_buffer = {};
    fileHandle.m_file->dstIOHandle.smbFh = {};
    fileHandle.m_file->m_blockStats.m_readReqCnt = 0;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    fileHandle.m_file->m_blockStats.m_writeReqCnt = 1;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_size = 0;
    fileHandle.m_retryCnt = 1;
    fileHandle.m_file->m_aclText = "test";

    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
}

void LibsmbWriterSyncInterfaceTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibsmbWriterSyncInterfaceTest::SetUpTestCase()
{}

void LibsmbWriterSyncInterfaceTest::TearDownTestCase()
{}

/*
 * 用例名称：SendWriterSyncRequest_MKDIR
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, SendWriterSyncRequest_MKDIR) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo = controlInfo;
    cbData->writeQueue = make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(SmbMkdirSync)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendWriterSyncRequest(fileHandle, cbData, LibsmbEvent::MKDIR), 0);
}

/*
 * 用例名称：SendWriterSyncRequest_SucDELETE
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, SendWriterSyncRequest_SucDELETE) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo = controlInfo;
    cbData->controlInfo->m_noOfDirDeleted = 0;
    cbData->writeQueue = make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(DeleteAllFilesInside)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendWriterSyncRequest(fileHandle, cbData, LibsmbEvent::DELETE), 0);
}

/*
 * 用例名称：SendWriterSyncRequest_NeedRetryDELETE
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, SendWriterSyncRequest_NeedRetryDELETE) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo = controlInfo;
    cbData->controlInfo->m_noOfDirDeleted = 0;
    cbData->writeQueue = make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(DeleteAllFilesInside)
            .stubs()
            .will(returnValue(-1));
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(SmbEnqueueToTimer)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendWriterSyncRequest(fileHandle, cbData, LibsmbEvent::DELETE), 0);
}

/*
 * 用例名称：SendWriterSyncRequest_Default
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, SendWriterSyncRequest_Default) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;

    EXPECT_EQ(SendWriterSyncRequest(fileHandle, cbData, LibsmbEvent::READ), 0);
}

/*
 * 用例名称：SmbMkdirSync
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, SmbMkdirSync) {
    auto mkdirParams = new(nothrow) SmbWriterCommonData();
    if (mkdirParams == nullptr) {
        return;
    }
    mkdirParams->pktStats = make_shared<PacketStats>();
    mkdirParams->fileHandle = fileHandle;
    mkdirParams->path = "/opt/oss";
    mkdirParams->params.dstRootPath = "/root";
    MOCKER_CPP(&Module::SmbContextWrapper::SmbMkdir)
            .stubs()
            .will(returnValue(-2))
            .then(returnValue(-12))
            .then(returnValue(0));
    MOCKER_CPP(MakeDirRecursively)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    MOCKER_CPP(HandleMkdirSyncReqStatus)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SmbMkdirSync(1, mkdirParams), -1);
    EXPECT_EQ(SmbMkdirSync(1, mkdirParams), 0);
    EXPECT_EQ(SmbMkdirSync(1, mkdirParams), 0);
    delete mkdirParams;
}

static string smbGetError_Stub()
{
    return "error";
}

/*
 * 用例名称：HandleMkdirSyncReqStatus
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, HandleMkdirSyncReqStatus) {
    auto mkdirParams = new(nothrow) SmbWriterCommonData();
    if (mkdirParams == nullptr) {
        return;
    }
    mkdirParams->pktStats = make_shared<PacketStats>();
    mkdirParams->controlInfo = make_shared<BackupControlInfo>();
    mkdirParams->fileHandle = fileHandle;
    mkdirParams->path = "/opt/oss";
    MOCKER_CPP(HandleDirExist)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(HandleConnectionException)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(CheckStatusAndIncStat)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
            .stubs()
            .will(invoke(smbGetError_Stub));
    EXPECT_EQ(HandleMkdirSyncReqStatus(0, mkdirParams->path, 1, mkdirParams), 0);
    EXPECT_EQ(HandleMkdirSyncReqStatus(-17, mkdirParams->path, 1, mkdirParams), -1);
    EXPECT_EQ(HandleMkdirSyncReqStatus(-17, mkdirParams->path, 1, mkdirParams), 0);
    EXPECT_EQ(HandleMkdirSyncReqStatus(-10, mkdirParams->path, 1, mkdirParams), -1);
    EXPECT_EQ(HandleMkdirSyncReqStatus(-11, mkdirParams->path, 1, mkdirParams), -1);
    delete mkdirParams;
}

/*
 * 用例名称：MakeDirRecursively
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, MakeDirRecursively) {
    auto mkdirParams = new(nothrow) SmbWriterCommonData();
    if (mkdirParams == nullptr) {
        return;
    }
    std::string targetFilePath = "";
    EXPECT_EQ(MakeDirRecursively(targetFilePath, mkdirParams), -1);
    targetFilePath = "/root/etc/";
    MOCKER_CPP(StatAndMkDir)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(MakeDirRecursively(targetFilePath, mkdirParams), 0);
    delete mkdirParams;
}

/*
 * 用例名称：StatAndMkDir
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, StatAndMkDir) {
    auto mkdirParams = new(nothrow) SmbWriterCommonData();
    if (mkdirParams == nullptr) {
        return;
    }
    std::string parentDirPath = "/opt/oss";
    std::string dirPath = "/root";
    MOCKER_CPP(&Module::SmbContextWrapper::SmbMkdir)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(HandleMkdirSyncReqStatus)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(StatAndMkDir(dirPath, parentDirPath, mkdirParams), 0);
    delete mkdirParams;
}

static int SmbStat64_stub(const char *path, struct smb2_stat_64 *st)
{
    st->smb2_type = SMB2_TYPE_DIRECTORY;
    return 0;
}

/*
 * 用例名称：HandleDirExist
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, HandleDirExist) {
    auto mkdirParams = new(nothrow) SmbWriterCommonData();
    if (mkdirParams == nullptr) {
        return;
    }
    mkdirParams->path = "/root";
    std::string curPath = "/opt/oss";
    mkdirParams->params.dstRootPath = "/oss";
    MOCKER_CPP(&Module::SmbContextWrapper::SmbStat64)
            .stubs()
            .will(invoke(SmbStat64_stub))
            .then(returnValue(0));
    MOCKER_CPP(&Module::SmbContextWrapper::SmbUnlink)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
            .stubs()
            .will(invoke(smbGetError_Stub));
    MOCKER_CPP(&Module::SmbContextWrapper::SmbMkdir)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(HandleDirExist(curPath, 1, mkdirParams), 0);
    EXPECT_EQ(HandleDirExist(curPath, 1, mkdirParams), 0);
    delete mkdirParams;
}

/*
 * 用例名称：HandleDirExist
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, SmbUnlink) {
    auto linkDeleteParams = new(nothrow) SmbWriterCommonData();
    if (linkDeleteParams == nullptr) {
        return;
    }
    linkDeleteParams->fileHandle = fileHandle;
    linkDeleteParams->path = "/root";
    std::string curPath = "/opt/oss";
    linkDeleteParams->params.dstRootPath = "/oss";
    MOCKER_CPP(&Module::SmbContextWrapper::SmbUnlink)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-2))
            .then(returnValue(-21))
            .then(returnValue(-22));
    MOCKER_CPP(DeleteAllFilesInside)
            .stubs()
            .will(returnValue(-1));
    MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
            .stubs()
            .will(invoke(smbGetError_Stub));
    EXPECT_EQ(SmbUnlink(linkDeleteParams), 0);
    EXPECT_EQ(SmbUnlink(linkDeleteParams), 0);
    EXPECT_EQ(SmbUnlink(linkDeleteParams), -1);
    EXPECT_EQ(SmbUnlink(linkDeleteParams), -1);
    delete linkDeleteParams;
}

/*
 * 用例名称：SmbRemoveFileAndDirRecursive
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterSyncInterfaceTest, SmbRemoveFileAndDirRecursive) {
    SmbMkdirParams rmdirParams = {};
    MOCKER_CPP(&Module::SmbContextWrapper::SmbStat64)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    MOCKER_CPP(ConcatRootPath)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SmbRemoveFileAndDirRecursive(fileHandle, rmdirParams), 0);
    EXPECT_EQ(SmbRemoveFileAndDirRecursive(fileHandle, rmdirParams), -1);

    fileHandle.m_file->m_fileName = "";
    EXPECT_EQ(SmbRemoveFileAndDirRecursive(fileHandle, rmdirParams), -1);
}
