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
#include "interface/LibsmbCommon.h"

using namespace std;
using namespace Module;

class LibsmbCommonTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void LibsmbCommonTest::SetUp()
{}

void LibsmbCommonTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibsmbCommonTest::SetUpTestCase()
{}

void LibsmbCommonTest::TearDownTestCase()
{}

/*
 * 用例名称：验证SmbVersion的转换
 * 前置条件：无
 * check点：返回版本正确
 */
TEST_F(LibsmbCommonTest, ConvertStringToSmbVersion) {
    EXPECT_EQ(ConvertStringToSmbVersion("3.1.1"), Module::SmbVersion::VERSION0311);
    EXPECT_EQ(ConvertStringToSmbVersion("3.02"), Module::SmbVersion::VERSION0302);
    EXPECT_EQ(ConvertStringToSmbVersion("3.0"), Module::SmbVersion::VERSION0300);
    EXPECT_EQ(ConvertStringToSmbVersion("2.1"), Module::SmbVersion::VERSION0210);
    EXPECT_EQ(ConvertStringToSmbVersion("2.0"), Module::SmbVersion::VERSION0202);
    EXPECT_EQ(ConvertStringToSmbVersion("3.0"), Module::SmbVersion::VERSION0300);
}

/*
 * 用例名称：验证FillContextParams
 * 前置条件：无
 * check点：校验参数正确
 */
TEST_F(LibsmbCommonTest, FillContextParams) {
    shared_ptr<LibsmbBackupAdvanceParams> advParams = make_shared<LibsmbBackupAdvanceParams>();
    Module::SmbContextArgs smbContextArgs;
    advParams->server = "";
    EXPECT_EQ(FillContextParams(smbContextArgs, advParams), false);
    advParams->share = "";

    advParams->server = "192.168.129.152";
    advParams->share = "shareName";
    EXPECT_EQ(FillContextParams(smbContextArgs, advParams), false);
    advParams->authType = "krb5";
    EXPECT_EQ(FillContextParams(smbContextArgs, advParams), true);
    advParams->authType = "ntlmssp";
    EXPECT_EQ(FillContextParams(smbContextArgs, advParams), true);
}

/*
 * 用例名称：验证LibsmbEventName的转换
 * 前置条件：无
 * check点：返回事件名字成功
 */
TEST_F(LibsmbCommonTest, GetLibsmbEventName) {
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::OPEN_SRC), "OPEN_SRC");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::READ), "READ");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::CLOSE_SRC), "CLOSE_SRC");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::ADS), "ADS");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::OPEN_DST), "OPEN_DST");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::WRITE), "WRITE");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::CLOSE_DST), "CLOSE_DST");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::SET_SD), "SET_SD");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::STAT_DST), "STAT_DST");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::SET_BASIC_INFO), "SET_BASIC_INFO");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::MKDIR), "MKDIR");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::DELETE), "DELETE");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::LINK), "LINK");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::UNLINK), "UNLINK");
    EXPECT_EQ(GetLibsmbEventName(LibsmbEvent::INVALID), "No Such Event");
}

/*
 * 用例名称：libsmb的event to PKT_TYPE
 * 前置条件：无
 * check点：轉化成功
 */
TEST_F(LibsmbCommonTest, LibsmbEventToPKT_TYPE) {
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::OPEN_SRC), PKT_TYPE::OPEN);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::READ), PKT_TYPE::READ);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::CLOSE_SRC), PKT_TYPE::CLOSE);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::ADS), PKT_TYPE::LSTAT);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::OPEN_DST), PKT_TYPE::CREATE);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::WRITE), PKT_TYPE::WRITE);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::CLOSE_DST), PKT_TYPE::CLOSE);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::SET_SD), PKT_TYPE::SETMETA);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::STAT_DST), PKT_TYPE::LSTAT);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::SET_BASIC_INFO), PKT_TYPE::SETMETA);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::MKDIR), PKT_TYPE::MKDIR);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::DELETE), PKT_TYPE::LINKDELETE);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::LINK), PKT_TYPE::HARDLINK);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::UNLINK), PKT_TYPE::OPEN);
    EXPECT_EQ(LibsmbEventToPKT_TYPE(LibsmbEvent::INVALID), PKT_TYPE::OPEN);
}

/*
 * 用例名称：删除前置的路径分隔符
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbCommonTest, RemoveFirstSeparator) {
    std::string path = "";
    EXPECT_EQ(RemoveFirstSeparator(path), "");
    path = ".//test";
    EXPECT_EQ(RemoveFirstSeparator(path), "test");
}

/*
 * 用例名称：获取路径名称
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbCommonTest, GetPathName) {
    std::string path = "";
    EXPECT_EQ(GetPathName(path), "");
    path = "/test/smb.txt";
    EXPECT_EQ(GetPathName(path), "/test");
}

/*
 * 用例名称：获取路径名称
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbCommonTest, SmbDisconnectContext) {

    std::shared_ptr<SmbContextWrapper> rootSmb;
    EXPECT_EQ(SmbDisconnectContext(rootSmb), FAILED);
}

/*
 * 用例名称：获取路径名称
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbCommonTest, IsFileReadOrWriteFailed) {
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    MOCKER_CPP(&FileDesc::GetDstState)
            .stubs()
            .will(returnValue(FileDescState::WRITE_FAILED))
            .then(returnValue(FileDescState::READED));
    MOCKER_CPP(&FileDesc::GetSrcState)
            .stubs()
            .will(returnValue(FileDescState::READ_FAILED));
    EXPECT_EQ(IsFileReadOrWriteFailed(fileHandle), true);
    EXPECT_EQ(IsFileReadOrWriteFailed(fileHandle), true);
}

/*
 * 用例名称：IfNeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbCommonTest, IfNeedRetry) {
    int retryCnt = 0;
    int maxRetryTimes = 0;
    int status = 0;
    EXPECT_EQ(IfNeedRetry(retryCnt, maxRetryTimes, status), false);
    maxRetryTimes = 1;
    status = -EINTR;
    EXPECT_EQ(IfNeedRetry(retryCnt, maxRetryTimes, status), true);
}

/*
 * 用例名称：IfNeedRetry
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbCommonTest, HandleConnectionException) {
    SmbContextArgs contextArgs;
    contextArgs.authType = SmbAuthType::KRB5;
    std::shared_ptr<SmbContextWrapper> smbContext;

    MOCKER_CPP(&Module::SmbContextWrapper::SmbEcho)
            .stubs()
            .will(returnValue(SUCCESS));
    EXPECT_EQ(HandleConnectionException(smbContext, contextArgs, 1), SUCCESS);
}

/*
 * 用例名称：获取路径名称
 * 前置条件：无
 * check点：结果预期相符
 */
TEST_F(LibsmbCommonTest, SmbEnqueueToTimer) {
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_retryCnt = 1;
    auto timer = new(nothrow) BackupTimer();
    if (timer == nullptr) {
        return ;
    }
    EXPECT_EQ(SmbEnqueueToTimer(timer, fileHandle), SUCCESS);
    delete timer;
}
