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
#include "Win32HardlinkWriter.h"
#include "OsPlatformDefines.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "Win32BackupEngineUtils.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;
using namespace Win32BackupEngineUtils;
 
Win32HardlinkWriter::Win32HardlinkWriter(const WriterParams &hardlinkWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : HostHardlinkWriter(hardlinkWriterParams, failureRecorder)
{
    INFOLOG("Construct Win32HardlinkWriter!");
}

Win32HardlinkWriter::~Win32HardlinkWriter()
{
    INFOLOG("Destruct Win32HardlinkWriter!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void Win32HardlinkWriter::ProcessWriteEntries(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetDstState();
    DBGLOG("process write entry %s state %d blockInfo %llu %llu %u", fileHandle.m_file->m_fileName.c_str(),
        (int)state, fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
 
    // 是小文件直接写数据， 提前退出
    if (fileHandle.m_file->m_size <= m_params.blockSize
        && state == FileDescState::INIT
        && !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, fileHandle)) {
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
        } else {
            InsertWriteCache(fileHandle);
        }
    }
    if (state == FileDescState::DST_OPENED ||
        state == FileDescState::PARTIAL_WRITED) {
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
 
void Win32HardlinkWriter::ProcessWriteData(FileHandle& fileHandle)
{
    fileHandle.m_file->SetDstState(FileDescState::PARTIAL_WRITED);
    fileHandle.m_file->m_blockStats.m_writeReqCnt++;
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    m_controlInfo->m_noOfBytesCopied += fileHandle.m_block.m_size;
    if ((fileHandle.m_file->m_size <= m_params.blockSize &&
        !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, fileHandle)) ||
        (fileHandle.m_file->m_blockStats.m_writeReqCnt == fileHandle.m_file->m_blockStats.m_totalCnt) ||
        (fileHandle.m_file->m_size == 0)) {
        m_hardlinkMap->SetTargetCopied(fileHandle.m_file->m_inode);
        DBGLOG("All blocks writed for %s, set target copied: %d", fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_file->m_inode);
        if (!m_params.writeMeta && (fileHandle.m_file->m_size <= m_params.blockSize &&
            !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, fileHandle))) {
            fileHandle.m_file->SetDstState(FileDescState::END);
            ++m_controlInfo->m_noOfFilesCopied;
        } else {
            fileHandle.m_file->SetDstState(FileDescState::WRITED);
            m_writeQueue->Push(fileHandle);
        }
    }
    return;
}
