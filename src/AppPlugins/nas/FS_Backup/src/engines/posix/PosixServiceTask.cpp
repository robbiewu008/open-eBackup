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
#include "PosixServiceTask.h"
#include <ctime>
#include <sys/acl.h>
#if defined(_AIX)||defined(SOLARIS)
#include <utime.h>
#else
#include <sys/xattr.h>
#endif
#include "log/Log.h"
#include "system/System.hpp"
#include "PosixUtils.h"
#include "HostMacros.h"

using namespace std;
using namespace FS_Backup;
using namespace PosixUtils;

namespace {
    const int SUCCESS = 0;
    const int FAILED = -1;
    const int NUM7 = 7;
}

void PosixServiceTask::SetCriticalErrorInfo(uint64_t err)
{
    if (err == ENOSPC) {
        m_backupFailReason = BackupPhaseStatus::FAILED_NOSPACE;
    }
    if (err == ESTALE) {
        m_backupFailReason = BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE;
    }
    return;
}

void PosixServiceTask::HandleOpenSrc()
{
    if (m_fileHandle.m_file->m_size <= m_params.blockSize) {
        DBGLOG("ignore open small file %s size %llu blockSize %u",
            m_fileHandle.m_file->m_fileName.c_str(), m_fileHandle.m_file->m_size, m_params.blockSize);
        m_result = SUCCESS;
        return;
    }
    std::string srcFile = PathConcat(m_params.srcRootPath, m_fileHandle.m_file->m_fileName, m_params.srcTrimPrefix);

    if (PosixUtils::IsSpecialFile(m_fileHandle.m_file->m_mode) != SpecialFileType::REG) {
        m_result = SUCCESS;
        return;
    }

    if (m_fileHandle.m_file->srcIOHandle.posixFd == -1) {
        m_fileHandle.m_file->srcIOHandle.posixFd = open(srcFile.c_str(), O_RDONLY);
        if (m_fileHandle.m_file->srcIOHandle.posixFd == -1) {
            m_errDetails = {srcFile, errno};
            ERRLOG("open failed %s errno %d", srcFile.c_str(), m_errDetails.second);
            m_result = FAILED;
            return;
        }
    }

    DBGLOG("open %s success!", srcFile.c_str());
    m_result = SUCCESS;
    return;
}

void PosixServiceTask::HandleOpenDst()
{
    if (m_fileHandle.m_file->m_size <= m_params.blockSize) {
        DBGLOG("ignore open small file %s size %llu blockSize %u",
            m_fileHandle.m_file->m_fileName.c_str(), m_fileHandle.m_file->m_size, m_params.blockSize);
        m_result = SUCCESS;
        return;
    }
    std::string dstFile = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    DBGLOG("open %s", dstFile.c_str());
    // special file , return
    if (PosixUtils::IsSpecialFile(m_fileHandle.m_file->m_mode) != SpecialFileType::REG) {
        DBGLOG("ignore special file %s", dstFile.c_str());
        m_result = SUCCESS;
        return;
    }

    m_result = ProcessOpenDst(dstFile);
    return;
}

int PosixServiceTask::ProcessOpenDst(const string& dstFile)
{
    uint flag = O_WRONLY | O_CREAT;
    if (m_params.backupType == BackupType::RESTORE || m_params.backupType == BackupType::FILE_LEVEL_RESTORE) {
        flag |= O_EXCL;
    } else {
        flag |= O_TRUNC;
    }
    if (m_fileHandle.m_file->dstIOHandle.posixFd != -1) {
        DBGLOG("already open %s", dstFile.c_str());
        return SUCCESS;
    }
    mode_t mode = m_params.writeMeta ? m_fileHandle.m_file->m_mode : 0666;
    DBGLOG("file %s mode %x", dstFile.c_str(), m_fileHandle.m_file->m_mode);
    m_fileHandle.m_file->dstIOHandle.posixFd = open(dstFile.c_str(), flag, mode);
    if (m_fileHandle.m_file->dstIOHandle.posixFd == -1) {
        CHECK_RETURN(errno == EEXIST, ProcessRestorePolicy(dstFile));
        if (errno == ENOENT) {
            string dstDir = PosixUtils::GetParentDirName(dstFile);
            CHECK_RETURN(!CreateDirectory(dstDir), FAILED);
            m_fileHandle.m_file->dstIOHandle.posixFd = open(dstFile.c_str(), flag, mode);
            if (m_fileHandle.m_file->dstIOHandle.posixFd != -1) {
                return ProcessOpenDstInteraction(dstFile);
            }
        }
        m_errDetails = {dstFile, errno};
        ERRLOG("open failed %s errno %d", dstFile.c_str(), m_errDetails.second);
        return FAILED;
    }
    if (m_params.writeSparseFile && (m_fileHandle.m_file->m_size > 0)) {
        int res = ftruncate(m_fileHandle.m_file->dstIOHandle.posixFd, m_fileHandle.m_file->m_size);
        if (res < 0) {
            m_errDetails = {dstFile, errno};
            ERRLOG("ftruncate failed %s errno %d", dstFile.c_str(), m_errDetails.second);
            return FAILED;
        }
    }
    DBGLOG("open %s success", dstFile.c_str());
    return SUCCESS;
}

