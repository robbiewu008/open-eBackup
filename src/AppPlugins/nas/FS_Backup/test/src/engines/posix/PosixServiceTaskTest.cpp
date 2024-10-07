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
#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "PosixServiceTask.h"
#include "log/Log.h"
#include "ThreadPool.h"
#include "ThreadPoolFactory.h"
#include "BackupStructs.h"

using ::testing::_;
using testing::AllOf;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using testing::ByMove;
using testing::DoAll;
using ::testing::Eq;
using ::testing::Field;
using ::testing::Ge;
using ::testing::Gt;
using testing::InitGoogleMock;
using ::testing::Invoke;
using ::testing::Ne;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::SetArgumentPointee;
using ::testing::Throw;

using namespace std;
using namespace Module;
using namespace FS_Backup;

class PosixServiceTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<PosixServiceTask> m_posixServiceTask = nullptr;
    Stub stub;
};

void PosixServiceTaskTest::SetUp()
{
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    FileHandle fileHandle;
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    
    HostEvent m_event = HostEvent::INVALID;
    std::shared_ptr<BlockBufferMap> m_bufferMapPtr = std::make_shared<BlockBufferMap>();
    HostParams m_params;
    BackupPhaseStatus m_backupFailReason = BackupPhaseStatus::FAILED;
    
    m_posixServiceTask = std::make_unique<PosixServiceTask>(m_event, m_bufferMapPtr, fileHandle, m_params);
}

void PosixServiceTaskTest::TearDown() {}

void PosixServiceTaskTest::SetUpTestCase() {}

void PosixServiceTaskTest::TearDownTestCase() {}

static int symlink_Stub_Suc(void *obj, const char *target, const char *linkpath)
{
    return SUCCESS;
}

static int symlink_Stub_Fail_ENOENT(void *obj, const char *target, const char *linkpath)
{
    errno = ENOENT;
    return FAILED;
}

static int symlink_Stub_Fail_EEXIST(void *obj, const char *target, const char *linkpath)
{
    errno = EEXIST;
    return FAILED;
}

static int mknod_Stub_Suc(void *obj, const char *path, mode_t mode, dev_t dev)
{
    std::cout << "enter mknod_Stub_Suc" << std::endl;
    return 0;
}

static int mknod_Stub_Fail_ENOENT(void *obj, const char *path, mode_t mode, dev_t dev)
{
    errno = ENOENT;
    return -1;
}

static int mknod_Stub_Fail_EEXIST(void *obj, const char *path, mode_t mode, dev_t dev)
{
    errno = EEXIST;
    return -1;
}

static ssize_t pread_Stub_1(void *obj, int fd, void *buf, size_t count, off_t offset)
{
    return 1;
}

static ssize_t pwrite_Stub_1(void *obj, int fd, void *buf, size_t count, off_t offset)
{
    return 1;
}

static int lchown_Stub_0(void *obj, const char *path, uid_t owner, gid_t group)
{
    return 0;
}

static int link_Stub_0(void *obj, const char *oldname, const char *newname)
{
    return 0;
}

/*
* 用例名称：CheckExec
* 前置条件：
* check点：检查Exec返回值
*/
TEST_F(PosixServiceTaskTest, CheckExec)
{
    EXPECT_NO_THROW(m_posixServiceTask->Exec());

    m_posixServiceTask->m_event = HostEvent::OPEN_SRC;
    EXPECT_NO_THROW(m_posixServiceTask->Exec());

    m_posixServiceTask->m_event = HostEvent::OPEN_DST;
    EXPECT_NO_THROW(m_posixServiceTask->Exec());

    m_posixServiceTask->m_event = HostEvent::READ_DATA;
    EXPECT_NO_THROW(m_posixServiceTask->Exec());
    
    m_posixServiceTask->m_event = HostEvent::READ_META;
    EXPECT_NO_THROW(m_posixServiceTask->Exec());
    
    m_posixServiceTask->m_event = HostEvent::WRITE_DATA;
    EXPECT_NO_THROW(m_posixServiceTask->Exec());
    
    m_posixServiceTask->m_event = HostEvent::WRITE_META;
    EXPECT_NO_THROW(m_posixServiceTask->Exec());
    
    m_posixServiceTask->m_event = HostEvent::LINK;
    EXPECT_NO_THROW(m_posixServiceTask->Exec());
    
    m_posixServiceTask->m_event = HostEvent::CLOSE_SRC;
    EXPECT_NO_THROW(m_posixServiceTask->Exec());
    
    m_posixServiceTask->m_event = HostEvent::CLOSE_DST;
    EXPECT_NO_THROW(m_posixServiceTask->Exec());
    
    // m_posixServiceTask->m_event = HostEvent::DELETE_ITEM;
    // EXPECT_NO_THROW(m_posixServiceTask->Exec());

    m_posixServiceTask->m_event = HostEvent::CREATE_DIR;
    EXPECT_NO_THROW(m_posixServiceTask->Exec());
}

