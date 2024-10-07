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
#include "interface/LibsmbReaderInterface.h"
#include "libsmb_ctx/SmbContextWrapper.h"

using namespace std;
using namespace Module;
namespace  {
    const std::string MOCK_SMB_ERR = "MOCK_SMB_ERR";
}

class LibsmbReaderInterfaceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    BackupQueueConfig config;

};

void LibsmbReaderInterfaceTest::SetUp()
{
    fileHandle.m_file = fileDesc;
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    fileHandle.m_file->m_dirName = "/opt/oss";
    fileHandle.m_file->m_blockStats.m_readReqCnt = 0;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    fileHandle.m_file->m_size = 0;
    fileHandle.m_retryCnt = 1;

    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
}

void LibsmbReaderInterfaceTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibsmbReaderInterfaceTest::SetUpTestCase()
{}

void LibsmbReaderInterfaceTest::TearDownTestCase()
{}

/*
 * 用例名称：SendReaderRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, SendReaderRequest) {
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();

    MOCKER_CPP(SendOpenRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendReaderRequest(fileHandle, cbData, LibsmbEvent::OPEN_SRC), 0);

    MOCKER_CPP(SendReadRequest)
            .stubs()
            .will(returnValue(-1));
    EXPECT_EQ(SendReaderRequest(fileHandle, cbData, LibsmbEvent::READ), -1);

    MOCKER_CPP(SendCloseRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendReaderRequest(fileHandle, cbData, LibsmbEvent::CLOSE_SRC), 0);

    MOCKER_CPP(SendAdsRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendReaderRequest(fileHandle, cbData, LibsmbEvent::ADS), 0);

    EXPECT_EQ(SendReaderRequest(fileHandle, cbData, LibsmbEvent::DELETE), 0);
}

/*
 * 用例名称：SendOpenRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, SendOpenRequest) {
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->params.srcRootPath = "/opt/oss";
    MOCKER_CPP(&Module::SmbContextWrapper::SmbOpenAsync)
        .stubs()
        .will(returnValue(0))
        .then(returnValue(-1));
    EXPECT_EQ(SendOpenRequest(fileHandle, cbData), 0);
    EXPECT_EQ(SendOpenRequest(fileHandle, cbData), -1);
}

/*
 * 用例名称：SendReadRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, SendReadRequest) {
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->params.srcRootPath = "/opt/oss";
    // SmbContextArgs smbContextArgs;
    // std::shared_ptr<Module::SmbContextWrapper> smbWrapper = make_shared<Module::SmbContextWrapper>(smbContextArgs);
    // cbData->readSmbContext = smbWrapper;
    EXPECT_EQ(SendReadRequest(fileHandle, cbData), -1);
    MOCKER_CPP(&Module::SmbContextWrapper::SmbReadAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    fileHandle.m_block.m_buffer = new uint8_t[1];
    EXPECT_EQ(SendReadRequest(fileHandle, cbData), 0);
    EXPECT_EQ(SendReadRequest(fileHandle, cbData), -1);
}

/*
 * 用例名称：SendCloseRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, SendCloseRequest) {
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->params.srcRootPath = "/opt/oss";
    MOCKER_CPP(&Module::SmbContextWrapper::SmbCloseAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    EXPECT_EQ(SendCloseRequest(fileHandle, cbData), 0);
    EXPECT_EQ(SendCloseRequest(fileHandle, cbData), -1);
}

/*
 * 用例名称：SendAdsRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, SmbAdsCb) {
    auto cbData = new(nothrow) SmbReaderCommonData();
    void *data;
    if (cbData == nullptr) {
        return;
    }
    BackupTimer timer;
    cbData->timer = &timer;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    //    struct smb2_context smb2;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbAdsCb(nullptr, -1, data, cbData));
    cbData = new(nothrow) SmbReaderCommonData();
    cbData->timer = &timer;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    // ENOENT == 2
    EXPECT_NO_THROW(SmbAdsCb(nullptr, -2, data, cbData));
}

/*
 * 用例名称：SendAdsRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, SendAdsRequest) {
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    fileHandle.m_file->m_fileName = "";
    cbData->params.srcRootPath = "/opt/oss/ads.txt";
    MOCKER_CPP(&Module::SmbContextWrapper::SmbGetStreamInfoAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    EXPECT_EQ(SendAdsRequest(fileHandle, cbData), 0);
    EXPECT_EQ(SendAdsRequest(fileHandle, cbData), -1);
}

/*
 * 用例名称：验证LibsmbCopyWriter线程启动
 * 前置条件：无
 * check点：线程正常启动后，正常退出
 */
TEST_F(LibsmbReaderInterfaceTest, SmbOpenCb) {
    int status = 0;
    void *data;
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }

    shared_ptr<BackupQueue<FileHandle>> readQueue = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    std::shared_ptr<BlockBufferMap> blockBufferMap = make_shared<BlockBufferMap>();
    cbData->blockBufferMap = blockBufferMap;
    cbData->readQueue = readQueue;
    cbData->aggregateQueue = aggregateQueue;
    FileHandle fileHandle;

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::LIBSMB, BackupIOEngine::LIBSMB);
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    fileHandle.m_file->m_size = 1;
    cbData->fileHandle = fileHandle;

    BackupTimer timer;
    cbData->timer = &timer;
    shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo = controlInfo;

    status = -1;
    SmbOpenCb(nullptr, status, data, cbData);
    EXPECT_EQ(controlInfo->m_noOfFilesReadFailed, 1);
}