int PosixServiceTask::ProcessOpenDstInteraction(const string &dstFile)
{
    DBGLOG("open file %s success", dstFile.c_str());
    if (m_fileHandle.m_file->m_size > 0) {
        int res = ftruncate(m_fileHandle.m_file->dstIOHandle.posixFd, m_fileHandle.m_file->m_size);
        if (res < 0) {
            m_errDetails = {dstFile, errno};
            ERRLOG("ftruncate failed %s errno %d", dstFile.c_str(), m_errDetails.second);
            return FAILED;
        }
    }
    return SUCCESS;
}

int PosixServiceTask::ProcessRestorePolicy(const string& dstFile)
{
    int flag = O_RDWR | O_CREAT | O_EXCL;
    DBGLOG("restore policy file %s", dstFile.c_str());
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::IGNORE_EXIST) {
        m_fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        DBGLOG("ignore exists file %s success!", dstFile.c_str());
        m_result = SUCCESS;
        return SUCCESS;
    }
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE_OLDER) {
        return ProcessOverwriteOlderPolicy(dstFile, flag);
    }
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE) {
        return ProcessOverwritePolicy(dstFile, flag);
    }
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::RENAME) {
        return ProcessOverrideRenamePolicy(dstFile, flag);
    }
    ERRLOG("invalid restore policy %d", (int)m_params.restoreReplacePolicy);
    return FAILED;
}

int PosixServiceTask::ProcessOverrideRenamePolicy(const string& dstFile, int flag)
{
    std::string newDstFile = PosixUtils::GetUnusedNewPath(dstFile);
    m_fileHandle.m_file->dstIOHandle.posixFd = ::open(newDstFile.c_str(), flag, m_fileHandle.m_file->m_mode);
    if (m_fileHandle.m_file->dstIOHandle.posixFd == -1) {
        m_errDetails = { dstFile, errno };
        ERRLOG("open renamed path failed %s errno %d, origin path: %s",
            newDstFile.c_str(), m_errDetails.second, dstFile.c_str());
        return FAILED;
    }
    if (m_fileHandle.m_file->m_size > 0) {
        int res = ftruncate(m_fileHandle.m_file->dstIOHandle.posixFd, m_fileHandle.m_file->m_size);
        if (res < 0) {
            m_errDetails = { dstFile, errno };
            ERRLOG("ftruncate failed %s errno %d", dstFile.c_str(), m_errDetails.second);
            return FAILED;
        }
    }
    DBGLOG("apply rename overite policy success, rename %s => %s", dstFile.c_str(), newDstFile.c_str());
    return SUCCESS;
}

int PosixServiceTask::ProcessOverwriteOlderPolicy(const string& dstFile, int flag)
{
    struct stat st;
    if (lstat(dstFile.c_str(), &st) != 0) {
        m_errDetails = {dstFile, errno};
        ERRLOG("stat failed %s errno %d", dstFile.c_str(), m_errDetails.second);
        return FAILED;
    }
    if (static_cast<uint64_t>(st.st_mtime) < m_fileHandle.m_file->m_mtime) {
        if (remove(dstFile.c_str()) != 0) {
            m_errDetails = {dstFile, errno};
            ERRLOG("remove failed : %s errno : %d", dstFile.c_str(), m_errDetails.second);
            return FAILED;
        }
        m_fileHandle.m_file->dstIOHandle.posixFd = open(dstFile.c_str(), flag, m_fileHandle.m_file->m_mode);
        if (m_fileHandle.m_file->dstIOHandle.posixFd == -1) {
            m_errDetails = {dstFile, errno};
            ERRLOG("open failed : %s errno : %d", dstFile.c_str(), m_errDetails.second);
            return FAILED;
        }
        if (m_fileHandle.m_file->m_size > 0) {
            int result = ftruncate(m_fileHandle.m_file->dstIOHandle.posixFd, m_fileHandle.m_file->m_size);
            if (result < 0) {
                m_errDetails = {dstFile, errno};
                ERRLOG("ftruncate failed : %s errno : %d", dstFile.c_str(), m_errDetails.second);
                return FAILED;
            }
        }
        DBGLOG("overwrite older %s success!", dstFile.c_str());
    } else {
        DBGLOG("skip %s success!", dstFile.c_str());
        m_fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        m_result = SUCCESS; // in order to enter success callback.
    }
    return SUCCESS;
}