/*
* 用例名称：CheckIsCriticalError
* 前置条件：
* check点：检查IsCriticalError返回值
*/
TEST_F(PosixServiceTaskTest, CheckIsCriticalError)
{
    m_posixServiceTask->m_backupFailReason = BackupPhaseStatus::FAILED_NOACCESS;
    EXPECT_EQ(m_posixServiceTask->IsCriticalError(), true);

    m_posixServiceTask->m_backupFailReason = BackupPhaseStatus::FAILED_NOSPACE;
    EXPECT_EQ(m_posixServiceTask->IsCriticalError(), true);

    m_posixServiceTask->m_backupFailReason = BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE;
    EXPECT_EQ(m_posixServiceTask->IsCriticalError(), true);

    m_posixServiceTask->m_backupFailReason = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    EXPECT_EQ(m_posixServiceTask->IsCriticalError(), true);

    m_posixServiceTask->m_backupFailReason = BackupPhaseStatus::INPROGRESS;
    EXPECT_EQ(m_posixServiceTask->IsCriticalError(), false);
}

/*
* 用例名称：CheckSetCriticalErrorInfo
* 前置条件：
* check点：检查SetCriticalErrorInfo返回值
*/
TEST_F(PosixServiceTaskTest, CheckSetCriticalErrorInfo)
{
    int err = ENOSPC;
    EXPECT_NO_THROW(m_posixServiceTask->SetCriticalErrorInfo(err));

    err = ESTALE;
    EXPECT_NO_THROW(m_posixServiceTask->SetCriticalErrorInfo(err));

    err = ENXIO;
    EXPECT_NO_THROW(m_posixServiceTask->SetCriticalErrorInfo(err));
}

/*
* 用例名称：CheckHandleOpenSrc
* 前置条件：
* check点：检查HandleOpenSrc返回值
*/
TEST_F(PosixServiceTaskTest, CheckHandleOpenSrc)
{
    m_posixServiceTask->m_fileHandle.m_file->m_size = 1;
    m_posixServiceTask->m_params.blockSize = 2;
    EXPECT_NO_THROW(m_posixServiceTask->HandleOpenSrc());
}

/*
* 用例名称：CheckProcessRestorePolicy
* 前置条件：
* check点：检查ProcessRestorePolicy返回值
*/
TEST_F(PosixServiceTaskTest, CheckProcessRestorePolicy)
{
    const string dstFile = "111";
    m_posixServiceTask->m_params.restoreReplacePolicy = RestoreReplacePolicy::IGNORE_EXIST;
    EXPECT_EQ(m_posixServiceTask->ProcessRestorePolicy(dstFile), SUCCESS);

    m_posixServiceTask->m_params.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE_OLDER;
    EXPECT_NO_THROW(m_posixServiceTask->ProcessRestorePolicy(dstFile));

    m_posixServiceTask->m_params.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    EXPECT_NO_THROW(m_posixServiceTask->ProcessRestorePolicy(dstFile));
}

