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
#include "gmock/gmock.h"
#include "mockcpp/mockcpp.hpp"
#include "llt_stub/named_stub.h"
#include "llt_stub/addr_pri.h"
#include "LibsmbDirReader.h"

using namespace std;

class LibsmbDirReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<LibsmbDirReader> m_libsmbDirReader = nullptr;
};

void LibsmbDirReaderTest::SetUp()
{
    BackupParams backupParams;
    backupParams.srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    ReaderParams dirReaderParams {};
    dirReaderParams.backupParams = backupParams;
    dirReaderParams.readQueuePtr = nullptr;
    dirReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    dirReaderParams.blockBufferMap = nullptr;
    m_libsmbDirReader = std::make_unique<LibsmbDirReader>(dirReaderParams);   
}

void LibsmbDirReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibsmbDirReaderTest::SetUpTestCase()
{}

void LibsmbDirReaderTest::TearDownTestCase()
{}

/*
 * 用例名称：验证LibsmbDirReader Abort函数
 * 前置条件：无
 * check点：Abort返回正确结果
 */
TEST_F(LibsmbDirReaderTest, Abort) {
    BackupParams backupParams;
    backupParams.srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    ReaderParams dirReaderParams {};
    dirReaderParams.backupParams = backupParams;
    dirReaderParams.readQueuePtr = nullptr;
    dirReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    dirReaderParams.blockBufferMap = nullptr;

    LibsmbDirReader libsmbDirReader(dirReaderParams);

    EXPECT_EQ(libsmbDirReader.Abort(), BackupRetCode::SUCCESS);
    EXPECT_EQ(libsmbDirReader.m_abort, true);
}

/*
 * 用例名称：验证LibsmbDirReader线程启动
 * 前置条件：无
 * check点：IsComplete返回正确结果
 */
TEST_F(LibsmbDirReaderTest, IsComplete) {
    BackupParams backupParams;
    backupParams.srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    ReaderParams dirReaderParams {};
    dirReaderParams.backupParams = backupParams;
    dirReaderParams.readQueuePtr = nullptr;
    dirReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    dirReaderParams.blockBufferMap = nullptr;

    LibsmbDirReader libsmbDirReader(dirReaderParams);

    EXPECT_EQ(libsmbDirReader.IsComplete(), false);
}

/*
 * 用例名称：GetStatus
 * 前置条件：无
 * check点：获取状态
 */
TEST_F(LibsmbDirReaderTest, GetStatus)
{   
    m_libsmbDirReader->m_controlInfo->m_readPhaseComplete = false;
    EXPECT_EQ(m_libsmbDirReader->GetStatus(), BackupPhaseStatus::INPROGRESS);
    m_libsmbDirReader->m_controlInfo->m_readPhaseComplete = true;
    m_libsmbDirReader->m_abort = true;
    EXPECT_EQ(m_libsmbDirReader->GetStatus(), BackupPhaseStatus::ABORTED);
    m_libsmbDirReader->m_abort = false;
    m_libsmbDirReader->m_controlInfo->m_failed  = true;
    EXPECT_EQ(m_libsmbDirReader->GetStatus(), BackupPhaseStatus::FAILED);
    m_libsmbDirReader->m_controlInfo->m_failed  = false;
    EXPECT_EQ(m_libsmbDirReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
 * 用例名称：IsAbort
 * 前置条件：无
 * check点：判断是否终止
 */
TEST_F(LibsmbDirReaderTest, IsAbort) 
{  
    m_libsmbDirReader->m_abort = false;
    m_libsmbDirReader->m_controlInfo->m_failed = false;
    m_libsmbDirReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_libsmbDirReader->IsAbort(), true);
    m_libsmbDirReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libsmbDirReader->IsAbort(), false);
}

/*
 * 用例名称：OpenFile
 * 前置条件：无
 * check点：打开文件等
 */
TEST_F(LibsmbDirReaderTest, OpenFile) 
{  
    FileHandle fileHandle;
    EXPECT_EQ(m_libsmbDirReader->OpenFile(fileHandle), 0);
    EXPECT_EQ(m_libsmbDirReader->ReadData(fileHandle), 0);
    EXPECT_EQ(m_libsmbDirReader->ReadMeta(fileHandle), 0);
    EXPECT_EQ(m_libsmbDirReader->CloseFile(fileHandle), 0);
}

/*
 * 用例名称：ThreadFunc
 * 前置条件：无
 * check点：线程函数
 */
TEST_F(LibsmbDirReaderTest, ThreadFunc) 
{  
    MOCKER_CPP(&LibsmbDirReader::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&LibsmbDirReader::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&BackupQueue<FileHandle>::WaitAndPop, bool(BackupQueue<FileHandle>::*)(FileHandle&, uint32_t)) // 模板类中重载函数打桩
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&BackupQueue<FileHandle>::Push, void(BackupQueue<FileHandle>::*)(FileHandle)) // 模板类中重载函数打桩
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libsmbDirReader->ThreadFunc());
    EXPECT_NO_THROW(m_libsmbDirReader->ThreadFunc());
    EXPECT_NO_THROW(m_libsmbDirReader->ThreadFunc());
}

static bool StubWaitAndPop(BackupQueue<FileHandle>* This, FileHandle& args, uint32_t args2)
{
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    args = fileHandle;
    return true;
}

TEST_F(LibsmbDirReaderTest, ThreadFunc_2) 
{  
    MOCKER_CPP(&LibsmbDirReader::IsComplete)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&LibsmbDirReader::IsAbort)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&BackupQueue<FileHandle>::WaitAndPop, bool(BackupQueue<FileHandle>::*)(FileHandle&, uint32_t)) // 模板类中重载函数打桩
            .stubs()
            .will(invoke(StubWaitAndPop));
    MOCKER_CPP(&BackupQueue<FileHandle>::Push, void(BackupQueue<FileHandle>::*)(FileHandle)) // 模板类中重载函数打桩
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libsmbDirReader->ThreadFunc());
}

/*
 * 用例名称：Start
 * 前置条件：无
 * check点：启动
 */
TEST_F(LibsmbDirReaderTest, Start) 
{  
    // boost::filesystem::path path1 = "/a/b.txt";
    // boost::filesystem::path path2 = "/a/b.txt";
    // boost::system::error_code ec;
    // MOCKER_CPP(boost::filesystem::exists, bool(const boost::filesystem::path&, boost::system::error_code&))
    //         .stubs()
    //         .will(throws(boost::filesystem::filesystem_error(str, path1, path2, ec)));

    // MOCKER_CPP(std::thread)
    //         .stubs()
    //         .will(returnValue(false))
    //         .then(returnValue(true));
    // EXPECT_NO_THROW(m_libsmbDirReader->Start()); // std::thread函数打桩问题
}