int PosixServiceTask::ProcessOverwritePolicy(const string& dstFile, int flag)
{
    if (remove(dstFile.c_str()) != 0) {
        m_errDetails = {dstFile, errno};
        ERRLOG("remove failed %s errno %d", dstFile.c_str(), m_errDetails.second);
        return FAILED;
    }
    m_fileHandle.m_file->dstIOHandle.posixFd = open(dstFile.c_str(), flag, m_fileHandle.m_file->m_mode);
    if (m_fileHandle.m_file->dstIOHandle.posixFd == -1) {
        m_errDetails = {dstFile, errno};
        ERRLOG("open failed %s errno %d", dstFile.c_str(), m_errDetails.second);
        return FAILED;
    }
    if (m_fileHandle.m_file->m_size > 0) {
        int res = ftruncate(m_fileHandle.m_file->dstIOHandle.posixFd, m_fileHandle.m_file->m_size);
        if (res < 0) {
            m_errDetails = {dstFile, errno};
            ERRLOG("ftruncate failed %s errno %d", dstFile.c_str(), m_errDetails.second);
            return FAILED;
        }
    }
    DBGLOG("overwrite %s success!", dstFile.c_str());
    return SUCCESS;
}

int PosixServiceTask::ProcessReadSoftLinkData()
{
    std::string srcFile = PathConcat(m_params.srcRootPath, m_fileHandle.m_file->m_fileName, m_params.srcTrimPrefix);
    ssize_t cnt = readlink(srcFile.c_str(), (char *)m_fileHandle.m_block.m_buffer, m_fileHandle.m_block.m_size);
    if (cnt == -1) {
        m_errDetails = {srcFile, errno};
        SetCriticalErrorInfo(m_errDetails.second);
        ERRLOG("readlink failed %s errno %d", srcFile.c_str(), m_errDetails.second);
        return FAILED;
    }

    m_fileHandle.m_block.m_buffer[cnt] = '\0';
    DBGLOG("readlink %s success!", srcFile.c_str());
    return SUCCESS;
}

int PosixServiceTask::ProcessReadSpecialFileData()
{
    std::string srcFile = PathConcat(m_params.srcRootPath, m_fileHandle.m_file->m_fileName, m_params.srcTrimPrefix);
    return SUCCESS;
}

void PosixServiceTask::HandleReadSpecialData()
{
    if (S_ISLNK(m_fileHandle.m_file->m_mode)) {
        m_result = ProcessReadSoftLinkData();
        return;
    }

    if (S_ISBLK(m_fileHandle.m_file->m_mode) || S_ISCHR(m_fileHandle.m_file->m_mode) ||
        S_ISFIFO(m_fileHandle.m_file->m_mode)) {
        m_result = ProcessReadSpecialFileData();
        return;
    }

    if (m_fileHandle.m_file->IsFlagSet(HUGE_OBJECT_FILE)) {
        DBGLOG("This is huge object storage file %s", m_fileHandle.m_file->m_fileName.c_str());
        m_result = SUCCESS;
        return;
    }
}

void PosixServiceTask::HandleReadData()
{
    if (S_ISLNK(m_fileHandle.m_file->m_mode) || S_ISBLK(m_fileHandle.m_file->m_mode)
        || S_ISCHR(m_fileHandle.m_file->m_mode) || S_ISFIFO(m_fileHandle.m_file->m_mode)
        || m_fileHandle.m_file->IsFlagSet(HUGE_OBJECT_FILE)) {
        HandleReadSpecialData();
        return;
    }

    std::string srcFile = PathConcat(m_params.srcRootPath, m_fileHandle.m_file->m_fileName, m_params.srcTrimPrefix);
    if (m_fileHandle.m_file->m_size <= m_params.blockSize) {
        DBGLOG("open small file %s blockSize %d", srcFile.c_str(), m_params.blockSize);
        m_fileHandle.m_file->srcIOHandle.posixFd = open(srcFile.c_str(), O_RDONLY);
    }
    if (m_fileHandle.m_file->srcIOHandle.posixFd == -1) {
        m_errDetails = {srcFile, errno};
        ERRLOG("not opened %s errno %d", srcFile.c_str(), m_errDetails.second);
        m_result = FAILED;
        return;
    }
    ssize_t cnt = pread(m_fileHandle.m_file->srcIOHandle.posixFd, (void *)(m_fileHandle.m_block.m_buffer),
        m_fileHandle.m_block.m_size, m_fileHandle.m_block.m_offset);
    if (cnt != static_cast<ssize_t>(m_fileHandle.m_block.m_size)) {
        uint32_t errcode = (errno == 0 ? E_BACKUP_READ_LESS_THAN_EXPECTED : errno);
        m_errDetails = {srcFile, errcode};
        ERRLOG("pread failed %s, cnt %d, size %d, errno %d, err message %s",
            srcFile.c_str(), cnt, m_fileHandle.m_block.m_size, m_errDetails.second, strerror(m_errDetails.second));
        SetCriticalErrorInfo(m_errDetails.second);
        if (m_params.discardReadError) {
            m_fileHandle.m_file->SetFlag(READ_FAILED_DISCARD);
        } else {
            m_result = FAILED;
            return;
        }
    }
    DBGLOG("read %s blockInfo %llu %llu %u success!", srcFile.c_str(),
        m_fileHandle.m_block.m_seq, m_fileHandle.m_block.m_offset, m_fileHandle.m_block.m_size);
    m_result = SUCCESS;

    if (m_fileHandle.m_file->m_size <= m_params.blockSize) {
        close(m_fileHandle.m_file->srcIOHandle.posixFd);
        m_fileHandle.m_file->srcIOHandle.posixFd = -1;
        DBGLOG("close small file %s size %llu blockSize %d",
            m_fileHandle.m_file->m_fileName.c_str(), m_fileHandle.m_file->m_size, m_params.blockSize);
    }

    return;
}

