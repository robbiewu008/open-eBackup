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
#include "interface/LibsmbWriterInterface.h"
#include "libsmb_ctx/SmbContextWrapper.h"

using namespace std;
using namespace Module;
namespace  {
    const std::string MOCK_SMB_ERR = "MOCK_SMB_ERR";
}

// struct smb2_context;

class LibsmbWriterInterfaceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    BackupQueueConfig config;
};

void LibsmbWriterInterfaceTest::SetUp()
{
    fileHandle.m_file = fileDesc;
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    fileHandle.m_file->m_dirName = "/opt/oss";
    fileHandle.m_file->m_fileAttr = 1;
    fileHandle.m_file->m_btime = 1;
    fileHandle.m_file->m_atime = 1;
    fileHandle.m_file->m_mtime = 1;
    fileHandle.m_file->m_ctime = 1;
    fileHandle.m_file->ClearFlag(IS_DIR);
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

void LibsmbWriterInterfaceTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibsmbWriterInterfaceTest::SetUpTestCase()
{}

void LibsmbWriterInterfaceTest::TearDownTestCase()
{}

/*
 * 用例名称：SendWriterRequest_SendOpenDstRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SendWriterRequest_SendOpenDstRequest) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();
    cbData->params.backupType = BackupType::RESTORE;
    cbData->fileHandle = fileHandle;
    MOCKER_CPP(&FileDesc::IsFlagSet)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&Module::SmbContextWrapper::SmbOpenAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::OPEN_DST), 0);
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::OPEN_DST), -1);
}

/*
 * 用例名称：SendWriterRequest_SendWriteRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SendWriterRequest_SendWriteRequest) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->pktStats = make_shared<PacketStats>();
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::WRITE), -1);

    cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->pktStats = make_shared<PacketStats>();
    fileHandle.m_block.m_buffer = new uint8_t[1];
    MOCKER_CPP(&Module::SmbContextWrapper::SmbWriteAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::WRITE), 0);
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::WRITE), -1);
}

/*
 * 用例名称：SendWriterRequest_SendCloseDstRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SendWriterRequest_SendCloseDstRequest) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->pktStats = make_shared<PacketStats>();
    MOCKER_CPP(&Module::SmbContextWrapper::SmbCloseAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::CLOSE_DST), 0);
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::CLOSE_DST), -1);
}

/*
 * 用例名称：SendWriterRequest_SendSetSdRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SendWriterRequest_SendSetSdRequest) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->pktStats = make_shared<PacketStats>();
    MOCKER_CPP(&Module::SmbContextWrapper::SmbSetSdAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::SET_SD), 0);
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::SET_SD), -1);
}

/*
 * 用例名称：SendWriterRequest_SendStatRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SendWriterRequest_SendStatRequest) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->pktStats = make_shared<PacketStats>();
    MOCKER_CPP(&Module::SmbContextWrapper::SmbStatAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::STAT_DST), 0);
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::STAT_DST), -1);
}

/*
 * 用例名称：SendWriterRequest_SendSetBasicInfoRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SendWriterRequest_SendSetBasicInfoRequest) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->pktStats = make_shared<PacketStats>();
    MOCKER_CPP(&Module::SmbContextWrapper::SmbSetBasicInfoAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::SET_BASIC_INFO), 0);
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::SET_BASIC_INFO), -1);
}

static string smbGetError_Stub()
{
    return "error";
}

/*
 * 用例名称：SendWriterRequest_SendHardlinkRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SendWriterRequest_SendHardlinkRequest) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->linkTargetPath = "/opt/oss";
    cbData->pktStats = make_shared<PacketStats>();
    MOCKER_CPP(&Module::SmbContextWrapper::SmbHardLinkAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
            .stubs()
            .will(invoke(smbGetError_Stub));
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::LINK), 0);
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::LINK), -1);
}

/*
 * 用例名称：SendWriterRequest_SendUnlinkRequest
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SendWriterRequest_SendUnlinkRequest) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->params.dstRootPath = "/opt/oss";
    cbData->pktStats = make_shared<PacketStats>();
    MOCKER_CPP(&Module::SmbContextWrapper::SmbUnlinkAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-1));
    MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
            .stubs()
            .will(invoke(smbGetError_Stub));
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::UNLINK), 0);
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::UNLINK), -1);
}

/*
 * 用例名称：SendWriterRequest_Default
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SendWriterRequest_Default) {
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->pktStats = make_shared<PacketStats>();
    EXPECT_EQ(SendWriterRequest(fileHandle, cbData, LibsmbEvent::ADS), 0);
    delete cbData;
    cbData = nullptr;
}

/*
 * 用例名称：WriterCallBack
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, WriterCallBack) {
    void *data;
    void *privateData;

    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    cbData->event = LibsmbEvent::OPEN_DST;
    privateData = cbData;
    MOCKER_CPP(&PacketStats::Increment)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(CheckStatusAndIncStat)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(SmbOpenDstCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(WriterCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::WRITE;
    MOCKER_CPP(SmbWriteCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(WriterCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::CLOSE_DST;
    MOCKER_CPP(SmbCloseDstCb)
    .stubs()
    .will(ignoreReturnValue());
    EXPECT_NO_THROW(WriterCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::SET_SD;
    MOCKER_CPP(SmbSetSdCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(WriterCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::STAT_DST;
    MOCKER_CPP(SmbStatCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(WriterCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::SET_BASIC_INFO;
    MOCKER_CPP(SmbSetBasicInfoCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(WriterCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::LINK;
    MOCKER_CPP(SmbHardLinkCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(WriterCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::UNLINK;
    MOCKER_CPP(SmbUnlinkCb)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(WriterCallBack(nullptr, -1, data, privateData));

    cbData->event = LibsmbEvent::INVALID;
    EXPECT_NO_THROW(WriterCallBack(nullptr, -1, data, privateData));

    EXPECT_NO_THROW(WriterCallBack(nullptr, -9, data, privateData));

    cbData->fileHandle.m_file = nullptr;
    privateData = cbData;
    // cbData->fileHandle.m_file 会导致 WriterCallBack 释放 cbData，因此最后不用 delete cbData;
    EXPECT_NO_THROW(WriterCallBack(nullptr, -1, data, privateData));
}

/*
 * 用例名称：SmbOpenDstCb_FileSize0
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbOpenDstCb_FileSize0) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    shared_ptr<BackupQueue<FileHandle>> writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    cbData->writeQueue = writeQueue;
    cbData->fileHandle = fileHandle;
    EXPECT_NO_THROW(SmbOpenDstCb(nullptr, 1, data, cbData));
}

/*
 * 用例名称：SmbOpenDstCb_FileSizeNot0
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbOpenDstCb_FileSizeNot0) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;

    // fileSize大小非0
    cbData->fileHandle.m_file->m_size = 1;
    EXPECT_NO_THROW(SmbOpenDstCb(nullptr, 1, data, cbData));
}

/*
 * 用例名称：SmbOpenDstCb_NeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbOpenDstCb_NeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(SmbEnqueueToTimer)
            .stubs()
            .will(ignoreReturnValue());
    cbData->fileHandle = fileHandle;
    cbData->pktStats = make_shared<PacketStats>();
    EXPECT_NO_THROW(SmbOpenDstCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbOpenDstCb_Status_ENOTDIR
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbOpenDstCb_Status_ENOTDIR) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    shared_ptr<BackupQueue<FileHandle>> dirQueue = make_shared<BackupQueue<FileHandle>>(config);
    cbData->dirQueue = dirQueue;
    cbData->fileHandle = fileHandle;
    EXPECT_NO_THROW(SmbOpenDstCb(nullptr, -20, data, cbData));
}

/*
 * 用例名称：SmbOpenDstCb_NEXT
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbOpenDstCb_NEXT) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    MOCKER_CPP(ProcessRestorePolicy)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesRead = 0;
    EXPECT_NO_THROW(SmbOpenDstCb(nullptr, -17, data, cbData));
    EXPECT_NO_THROW(SmbOpenDstCb(nullptr, -3, data, cbData));
}

/*
 * 用例名称：ProcessRestorePolicy
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, ProcessRestorePolicy) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesRead = 0;
    cbData->controlInfo->m_noOfBytesCopied = 1;
    cbData->params.restoreReplacePolicy = RestoreReplacePolicy::IGNORE_EXIST;
    cbData->blockBufferMap = make_shared<BlockBufferMap>();
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(ProcessRestorePolicy(cbData), 0);

    // cbData->fileHandle = fileHandle;
    // cbData->params.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE_OLDER;
    // MOCKER_CPP(SendWriterRequest)
    //         .stubs()
    //         .will(ignoreReturnValue());
    // EXPECT_EQ(ProcessRestorePolicy(cbData), 0);

    // cbData->fileHandle = fileHandle;
    // cbData->params.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    // EXPECT_EQ(ProcessRestorePolicy(cbData), 0);

    // cbData->fileHandle = fileHandle;
    // cbData->params.restoreReplacePolicy = RestoreReplacePolicy::NONE;
    // EXPECT_EQ(ProcessRestorePolicy(cbData), -1);
    // delete cbData;
    // cbData = nullptr;
}

/*
 * 用例名称：SmbUnlinkCb
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbUnlinkCb) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    BackupTimer timer;
    cbData->timer = &timer;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true));
    EXPECT_NO_THROW(SmbUnlinkCb(nullptr, 0, data, cbData));\
}

/*
 * 用例名称：SmbStatCb_NeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbStatCb_NeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    BackupTimer timer;
    cbData->timer = &timer;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbStatCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbStatCb_NotNeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbStatCb_NotNeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    BackupTimer timer;
    cbData->timer = &timer;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbStatCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbStatCb_NotNeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbStatCb_smb2) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    auto smb2 = new(nothrow) smb2_stat_64();
    smb2->smb2_mtime = 0;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    data = smb2;
    MOCKER_CPP(SendWriterRequest)
            .stubs()
            .will(ignoreReturnValue());
    // smb2_mtime < cbData->fileHandle.m_file->m_mtime
    EXPECT_NO_THROW(SmbStatCb(nullptr, 0, data, cbData));

    // smb2_mtime = cbData->fileHandle.m_file->m_mtime
    smb2->smb2_mtime = 1;
    data = smb2;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesCopied = 0;
    cbData->controlInfo->m_noOfBytesCopied = 1;
    cbData->blockBufferMap = make_shared<BlockBufferMap>();
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(SmbStatCb(nullptr, 0, data, cbData));
    delete smb2;
}

/*
 * 用例名称：SmbSetBasicInfoCb_NeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbSetBasicInfoCb_NeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    BackupTimer timer;
    cbData->timer = &timer;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbSetBasicInfoCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbSetBasicInfoCb_NotNeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbSetBasicInfoCb_NotNeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesFailed = 1;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbSetBasicInfoCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbSetBasicInfoCb
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbSetBasicInfoCb) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesFailed = 1;
    cbData->controlInfo->m_noOfBytesCopied = 1;
    EXPECT_NO_THROW(SmbSetBasicInfoCb(nullptr, 0, data, cbData));
}

/*
 * 用例名称：SmbSetSdCb_NeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbSetSdCb_NeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    BackupTimer timer;
    cbData->timer = &timer;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbSetSdCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbSetSdCb_NotNeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbSetSdCb_NotNeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfDirFailed = 1;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbSetSdCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbSetSdCb
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbSetSdCb) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    fileHandle.m_file->ClearFlag(IS_DIR);
    cbData->fileHandle = fileHandle;
    MOCKER_CPP(SendWriterRequest)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(SmbSetSdCb(nullptr, 0, data, cbData));

    cbData->fileHandle.m_file->SetFlag(IS_DIR);
    cbData->controlInfo = make_shared<BackupControlInfo>();
    EXPECT_NO_THROW(SmbSetSdCb(nullptr, 0, data, cbData));
}

/*
 * 用例名称：SmbCloseDstCb_NeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbCloseDstCb_NeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    BackupTimer timer;
    cbData->timer = &timer;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true));
    EXPECT_NO_THROW(SmbCloseDstCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbCloseDstCb_NotNeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbCloseDstCb_NotNeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesFailed = 1;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbCloseDstCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbCloseDstCb
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbCloseDstCb) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    shared_ptr<BackupQueue<FileHandle>> writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    cbData->writeQueue = writeQueue;
    cbData->fileHandle = fileHandle;
    EXPECT_NO_THROW(SmbCloseDstCb(nullptr, 0, data, cbData));
}

/*
 * 用例名称：SmbHardLinkCb_NeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbHardLinkCb_NeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    BackupTimer timer;
    cbData->timer = &timer;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbHardLinkCb(nullptr, -1, data, cbData));
}

/*
 * 用例名称：SmbHardLinkCb_ESTALE
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbHardLinkCb_ESTALE) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    shared_ptr<BackupQueue<FileHandle>> dirQueue = make_shared<BackupQueue<FileHandle>>(config);
    cbData->dirQueue = dirQueue;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbHardLinkCb(nullptr, -116, data, cbData));
}

/*
 * 用例名称：SmbHardLinkCb_EEXIST
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbHardLinkCb_EEXIST) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    BackupTimer timer;
    cbData->timer = &timer;
    shared_ptr<BackupQueue<FileHandle>> dirQueue = make_shared<BackupQueue<FileHandle>>(config);
    cbData->dirQueue = dirQueue;
    cbData->params.backupType = BackupType::BACKUP_FULL;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    MOCKER_CPP(SendWriterRequest)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(SmbHardLinkCb(nullptr, -17, data, cbData));

    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesFailed = 1;
    EXPECT_NO_THROW(SmbHardLinkCb(nullptr, -18, data, cbData));
}

/*
 * 用例名称：SmbHardLinkCb
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbHardLinkCb) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->hardlinkMap = make_shared<HardLinkMap>();
    cbData->fileHandle = fileHandle;
    shared_ptr<BackupQueue<FileHandle>> dirQueue = make_shared<BackupQueue<FileHandle>>(config);
    cbData->dirQueue = dirQueue;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesCopied = 1;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&HardLinkMap::IncreaseRef)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    EXPECT_NO_THROW(SmbHardLinkCb(nullptr, 0, data, cbData));
}

/*
 * 用例名称：SmbWriteCb_NeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbWriteCb_NeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    BackupTimer timer;
    cbData->timer = &timer;
    cbData->pktStats = make_shared<PacketStats>();
    cbData->fileHandle = fileHandle;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(true));
    EXPECT_NO_THROW(SmbWriteCb(nullptr, -102, data, cbData));
}

/*
 * 用例名称：SmbWriteCb_NotNeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbWriteCb_NotNeedRetry) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesFailed = 1;
    cbData->fileHandle.m_file->m_blockStats.m_totalCnt = 2;
    cbData->fileHandle.m_file->m_size = 1;
    cbData->fileHandle.m_block.m_buffer = new uint8_t[1];
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(SendWriterRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_NO_THROW(SmbWriteCb(nullptr, -1, data, cbData));
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    fileHandle.m_file->m_size = 0;
}

/*
 * 用例名称：SmbWriteCb
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbWriterInterfaceTest, SmbWriteCb) {
    void *data;
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        return;
    }
    cbData->fileHandle = fileHandle;
    cbData->controlInfo = make_shared<BackupControlInfo>();
    cbData->controlInfo->m_noOfFilesFailed = 1;
    MOCKER_CPP(IfNeedRetry)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(smb2_get_error)
            .stubs()
            .will(returnValue(MOCK_SMB_ERR.c_str()));
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(SendWriterRequest)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(SmbWriteCb(nullptr, 0, data, cbData));
}