/*
* 用例名称：CheckProcessReadSoftLinkData
* 前置条件：
* check点：检查ProcessReadSoftLinkData返回值
*/
TEST_F(PosixServiceTaskTest, CheckProcessReadSoftLinkData)
{
    EXPECT_EQ(m_posixServiceTask->ProcessReadSoftLinkData(), FAILED);
}

/*
* 用例名称：CheckProcessReadSpecialFileData
* 前置条件：
* check点：检查ProcessReadSpecialFileData返回值
*/
TEST_F(PosixServiceTaskTest, CheckProcessReadSpecialFileData)
{
    EXPECT_EQ(m_posixServiceTask->ProcessReadSpecialFileData(), SUCCESS);
}

/*
* 用例名称：CheckProcessWriteSoftLinkData
* 前置条件：
* check点：检查ProcessWriteSoftLinkData返回值
*/
TEST_F(PosixServiceTaskTest, CheckProcessWriteSoftLinkData)
{
    stub.set(symlink, symlink_Stub_Suc);
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSoftLinkData(), SUCCESS);
    stub.reset(symlink);

    stub.set(symlink, symlink_Stub_Fail_ENOENT);
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSoftLinkData(), FAILED);
    stub.reset(symlink);

    stub.set(symlink, symlink_Stub_Fail_EEXIST);
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSoftLinkData(), FAILED);
    stub.reset(symlink);
}

/*
* 用例名称：CheckProcessWriteSpecialFileData
* 前置条件：
* check点：检查ProcessWriteSpecialFileData返回值
*/
TEST_F(PosixServiceTaskTest, CheckProcessWriteSpecialFileData)
{
    Stub stub;
    m_posixServiceTask->m_params.backupType = BackupType::BACKUP_FULL;
    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0060000;
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSpecialFileData(), SUCCESS);
    
    m_posixServiceTask->m_params.backupType = BackupType::BACKUP_INC;
    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0020000;
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSpecialFileData(), SUCCESS);

    m_posixServiceTask->m_params.backupType = BackupType::BACKUP_FULL;
    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0001000;

    stub.set(mknod, mknod_Stub_Fail_ENOENT);
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSpecialFileData(), FAILED);
    stub.reset(mknod);

    stub.set(mknod, mknod_Stub_Fail_EEXIST);
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSpecialFileData(), FAILED);
    stub.reset(mknod);

}

/*
* 用例名称：CheckHandleCloseSrc
* 前置条件：
* check点：检查HandleCloseSrc返回值
*/
TEST_F(PosixServiceTaskTest, CheckHandleCloseSrc)
{
    m_posixServiceTask->m_fileHandle.m_file->m_size = 1;
    m_posixServiceTask->m_params.blockSize = 2;
    EXPECT_NO_THROW(m_posixServiceTask->HandleCloseSrc());

    m_posixServiceTask->m_fileHandle.m_file->m_size = 2;
    m_posixServiceTask->m_params.blockSize = 1;
    m_posixServiceTask->m_fileHandle.m_file->srcIOHandle.posixFd = -1;
    EXPECT_NO_THROW(m_posixServiceTask->HandleCloseSrc());
}

/*
* 用例名称：CheckHandleCloseDst
* 前置条件：
* check点：检查HandleCloseDst返回值
*/
TEST_F(PosixServiceTaskTest, CheckHandleCloseDst)
{
    m_posixServiceTask->m_fileHandle.m_file->m_size = 1;
    m_posixServiceTask->m_params.blockSize = 2;
    EXPECT_NO_THROW(m_posixServiceTask->HandleCloseDst());

    m_posixServiceTask->m_fileHandle.m_file->m_size = 2;
    m_posixServiceTask->m_params.blockSize = 1;
    m_posixServiceTask->m_fileHandle.m_file->srcIOHandle.posixFd = -1;
    EXPECT_NO_THROW(m_posixServiceTask->HandleCloseDst());
}

/*
* 用例名称：CheckHandleReadMeta
* 前置条件：
* check点：检查HandleReadMeta返回值
*/
TEST_F(PosixServiceTaskTest, CheckHandleReadMeta)
{
    EXPECT_NO_THROW(m_posixServiceTask->HandleReadMeta());
}