void PosixServiceTask::HandleWriteData()
{
    if (S_ISLNK(m_fileHandle.m_file->m_mode)) {
        m_result = ProcessWriteSoftLinkData();
        return;
    }

    if (S_ISBLK(m_fileHandle.m_file->m_mode) ||
        S_ISCHR(m_fileHandle.m_file->m_mode) ||
        S_ISFIFO(m_fileHandle.m_file->m_mode)) {
        m_result = ProcessWriteSpecialFileData();
        return;
    }

    std::string dstFile = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    if (m_fileHandle.m_file->m_size <= m_params.blockSize) {
        DBGLOG("open small file %s size %llu blockSize %d",
            m_fileHandle.m_file->m_fileName.c_str(), m_fileHandle.m_file->m_size, m_params.blockSize);
        m_result = ProcessOpenDst(dstFile);
        if ((m_result == FAILED) || (m_fileHandle.m_file->GetDstState() == FileDescState::WRITE_SKIP)) {
            return;
        }
    }
    if (m_fileHandle.m_file->dstIOHandle.posixFd == -1) {
        m_errDetails = {dstFile, errno};
        ERRLOG("not opened %s errno %d", dstFile.c_str(), m_errDetails.second);
        m_result = FAILED;
        return;
    }
    ssize_t cnt = pwrite(m_fileHandle.m_file->dstIOHandle.posixFd, (const void *)(m_fileHandle.m_block.m_buffer),
        m_fileHandle.m_block.m_size, m_fileHandle.m_block.m_offset);
    if (cnt != static_cast<ssize_t>(m_fileHandle.m_block.m_size)) {
        m_errDetails = {dstFile, errno};
        ERRLOG("pwrite failed %s size %u cnt %d errno %d",
            dstFile.c_str(), m_fileHandle.m_block.m_size, cnt, m_errDetails.second);
        SetCriticalErrorInfo(m_errDetails.second);
        m_result = FAILED;
        return;
    }
    DBGLOG("write %s %d %llu %llu success!", dstFile.c_str(),
        m_fileHandle.m_block.m_seq, m_fileHandle.m_block.m_offset, m_fileHandle.m_block.m_size);
    m_result = SUCCESS;
    CloseSmallFileDstFd();
    return;
}

int PosixServiceTask::ProcessWriteSoftLinkData()
{
    std::string dstFile = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    DBGLOG("symlink %s", dstFile.c_str());
    int ret = symlink((const char *)m_fileHandle.m_block.m_buffer, dstFile.c_str());
    if (ret == -1) {
        if (errno == ENOENT) {
            string dstDir = PosixUtils::GetParentDirName(dstFile);
            if (!CreateDirectory(dstDir)) {
                return FAILED;
            }
            ret = symlink((const char *)m_fileHandle.m_block.m_buffer, dstFile.c_str());
            if (ret == 0) {
                DBGLOG("symlink %s success!", dstFile.c_str());
                return SUCCESS;
            }
        }
        if (errno == EEXIST) {
            bool isContinue = true;
            std::string newDstFile = dstFile;
            int applyResult = ProcessWriteSpecialFileReplacePolicy(dstFile, newDstFile, isContinue);
            if (!isContinue) {
                return applyResult;
            }
            ret = symlink((const char *)m_fileHandle.m_block.m_buffer, newDstFile.c_str());
            if (ret == 0) {
                DBGLOG("symlink %s success!", dstFile.c_str());
                return SUCCESS;
            }
        }
        m_errDetails = {dstFile, errno};
        SetCriticalErrorInfo(m_errDetails.second);
        ERRLOG("symlink failed %s errno %d", dstFile.c_str(), m_errDetails.second);
        return FAILED;
    }
    DBGLOG("symlink %s success!", dstFile.c_str());
    return SUCCESS;
}

