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
#include "Win32HardlinkReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "Win32BackupEngineUtils.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;
using namespace Win32BackupEngineUtils;

Win32HardlinkReader::Win32HardlinkReader(const ReaderParams &hardlinkReaderParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : HostHardlinkReader(hardlinkReaderParams, failureRecorder)
{
    INFOLOG("Construct Win32HardlinkReader!");
}

Win32HardlinkReader::~Win32HardlinkReader()
{
    INFOLOG("Destruct Win32HardlinkReader!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
}

int Win32HardlinkReader::ReadEmptyData(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadData: %s", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
    ++m_controlInfo->m_noOfFilesRead;
    PushToAggregator(fileHandle);
    DBGLOG("%s is empty file, needn't read it! readed for now: %d", fileHandle.m_file->m_fileName.c_str(),
        m_controlInfo->m_noOfFilesRead.load());
    return SUCCESS;
}
 
int Win32HardlinkReader::ReadSymlinkData(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadData: %s", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_file->m_size + 1];
    fileHandle.m_block.m_size = fileHandle.m_file->m_size;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    DBGLOG("total blocks: %d, file size: %d",
        fileHandle.m_file->m_blockStats.m_totalCnt.load(), fileHandle.m_file->m_size);
    
    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::READ_DATA, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        ERRLOG("put read file task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_readTaskProduce;
    DBGLOG("total readTask produce for now: %d", m_readTaskProduce.load());
    return SUCCESS;
}
 
int Win32HardlinkReader::ReadNormalData(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadData: %s block info: %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
 
    fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_block.m_size];
 
    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::READ_DATA, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        ERRLOG("put read file task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_readTaskProduce;
 
    DBGLOG("total readTask produce for now: %d", m_readTaskProduce.load());
    return SUCCESS;
}
 
void Win32HardlinkReader::ProcessReadEntries(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetSrcState();
    DBGLOG("process read entry %s state %d blockInfo %llu %llu %u", fileHandle.m_file->m_fileName.c_str(),
        (int)state, fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);

    // 是小文件直接读数据， 提前结束
    if (fileHandle.m_file->m_size <= m_params.blockSize &&
        !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, fileHandle) &&
        state != FileDescState::LINK)  {
        fileHandle.m_block.m_size = fileHandle.m_file->m_size;
        fileHandle.m_block.m_offset = 0;
        fileHandle.m_block.m_seq = 1;
        fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);
        ReadData(fileHandle);
        return;
    }
 
    if (state == FileDescState::INIT) {
        OpenFile(fileHandle);
    }
    if (state == FileDescState::SRC_OPENED) {
        ReadData(fileHandle);
    }
    if ((state == FileDescState::READED) || (state == FileDescState::META_READED)) {
        CloseFile(fileHandle);
    }
    if (state == FileDescState::LINK) {
        ++m_controlInfo->m_noOfFilesRead;
        DBGLOG("HardlinkReader pass link %s to aggregate", fileHandle.m_file->m_fileName.c_str());
        fileHandle.m_file->SetDstState(FileDescState::LINK);
        PushToAggregator(fileHandle);
    }
 
    return;
}
 
void Win32HardlinkReader::HandleSuccessEvent(shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    FileDescState state = fileHandle.m_file->GetSrcState();
    DBGLOG("Win32 hardlink reader success %s event %d state %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(event), static_cast<int>(state));
    if (fileHandle.m_file->IsFlagSet(READ_FAILED_DISCARD)) {
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, taskPtr->m_errDetails);
    }
    // 小文件readdata后已经close了， 直接push给aggregator
    if (fileHandle.m_file->m_size <= m_params.blockSize &&
        !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, fileHandle)) {
        fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
        ++m_controlInfo->m_noOfFilesRead;
        PushToAggregator(fileHandle);
        DBGLOG("Readed small files : %s, readed for now: %d", fileHandle.m_file->m_fileName.c_str(),
            m_controlInfo->m_noOfFilesRead.load());
        return;
    }
 
    if (event == HostEvent::CLOSE_SRC) {
        ++m_controlInfo->m_noOfFilesRead;
        DBGLOG("Readed Files for now : %d", m_controlInfo->m_noOfFilesRead.load());
        return;
    }
 
    if (event == HostEvent::OPEN_SRC) {
        fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);
        PushToAggregator(fileHandle); // push to aggregate to write to open dst
        PushToReader(fileHandle); // decompose to blocks and push to aggregate
        return;
    }
 
    if (event == HostEvent::READ_DATA) {
        fileHandle.m_file->m_blockStats.m_readReqCnt++;
        if (fileHandle.m_file->m_blockStats.m_totalCnt == fileHandle.m_file->m_blockStats.m_readReqCnt ||
            fileHandle.m_file->m_size == 0) {
            fileHandle.m_file->SetSrcState(FileDescState::READED);
            PushToReader(fileHandle); // push to aggregate
        }
        PushToAggregator(fileHandle); // file handle with data block, push to aggregate to write to write data
        return;
    }
 
    return;
}