/*
 * 用例名称：验证LibsmbCopyWriter线程启动
 * 前置条件：无
 * check点：线程正常启动后，正常退出
 */
TEST_F(LibsmbReaderInterfaceTest, SmbReadCb) {
    int status = 0;
    void *data;
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }

    BackupQueueConfig config;
    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    shared_ptr<BackupQueue<FileHandle>> aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    cbData->readQueue = make_shared<BackupQueue<FileHandle>>(config);
    cbData->aggregateQueue = aggregateQueue;
    FileHandle fileHandle;

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::LIBSMB, BackupIOEngine::LIBSMB);
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);
    cbData->fileHandle = fileHandle;

    BackupTimer timer;
    cbData->timer = &timer;
    cbData->controlInfo = make_shared<BackupControlInfo>();

    MOCKER_CPP(SendReaderRequest)
            .stubs()
            .will(ignoreReturnValue());

    SmbReadCb(nullptr, status, data, cbData);
    // smb初始化失败，start返回failed
    FileHandle fh;
    aggregateQueue->WaitAndPop(fh);
    EXPECT_EQ(fh.m_file->m_fileName, fileHandle.m_file->m_fileName);
}

/*
 * 用例名称：SmbCloseCb_FAIL_STSTUS
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, SmbCloseCb_FAIL_STSTUS) {
    void *data;
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesRead = 0;
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbCloseCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbCloseCb_SUCCESS_STSTUS
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, SmbCloseCb_SUCCESS_STSTUS) {
    void *data;
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesRead = 0;
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(SmbCloseCb(nullptr, 0, data, cbData));
}

/*
 * 用例名称：HandleSmbReadStatusSuccess
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, HandleSmbReadStatusSuccess) {
    void *data;
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesRead = 0;
    cbData->controlInfo->m_readProduce = 0;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileDesc::GetSrcState)
            .stubs()
            .will(returnValue(FileDescState::SRC_OPENED));
    MOCKER_CPP(SendReaderRequest)
        .stubs()
        .will(ignoreReturnValue());
    EXPECT_NO_THROW(HandleSmbReadStatusSuccess(cbData));
}

/*
* 用例名称：HandleSmbReadStatusFailed
* 前置条件：无
* check点：结果预期相符
*/
TEST_F(LibsmbReaderInterfaceTest, HandleSmbReadStatusFailed) {
    void *data;
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->blockBufferMap = make_shared<BlockBufferMap>();
    cbData->controlInfo->m_noOfFilesRead = 0;
    cbData->controlInfo->m_readProduce = 0;
    cbData->controlInfo->m_noOfFilesFailed = 0;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    cbData->fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::LIBSMB, BackupIOEngine::LIBSMB);
    MOCKER_CPP(IsFileReadOrWriteFailed)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileDesc::GetSrcState)
            .stubs()
            .will(returnValue(FileDescState::SRC_OPENED));
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(HandleSmbReadStatusFailed(nullptr, data, 1, cbData));
}

/*
 * 用例名称：HandleSmbOpenStatusFailed_NeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, HandleSmbOpenStatusFailed_NeedRetry) {
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    BackupTimer timer;
    cbData->timer = &timer;
    cbData->pktStats = make_shared<PacketStats>();
    MOCKER_CPP(IfNeedRetry)
        .stubs()
        .will(returnValue(true));
    EXPECT_NO_THROW(HandleSmbOpenStatusFailed(nullptr, 1, cbData));
}

/*
 * 用例名称：HandleSmbOpenStatusFailed
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, HandleSmbOpenStatusFailed) {
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    BackupTimer timer;
    cbData->timer = &timer;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesRead = 0;
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(HandleSmbOpenStatusFailed(nullptr, 1, cbData));
}

/*
 * 用例名称：SmbOpenCbSendBlock
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, SmbOpenCbSendBlock) {
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->params.blockSize = 2;
    cbData->fileHandle = fileHandle;
    EXPECT_NO_THROW(SmbOpenCbSendBlock(cbData));

    // 预制数据file size为1
    cbData->fileHandle.m_file->m_size = 1;
    MOCKER_CPP(&BlockBufferMap::Add)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(SendReaderRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    EXPECT_NO_THROW(SmbOpenCbSendBlock(cbData));
    EXPECT_NO_THROW(SmbOpenCbSendBlock(cbData));
}

/*
 * 用例名称：ReaderCallBack
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbReaderInterfaceTest, ReaderCallBack) {
    void *data;
    void *privateData;

    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->params.blockSize = 2;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    cbData->event = LibsmbEvent::OPEN_SRC;
    privateData = cbData;
    MOCKER_CPP(&PacketStats::Increment)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(CheckStatusAndIncStat)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(SmbOpenCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(ReaderCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::READ;
    MOCKER_CPP(SmbReadCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(ReaderCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::CLOSE_SRC;
    MOCKER_CPP(SmbCloseCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(ReaderCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::ADS;
    MOCKER_CPP(SmbAdsCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(ReaderCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::INVALID;
    EXPECT_NO_THROW(ReaderCallBack(nullptr, -1, data, privateData));

    cbData->fileHandle.m_file = nullptr;
    privateData = cbData;
    EXPECT_NO_THROW(ReaderCallBack(nullptr, -1, data, privateData));
}
