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
#include "PosixConstants.h"
#include <sys/stat.h>
#include "PosixCopyWriter.h"
#include "PosixServiceTask.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "PosixUtils.h"
#include "FSBackupUtils.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int RETRY_TIME_MILLISENCOND = 1000;
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
}

PosixCopyWriter::PosixCopyWriter(
    const WriterParams &copyWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : HostCopyWriter(copyWriterParams, failureRecorder)
{
    INFOLOG("Construct PosixCopyWriter!");
    m_params.zeroUmask = (FSBackupUtils::GetUmask() == 0);
}

PosixCopyWriter::~PosixCopyWriter()
{
    INFOLOG("Destruct PosixCopyWriter!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    CloseOpenedHandle();
}

int PosixCopyWriter::WriteMeta(FileHandle& fileHandle)
{
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::WRITE_META, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put write meta file task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_writeTaskProduce;
    return SUCCESS;
}

// Hint:: need to add 'RecordFailureDetail' and unify later
void PosixCopyWriter::ProcessWriteEntries(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetDstState();

    DBGLOG("process write entry %s state %d blockInfo %llu %llu %u scannermode %s",
        fileHandle.m_file->m_fileName.c_str(), (int)state,
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size,
        fileHandle.m_file->m_scannermode.c_str());
    if (S_ISDIR(fileHandle.m_file->m_mode)) {
        CreateDir(fileHandle);
        return;
    }

    // 对于meta_modified的文件，在writer的路径只有INIT->WRITEMETA->END
    if ((m_params.backupType == BackupType::BACKUP_FULL || m_params.backupType == BackupType::BACKUP_INC) &&
        FSBackupUtils::IsHandleMetaModified(fileHandle.m_file->m_scannermode,
        m_backupParams.commonParams.backupDataFormat)) {
        WriteMeta(fileHandle);
        return;
    }

    // 是小文件直接写数据, 提前退出, 第二个判断是只处理一次
    if (fileHandle.m_file->m_size <= m_params.blockSize && state == FileDescState::INIT) {
        // 对于小文件， 意义为open 的block直接丢弃
        if (IsOpenBlock(fileHandle)) {
            return;
        }
        fileHandle.m_file->SetDstState(FileDescState::DST_OPENED);
        WriteData(fileHandle);
        return;
    }

    if (state == FileDescState::INIT) {
        if (IsOpenBlock(fileHandle)) {
            OpenFile(fileHandle);
            return;
        }
        InsertWriteCache(fileHandle);
    }

    if ((state == FileDescState::DST_OPENED) || (state == FileDescState::PARTIAL_WRITED)) {
        WriteData(fileHandle);
    }
    if (state == FileDescState::WRITED) {
        CloseFile(fileHandle);
    }
    if (state == FileDescState::DST_CLOSED) {
        WriteMeta(fileHandle);
    }
    if (state == FileDescState::WRITE_FAILED || state == FileDescState::WRITE_SKIP || state == FileDescState::END) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    }

    return;
}

// Hint:: need to check sparse handle logic according to Win32CopyWriter to unify later
void PosixCopyWriter::ProcessWriteData(FileHandle& fileHandle)
{
    fileHandle.m_file->SetDstState(FileDescState::PARTIAL_WRITED);
    ++fileHandle.m_file->m_blockStats.m_writeReqCnt;
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    m_controlInfo->m_noOfBytesCopied += fileHandle.m_block.m_size;
    if ((fileHandle.m_file->m_size <= m_params.blockSize) ||
        (fileHandle.m_file->m_blockStats.m_writeReqCnt == fileHandle.m_file->m_blockStats.m_totalCnt) ||
        (fileHandle.m_file->m_size == 0)) {
        DBGLOG("All blocks writed for %s", fileHandle.m_file->m_fileName.c_str());
        if (!m_params.writeMeta && (fileHandle.m_file->m_size <= m_params.blockSize)) {
            fileHandle.m_file->SetDstState(FileDescState::END);
            m_controlInfo->m_noOfFilesCopied += fileHandle.m_file->m_originalFileCount;
        } else {
            fileHandle.m_file->SetDstState(FileDescState::WRITED);

            m_writeQueue->Push(fileHandle);
        }
    }
    return;
}

void PosixCopyWriter::CloseOpenedHandle()
{
    for (auto it = m_dstOpenedHandleSet.begin(); it != m_dstOpenedHandleSet.end(); it++) {
        DBGLOG("Close handle(%s)", (*it)->m_fileName.c_str());
        if ((*it)->dstIOHandle.posixFd != -1) {
            close((*it)->dstIOHandle.posixFd);
            (*it)->dstIOHandle.posixFd = -1;
        }
    }
}