int PosixServiceTask::ProcessWriteSpecialFileData()
{
    if ((m_params.backupType == BackupType::BACKUP_FULL || m_params.backupType == BackupType::BACKUP_INC) &&
        (S_ISBLK(m_fileHandle.m_file->m_mode) || S_ISCHR(m_fileHandle.m_file->m_mode))) {
        DBGLOG("skip special file %s", m_fileHandle.m_file->m_fileName.c_str());
        return SUCCESS;
    }
    std::string dstFile = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    DBGLOG("mknod %s", dstFile.c_str());
    int ret = mknod(dstFile.c_str(), m_fileHandle.m_file->m_mode, m_fileHandle.m_file->m_rdev);
    if (ret == -1) {
        if (errno == ENOENT) {
            string dstDir = PosixUtils::GetParentDirName(dstFile);
            if (!CreateDirectory(dstDir)) {
                return FAILED;
            }
            ret = mknod(dstFile.c_str(), m_fileHandle.m_file->m_mode, m_fileHandle.m_file->m_rdev);
            if (ret == 0) {
                DBGLOG("mknod %s success!", dstFile.c_str());
                return SUCCESS;
            }
        }
        if (errno == EEXIST) {
            DBGLOG("mknod %s already exist!", dstFile.c_str());
            bool isContinue = true;
            std::string newDstFile = dstFile;
            int applyResult = ProcessWriteSpecialFileReplacePolicy(dstFile, newDstFile, isContinue);
            if (!isContinue) {
                return applyResult;
            }
            ret = mknod(newDstFile.c_str(), m_fileHandle.m_file->m_mode, m_fileHandle.m_file->m_rdev);
            if (ret == 0) {
                DBGLOG("mknod %s success!", newDstFile.c_str());
                return SUCCESS;
            }
        }
        m_errDetails = { dstFile, errno };
        SetCriticalErrorInfo(m_errDetails.second);
        ERRLOG("mknod failed %s %hu %llu errno %d", dstFile.c_str(), m_fileHandle.m_file->m_mode,
            m_fileHandle.m_file->m_rdev, m_errDetails.second);
        return FAILED;
    }
    DBGLOG("mknod %s success!", dstFile.c_str());
    return SUCCESS;
}

void PosixServiceTask::HandleCloseSrc()
{
    if (m_fileHandle.m_file->m_size <= m_params.blockSize) {
        DBGLOG("ignore close small file %s size %llu blockSize %d",
            m_fileHandle.m_file->m_fileName.c_str(), m_fileHandle.m_file->m_size, m_params.blockSize);
        m_result = SUCCESS;
        return;
    }
    std::string srcFile = PathConcat(m_params.srcRootPath, m_fileHandle.m_file->m_fileName, m_params.srcTrimPrefix);
    if (m_fileHandle.m_file->srcIOHandle.posixFd != -1) {
        close(m_fileHandle.m_file->srcIOHandle.posixFd);
        m_fileHandle.m_file->srcIOHandle.posixFd = -1;
    }
    DBGLOG("close %s success!", m_fileHandle.m_file->m_fileName.c_str());
    m_result = SUCCESS;
    return;
}

void PosixServiceTask::HandleCloseDst()
{
    if (m_fileHandle.m_file->m_size <= m_params.blockSize) {
        DBGLOG("ignore close small file %s size %llu blockSize %d",
            m_fileHandle.m_file->m_fileName.c_str(), m_fileHandle.m_file->m_size, m_params.blockSize);
        m_result = SUCCESS;
        return;
    }
    std::string dstFile = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    if (m_fileHandle.m_file->dstIOHandle.posixFd != -1) {
        close(m_fileHandle.m_file->dstIOHandle.posixFd);
        m_fileHandle.m_file->dstIOHandle.posixFd = -1;
    }
    DBGLOG("close %s success!", m_fileHandle.m_file->m_fileName.c_str());
    m_result = SUCCESS;
    return;
}

void PosixServiceTask::HandleReadMeta()
{
    m_result = SUCCESS;
    return;
}

void PosixServiceTask::HandleWriteMeta()
{
    // write meta 备份时跳过特殊文件， 恢复时不跳过
    if (m_params.backupType == BackupType::BACKUP_FULL || m_params.backupType == BackupType::BACKUP_INC) {
        if (S_ISBLK(m_fileHandle.m_file->m_mode) || S_ISCHR(m_fileHandle.m_file->m_mode)) {
            DBGLOG("skip special file %s", m_fileHandle.m_file->m_fileName.c_str());
            m_result = SUCCESS;
            return;
        }
    }

    std::string dstPath = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    DBGLOG("write meta %s", dstPath.c_str());
    int ret = lchown(dstPath.c_str(), m_fileHandle.m_file->m_uid, m_fileHandle.m_file->m_gid);
    if (ret != 0) {
        m_errDetails = {dstPath, errno};
        ERRLOG("chown failed %s %u %u errno %d", dstPath.c_str(), m_fileHandle.m_file->m_uid,
            m_fileHandle.m_file->m_gid, m_errDetails.second);
        SetCriticalErrorInfo(m_errDetails.second);
        m_result = FAILED;
        return;
    }
    if (!SetUtime(dstPath)) {
        m_result = FAILED;
        return;
    }
    if (ShouldWriteMode()) {
        ret = chmod(dstPath.c_str(), m_fileHandle.m_file->m_mode);
        if (ret != 0) {
            m_errDetails = {dstPath, errno};
            ERRLOG("chmod failed %s %hu errno %d", dstPath.c_str(), m_fileHandle.m_file->m_mode, m_errDetails.second);
            m_result = FAILED;
            return;
        }
    }
    if (m_params.backupType == BackupType::RESTORE || m_params.backupType == BackupType::FILE_LEVEL_RESTORE) {
        if (!SetAcl(dstPath) || !SetXattr(dstPath)) {
            m_result = FAILED;
            return;
        }
    }
    DBGLOG("write meta %s success!", dstPath.c_str());
    m_result = SUCCESS;
    return;
}