/*
* 用例名称：CheckHandleWriteMeta
* 前置条件：
* check点：检查HandleWriteMeta返回值
*/
TEST_F(PosixServiceTaskTest, CheckHandleWriteMeta)
{
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteMeta());

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0060000;
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteMeta());

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0020000;
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteMeta());

    stub.set(lchown, lchown_Stub_0);
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteMeta());
    stub.reset(lchown);
}

/*
* 用例名称：CheckHandleLink
* 前置条件：
* check点：检查HandleLink返回值
*/
TEST_F(PosixServiceTaskTest, CheckHandleLink)
{
    stub.set(link, link_Stub_0);
    EXPECT_NO_THROW(m_posixServiceTask->HandleLink());
    stub.reset(link);

}

/*
* 用例名称：CheckCloseSmallFileDstFd
* 前置条件：
* check点：检查CloseSmallFileDstFd返回值
*/
TEST_F(PosixServiceTaskTest, CheckCloseSmallFileDstFd)
{
    m_posixServiceTask->m_fileHandle.m_file->m_size = 1;
    m_posixServiceTask->m_params.blockSize = 2;
    EXPECT_NO_THROW(m_posixServiceTask->CloseSmallFileDstFd());
}

/*
* 用例名称：CheckHandleReadData
* 前置条件：
* check点：检查HandleReadData返回值
*/
TEST_F(PosixServiceTaskTest, CheckHandleReadData)
{
    /*
    #define S_IFMT  00170000
    #define S_IFSOCK 0140000
    #define S_IFLNK	 0120000
    #define S_IFREG  0100000
    #define S_IFBLK  0060000
    #define S_IFDIR  0040000
    #define S_IFCHR  0020000
    #define S_IFIFO  0010000
    #define S_ISUID  0004000
    #define S_ISGID  0002000
    #define S_ISVTX  0001000

    #define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
    #define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
    #define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
    #define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
    #define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
    #define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
    #define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)
    */
    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0120000;
    EXPECT_NO_THROW(m_posixServiceTask->HandleReadData());

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0060000;
    EXPECT_NO_THROW(m_posixServiceTask->HandleReadData());

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0020000;
    EXPECT_NO_THROW(m_posixServiceTask->HandleReadData());

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0010000;
    EXPECT_NO_THROW(m_posixServiceTask->HandleReadData());

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0001000;
    m_posixServiceTask->m_fileHandle.m_file->m_size = 2;
    m_posixServiceTask->m_params.blockSize = 1;
    m_posixServiceTask->m_fileHandle.m_file->srcIOHandle.posixFd = -1;
    EXPECT_NO_THROW(m_posixServiceTask->HandleReadData());

    m_posixServiceTask->m_fileHandle.m_file->srcIOHandle.posixFd = 0;
    EXPECT_NO_THROW(m_posixServiceTask->HandleReadData());

    m_posixServiceTask->m_fileHandle.m_block.m_size = 1;
    stub.set(pread, pread_Stub_1);
    EXPECT_NO_THROW(m_posixServiceTask->HandleReadData());
    stub.reset(pread);
}

/*
* 用例名称：CheckHandleWriteData
* 前置条件：
* check点：检查HandleWriteData返回值
*/
TEST_F(PosixServiceTaskTest, CheckHandleWriteData)
{
    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0120000;
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteData());

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0060000;
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteData());

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0020000;
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteData());

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0010000;
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteData());

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0001000;
    m_posixServiceTask->m_fileHandle.m_file->m_size = 1;
    m_posixServiceTask->m_params.blockSize = 2;
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteData());

    m_posixServiceTask->m_fileHandle.m_file->m_size = 2;
    m_posixServiceTask->m_params.blockSize = 1;
    m_posixServiceTask->m_fileHandle.m_file->dstIOHandle.posixFd = -1;
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteData());

    m_posixServiceTask->m_fileHandle.m_file->dstIOHandle.posixFd = 0;
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteData());

    m_posixServiceTask->m_fileHandle.m_block.m_size = 1;
    stub.set(pwrite, pwrite_Stub_1);
    EXPECT_NO_THROW(m_posixServiceTask->HandleWriteData());
    stub.reset(pwrite);
}