void PosixServiceTask::HandleLink()
{
    std::string dstFile = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    std::string target = m_params.linkTarget;
    DBGLOG("link file %s target %s", dstFile.c_str(), target.c_str());
    int ret = link(target.c_str(), dstFile.c_str());
    if (ret < 0) {
        if (errno == EEXIST) {
            remove(dstFile.c_str()); // file can alse be existed when doing backup
            ret = link(target.c_str(), dstFile.c_str());
            if (ret == 0) {
                DBGLOG("link file %s target %s success!", dstFile.c_str(), target.c_str());
                m_result = SUCCESS;
                return;
            }
        }
        m_errDetails = {dstFile, errno};
        SetCriticalErrorInfo(m_errDetails.second);
        ERRLOG("link file failed %s target %s errno %d", dstFile.c_str(), target.c_str(), m_errDetails.second);
        m_result = FAILED;
        return;
    }
    DBGLOG("link file %s target %s success!", dstFile.c_str(), target.c_str());
    m_result = SUCCESS;
    return;
}

void PosixServiceTask::HandleDelete()
{
    std::string path = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    DBGLOG("delete file or dir %s ", path.c_str());
    bool removeRes = RemoveFile(path);
    if (!removeRes) {
        ERRLOG("failed to remove file: %s.", path.c_str());
        m_result = FAILED;
        return;
    }
    DBGLOG("delete file or dir %s success!", path.c_str());
    m_result = SUCCESS;
    return;
}

bool PosixServiceTask::RemoveFile(const std::string &filePath)
{
    boost::filesystem::path tmpPath(filePath);
    boost::system::error_code ec;
    try {
        if (!boost::filesystem::exists(tmpPath, ec)) {
            DBGLOG("file %s not exist.", filePath.c_str());
            return true;
        }
        if (ec.value() != boost::system::errc::success) {
            ERRLOG("boost::filesystem::exists() reported failure as: %s", ec.message().c_str());
            return false;
        }
        boost::uintmax_t cnt = boost::filesystem::remove_all(tmpPath, ec);
        if (ec.value() != boost::system::errc::success) {
            ERRLOG("delete file (%s) failed, reason: ", ec.message().c_str());
            return false;
        }
        INFOLOG("delete %d items.", cnt);
    } catch (const boost::filesystem::filesystem_error &e) {
        ERRLOG("boost::filesystem catch exeption: %s, path: %s.", e.code().message().c_str(), filePath.c_str());
        return false;
    }
    return true;
}

void PosixServiceTask::HandleCreateDir()
{
    std::string dstDir = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    if (!CreateDirectory(dstDir)) {
        m_result = FAILED;
        return;
    }
    DBGLOG("create dir %s success!", dstDir.c_str());
    if (m_params.backupType == BackupType::RESTORE || m_params.backupType == BackupType::FILE_LEVEL_RESTORE) {
        if (!SetAcl(dstDir) || !SetXattr(dstDir)) {
            ERRLOG("set acl or xattr failed for %s", dstDir.c_str());
        }
    }
    m_result = SUCCESS;
    return;
}

bool PosixServiceTask::SetUtime(const std::string &dstPath)
{
    DBGLOG("set time %s %llu %llu", dstPath.c_str(), m_fileHandle.m_file->m_atime, m_fileHandle.m_file->m_mtime);
#if defined(_AIX) || defined(SOLARIS)
    struct stat st;
    if (lstat(dstPath.c_str(), &st) == SUCCESS && S_ISLNK(st.st_mode)) {
        DBGLOG("file: %s is a soft link, do not support set utimes!", dstPath.c_str());
        return true;
    }
    struct utimbuf times;
    times.actime = m_fileHandle.m_file->m_atime;
    times.modtime = m_fileHandle.m_file->m_mtime;
    if (utime(dstPath.c_str(), &times) < 0) {
#else
    struct timeval times[2];
    times[0].tv_sec = m_fileHandle.m_file->m_atime;
    times[0].tv_usec = 0;
    times[1].tv_sec = m_fileHandle.m_file->m_mtime;
    times[1].tv_usec = 0;
    if (lutimes(dstPath.c_str(), times) < 0) {
#endif
        m_errDetails = {dstPath, errno};
        ERRLOG("lutimes failed %s errno %d", dstPath.c_str(), m_errDetails.second);
        return false;
    }
    return true;
}

bool PosixServiceTask::SetAcl(const string &dstPath)
{
#ifdef _AIX
    return SetAcl4Aix(dstPath);
#elif defined(SOLARIS)
    return SetAcl4Solaris(dstPath);
#else
    return SetAcl4Posix(dstPath);
#endif
}

#if !defined(_AIX) && !defined(SOLARIS)
bool PosixServiceTask::SetAcl4Posix(const std::string &dstPath) {
    if (m_params.writeAcl && !m_fileHandle.m_file->m_aclText.empty()) {
        acl_t acl = acl_from_text(m_fileHandle.m_file->m_aclText.c_str());
        if (acl == nullptr) {
            m_errDetails = {dstPath, errno};
            ERRLOG("format acl from text %s failed %s errno %d",
                m_fileHandle.m_file->m_aclText.c_str(), dstPath.c_str(), m_errDetails.second);
            return false;
        }
        if (acl_set_file(dstPath.c_str(), ACL_TYPE_ACCESS, acl) != 0) {
            m_errDetails = {dstPath, errno};
            ERRLOG("set acl %s failed %s errno %d",
                m_fileHandle.m_file->m_aclText.c_str(), dstPath.c_str(), m_errDetails.second);
            return false;
        }
        if (acl_free((void*)acl) != 0) {
            m_errDetails = {dstPath, errno};
            ERRLOG("free acl %s failed %s errno %d",
                m_fileHandle.m_file->m_aclText.c_str(), dstPath.c_str(), m_errDetails.second);
            return false;
        }
    }
    if (m_params.writeAcl && !m_fileHandle.m_file->m_defaultAclText.empty()) {
        acl_t acl = acl_from_text(m_fileHandle.m_file->m_defaultAclText.c_str());
        if (acl == nullptr) {
            m_errDetails = {dstPath, errno};
            ERRLOG("format acl from text %s failed %s errno %d",
                m_fileHandle.m_file->m_defaultAclText.c_str(), dstPath.c_str(), m_errDetails.second);
            return false;
        }
        if (acl_set_file(dstPath.c_str(), ACL_TYPE_DEFAULT, acl) != 0) {
            m_errDetails = {dstPath, errno};
            ERRLOG("set acl %s failed %s errno %d",
                m_fileHandle.m_file->m_defaultAclText.c_str(), dstPath.c_str(), m_errDetails.second);
            return false;
        }
        if (acl_free((void*)acl) != 0) {
            m_errDetails = {dstPath, errno};
            ERRLOG("free acl %s failed %s errno %d",
                m_fileHandle.m_file->m_defaultAclText.c_str(), dstPath.c_str(), m_errDetails.second);
            return false;
        }
    }
    return true;
}
#endif

#ifdef _AIX
bool PosixServiceTask::SetAcl4Aix(const std::string &dstPath)
{
    if (m_params.writeAcl && !m_fileHandle.m_file->m_aclText.empty()) {
        char acl[MAX_ACL_SIZE];
        size_t aclSize = sizeof(acl);
        acl_type_t type;
        type.u64 = ACL_ANY;

        // 解析type.u64 以及 aclText
        auto position = m_fileHandle.m_file->m_aclText.find("aixacl:");
        if (position == std::string::npos) {
            ERRLOG("format acl from text failed for text is empty, failed %s", dstPath.c_str());
            return false;
        }
        string aclText = m_fileHandle.m_file->m_aclText.substr(position + NUM7);
        type.u64 = stoull(m_fileHandle.m_file->m_aclText.substr(0, position));

        if (aclx_scanStr(const_cast<char*>(aclText.c_str()), acl, &aclSize, type) < 0) {
            m_errDetails = {dstPath, errno};
            ERRLOG("Failed to get acl from aclText %s failed %s errno %d",
                aclText.c_str(), dstPath.c_str(), m_errDetails.second);
            return false;
        }

        // 使用path保存dstPath值，防止系统接口改变dstPath的值
        string path = dstPath;
        if (aclx_put(const_cast<char*>(path.c_str()), SET_ACL, type, acl, aclSize, 0) < 0) {
            m_errDetails = {dstPath, errno};
            ERRLOG("set acl %s failed %s errno %d",
                aclText.c_str(), dstPath.c_str(), m_errDetails.second);
            return false;
        }
    }
    return true;
}
#endif

#ifdef SOLARIS
bool PosixServiceTask::SetAcl4Solaris(const std::string &dstPath)
{
    INFOLOG("Enter SetAcl4Posix: %s, %s, %s", dstPath.c_str(), m_fileHandle.m_file->m_fileName.c_str(),
        m_fileHandle.m_file->m_aclText.c_str());
    if (m_params.writeAcl && !m_fileHandle.m_file->m_aclText.empty()) {
        acl_t *aclp = nullptr;
        if (acl_fromtext(m_fileHandle.m_file->m_aclText.c_str(), &aclp) != 0) {
            m_errDetails = {dstPath, errno};
            ERRLOG("format acl from text %s failed %s errno %d",
                m_fileHandle.m_file->m_aclText.c_str(),
                dstPath.c_str(),
                m_errDetails.second);
            return false;
        }
        if (acl_set(dstPath.c_str(), aclp) < 0) {
            m_errDetails = {dstPath, errno};
            ERRLOG("set acl %s failed %s errno %d",
                m_fileHandle.m_file->m_aclText.c_str(),
                dstPath.c_str(),
                m_errDetails.second);
                acl_free(aclp);
            return false;
        }
        acl_free(aclp);  // SOLARIS中该函数无返回值
    }
    return true;
}
#endif

bool PosixServiceTask::SetXattr(const string &dstPath)
{
#ifdef _AIX
    DBGLOG("Setxattr function is missing on AIX platform!");
#elif defined(SOLARIS)
    DBGLOG("Setxattr function is missing on SunOS platform!");
#else
    if (m_params.writeExtendAttribute && !m_fileHandle.m_file->m_xattr.empty()) {
        for (auto &xattr : m_fileHandle.m_file->m_xattr) {
            char *value = (char *)malloc(xattr.second.size());
            if (value == nullptr) {
                ERRLOG("malloc failed");
                return false;
            }
            memcpy_s(value, xattr.second.size(), xattr.second.c_str(), xattr.second.size());
            for (size_t i = 0; i < xattr.second.size(); ++i) {
                if (static_cast<unsigned char>(value[i]) == 0xFF) {
                    value[i] = 0;
                }
            }
            int ret = lsetxattr(dstPath.c_str(), xattr.first.c_str(), value, xattr.second.size(), XATTR_CREATE);
            if (ret == 0) {
                free(value);
                continue;
            }
            if (errno != EEXIST) {
                m_errDetails = {dstPath, errno};
                ERRLOG("set xattr key %s value %s failed %s errno %d",
                    xattr.first.c_str(), xattr.second.c_str(), dstPath.c_str(), m_errDetails.second);
                free(value);
                return false;
            }
            ret = lsetxattr(dstPath.c_str(), xattr.first.c_str(), value, xattr.second.size(), XATTR_REPLACE);
            if (ret != 0) {
                m_errDetails = {dstPath, errno};
                ERRLOG("set xattr key %s value %s failed %s errno %d",
                    xattr.first.c_str(), xattr.second.c_str(), dstPath.c_str(), m_errDetails.second);
                free(value);
                return false;
            }
            free(value);
        }
    }
#endif
    return true;
}

bool PosixServiceTask::ShouldWriteMode()
{
    if (S_ISLNK(m_fileHandle.m_file->m_mode)) {
        return false;
    }
    if ((m_params.backupType == BackupType::BACKUP_FULL) && m_params.zeroUmask) {
        return false;
    }
    return true;
}

int PosixServiceTask::ProcessWriteSpecialFileReplacePolicy(
    const std::string& dstFile, std::string& newDstFile, bool &isContinue)
{
    isContinue = true;
    if (m_params.backupType == BackupType::BACKUP_INC ||
        m_params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE) {
        remove(dstFile.c_str());
    }
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::IGNORE_EXIST) {
        m_fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        DBGLOG("ignore exists file %s success!", dstFile.c_str());
        isContinue = false;
        return SUCCESS;
    }
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE_OLDER) {
        struct stat st;
        if (lstat(dstFile.c_str(), &st) != 0) {
            m_errDetails = {dstFile, errno};
            ERRLOG("stat failed %s errno %d", dstFile.c_str(), m_errDetails.second);
            isContinue = false;
            return FAILED;
        }
        if (static_cast<uint64_t>(st.st_mtime) >= m_fileHandle.m_file->m_mtime) {
            m_fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
            DBGLOG("skip file %s!", dstFile.c_str());
            isContinue = false;
            return SUCCESS;
        }
        remove(dstFile.c_str());
        return SUCCESS;
    }
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::RENAME) {
        newDstFile = PosixUtils::GetUnusedNewPath(dstFile);
        DBGLOG("create special file override policy: rename %s => %s", dstFile.c_str(), newDstFile.c_str());
        isContinue = true;
        return SUCCESS;
    }
    return SUCCESS;
}