/*
* 用例名称：CheckShouldWriteMode
* 前置条件：
* check点：检查ShouldWriteMode返回值
*/
TEST_F(PosixServiceTaskTest, CheckShouldWriteMode)
{
    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0120000;
    EXPECT_EQ(m_posixServiceTask->ShouldWriteMode(), false);

    m_posixServiceTask->m_fileHandle.m_file->m_mode = 0010000;
    m_posixServiceTask->m_params.backupType = BackupType::BACKUP_FULL;
    m_posixServiceTask->m_params.zeroUmask = true;
    EXPECT_EQ(m_posixServiceTask->ShouldWriteMode(), false);

    m_posixServiceTask->m_params.zeroUmask = false;
    EXPECT_EQ(m_posixServiceTask->ShouldWriteMode(), true);
}

static int stat_Stub(const char *file_name, struct stat *buf)
{
    return 1;
}

/*
* 用例名称：CheckProcessWriteSpecialFileReplacePolicy
* 前置条件：
* check点：检查ProcessWriteSpecialFileReplacePolicy返回值
*/
TEST_F(PosixServiceTaskTest, CheckProcessWriteSpecialFileReplacePolicy)
{
    std::string dstFile = "111";
    std::string newDstFile = dstFile;
    bool isContinue = true;
    m_posixServiceTask->m_params.backupType = BackupType::BACKUP_INC;
    m_posixServiceTask->m_params.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSpecialFileReplacePolicy(dstFile, newDstFile, isContinue), SUCCESS);

    m_posixServiceTask->m_params.backupType = BackupType::UNKNOWN_TYPE;
    m_posixServiceTask->m_params.restoreReplacePolicy = RestoreReplacePolicy::IGNORE_EXIST;
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSpecialFileReplacePolicy(dstFile, newDstFile, isContinue), SUCCESS);

    m_posixServiceTask->m_params.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE_OLDER;
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSpecialFileReplacePolicy(dstFile, newDstFile, isContinue), FAILED);

    m_posixServiceTask->m_params.restoreReplacePolicy = RestoreReplacePolicy::RENAME;
    EXPECT_EQ(m_posixServiceTask->ProcessWriteSpecialFileReplacePolicy(dstFile, newDstFile, isContinue), SUCCESS);
}

/*
* 用例名称：CheckSetAcl
* 前置条件：
* check点：检查SetAcl返回值
*/
TEST_F(PosixServiceTaskTest, CheckSetAcl)
{
    std::string dstPath = "111";

    m_posixServiceTask->m_params.writeAcl = false;
    EXPECT_EQ(m_posixServiceTask->SetAcl(dstPath), true);

    m_posixServiceTask->m_params.writeAcl = true;
    m_posixServiceTask->m_fileHandle.m_file->m_aclText = "111";
    EXPECT_EQ(m_posixServiceTask->SetAcl(dstPath), false);

    m_posixServiceTask->m_fileHandle.m_file->m_aclText = "";
    m_posixServiceTask->m_fileHandle.m_file->m_defaultAclText = "111";
    EXPECT_EQ(m_posixServiceTask->SetAcl(dstPath), false);
}

/*
* 用例名称：CheckSetXattr
* 前置条件：
* check点：检查SetXattr返回值
*/
TEST_F(PosixServiceTaskTest, CheckSetXattr)
{
    std::string dstPath = "111";

    m_posixServiceTask->m_params.writeExtendAttribute = false;
    EXPECT_EQ(m_posixServiceTask->SetXattr(dstPath), true);

    m_posixServiceTask->m_params.writeExtendAttribute = true;
    m_posixServiceTask->m_fileHandle.m_file->m_xattr = {{"111", "222"}};
    EXPECT_EQ(m_posixServiceTask->SetXattr(dstPath), false);
}