void PosixServiceTask::CloseSmallFileDstFd()
{
    if (m_fileHandle.m_file->m_size <= m_params.blockSize) {
        close(m_fileHandle.m_file->dstIOHandle.posixFd);
        m_fileHandle.m_file->dstIOHandle.posixFd = -1;
        DBGLOG("close small file %s size %llu blockSize %d",
            m_fileHandle.m_file->m_fileName.c_str(), m_fileHandle.m_file->m_size, m_params.blockSize);
    }
}

bool PosixServiceTask::CreateDirectory(const std::string &path)
{
    DBGLOG("create dir %s", path.c_str());
    int err = PosixUtils::RecurseCreateDirectoryWithErr(path);
    if (err != 0) {
        m_errDetails = {path, err};
        SetCriticalErrorInfo(m_errDetails.second);
        ERRLOG("recurse create directory failed %s errno %d", path.c_str(), err);
        return false;
    }
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) {
        m_errDetails = {path, errno};
        SetCriticalErrorInfo(m_errDetails.second);
        ERRLOG("recurse create dir failed %s errno %d", path.c_str(), m_errDetails.second);
        return false;
    }
    if (!S_ISDIR(st.st_mode)) {
        ERRLOG("recurse create dir failed %s,not a directory", path.c_str());
        return false;
    }
    return true;
}
