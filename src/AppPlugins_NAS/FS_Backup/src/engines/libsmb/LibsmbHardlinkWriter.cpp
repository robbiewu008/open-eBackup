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
#include "LibsmbHardlinkWriter.h"
#include <sys/stat.h>
#include "Libsmb.h"
#include "log/Log.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    constexpr int RECONNECT_CONTEXT_RETRY_TIMES = 5;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
    const int PENDING_PACKET_REACH_THRESHOLD_TIMER_MILLISECOND = 5000;
    const int PENDING_PACKET_REACH_THRESHOLD_SLEEP_SECOND = 1;
    constexpr int DEFAULT_POLL_EXPIRED_TIME = 100;
}

LibsmbHardlinkWriter::LibsmbHardlinkWriter(const WriterParams &hardlinkWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : WriterBase(hardlinkWriterParams)
{
    INFOLOG("Construct LibsmbHardlinkWriter!");
    m_failureRecorder = failureRecorder;
    m_dstAdvParams = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.dstAdvParams);
    FillContextParams(m_params.dstSmbContextArgs, m_dstAdvParams);
    INFOLOG("serverCheckMaxCount: %d!", m_dstAdvParams->serverCheckMaxCount);
    m_pktStats = make_shared<PacketStats>();

    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.writeMeta = m_backupParams.commonParams.writeMeta;

    m_params.dstRootPath = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.dstAdvParams)->rootPath;
}

LibsmbHardlinkWriter::~LibsmbHardlinkWriter()
{
    INFOLOG("Destruct LibsmbHardlinkWriter!");
    if (m_syncThread.joinable()) {
        m_syncThread.join();
    }
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

int LibsmbHardlinkWriter::SmbConnectContexts()
{
    m_asyncContext = SmbConnectContext(m_params.dstSmbContextArgs);
    if (m_asyncContext == nullptr) {
        ERRLOG("Smb connect contexts failed!");
        return FAILED;
    }
    m_syncContext = SmbConnectContext(m_params.dstSmbContextArgs);
    if (m_syncContext == nullptr) {
        ERRLOG("Smb connect contexts failed!");
        return FAILED;
    }
    return SUCCESS;
}

void LibsmbHardlinkWriter::SmbDisconnectContexts()
{
    INFOLOG("LibsmbHardlinkWriter SmbDisconnectContexts");
    SmbDisconnectContext(m_asyncContext);
}

void LibsmbHardlinkWriter::SmbDisconnectSyncContexts()
{
    INFOLOG("LibsmbHardlinkWriter SmbDisconnectSyncContexts");
    SmbDisconnectContext(m_syncContext);
}

BackupRetCode LibsmbHardlinkWriter::Start()
{
    INFOLOG("LibsmbHardlinkWriter call Start!");

    BackupQueueConfig config;
    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    m_dirQueue = make_shared<BackupQueue<FileHandle>>(config);

    if (SmbConnectContexts() != SUCCESS) {
        return BackupRetCode::FAILED;
    }
    try {
        m_syncThread = thread(&LibsmbHardlinkWriter::SyncThreadFunc, this);
        m_thread = thread(&LibsmbHardlinkWriter::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        SmbDisconnectContexts();
        SmbDisconnectSyncContexts();
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        SmbDisconnectContexts();
        SmbDisconnectSyncContexts();
        return BackupRetCode::FAILED;
    }

    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbHardlinkWriter::Abort()
{
    INFOLOG("LibsmbHardlinkWriter Enter Abort");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbHardlinkWriter::Destroy()
{
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibsmbHardlinkWriter::GetStatus()
{
    m_pktStats->Print();
    return FSBackupUtils::GetWriterStatus(m_controlInfo, m_abort, BackupPhaseStatus::FAILED);
}

bool LibsmbHardlinkWriter::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

bool LibsmbHardlinkWriter::IsComplete()
{
    /* in write phase, faild item should contain not only write failed but also read failed */
    /* thus, use BackupControlInfo::m_noOfFilesFailed and BackupControlInfo::m_noOfDirFailed */
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("aggrComplete %d "
            "(pending packet counts %d, writeQueueSize %d, dirQueueSize %d, writeCacheSize %d) "
            "(m_noOfFilesCopied %d "
            "skipFiles %d  backupFailedFiles %d) "
            "(totalFiles %d)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING),
            m_writeQueue->GetSize(), m_dirQueue->GetSize(), m_writeCache.size(),
            m_controlInfo->m_noOfFilesCopied.load(),
            m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_noOfFilesFailed.load(),
            m_controlInfo->m_noOfFilesToBackup.load());
    }

    if ((m_controlInfo->m_aggregatePhaseComplete) &&
        (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) == 0) &&
        m_writeQueue->Empty() &&
        m_dirQueue->Empty() &&
        (m_timer.GetCount() == 0) &&
        (m_writeCache.size() == 0)) {
        INFOLOG("aggrComplete %d "
            "(pending packet counts %d, writeQueueSize %d, dirQueueSize %d, writeCacheSize %d) "
            "(m_noOfFilesCopied %d "
            "skipFiles %d  backupFailedFiles %d) "
            "(totalFiles %d)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING),
            m_writeQueue->GetSize(), m_dirQueue->GetSize(), m_writeCache.size(),
            m_controlInfo->m_noOfFilesCopied.load(),
            m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_noOfFilesFailed.load(),
            m_controlInfo->m_noOfFilesToBackup.load());
        return true;
    }

    return false;
}

bool LibsmbHardlinkWriter::IsMkdirComplete() const
{
    if (m_controlInfo->m_writePhaseComplete && m_dirQueue->Empty()) {
        DBGLOG("CopyWriter mkdir thread complete!");
        return true;
    }
    return false;
}

void LibsmbHardlinkWriter::HandleComplete()
{
    INFOLOG("Complete LibsmbHardlinkWriter");
    m_controlInfo->m_writePhaseComplete = true;
}

int LibsmbHardlinkWriter::OpenFile(FileHandle &fileHandle)
{
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::OPEN_DST) != SUCCESS) {
        return FAILED;
    }
    DBGLOG("Enter OpenFile:%s", fileHandle.m_file->m_fileName.c_str());
    return SUCCESS;
}

int LibsmbHardlinkWriter::WriteMeta(FileHandle &fileHandle)
{
    // 对于文件，接收到setsd的响应后，会继续发setbasicinfo请求
    // 对于目录，setbasicinfo请求会在dirmtime阶段再发
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::SET_SD) != SUCCESS) {
        return FAILED;
    }

    vector<FileHandle> linkFileHandles = m_hardlinkMap->GetLinks(fileHandle.m_file->m_inode);
    bool isDir = fileHandle.m_file->IsFlagSet(IS_DIR);
    DBGLOG("WriteMeta name: %s, %d, %d, links = %d", fileHandle.m_file->m_fileName.c_str(),
        isDir, fileHandle.m_file->m_mode, linkFileHandles.size());
    for (auto linkFileHandle : linkFileHandles) {
        string targetPath;
        int ret = m_hardlinkMap->GetTargetPath(fileHandle.m_file->m_inode, targetPath);
        if (ret != SUCCESS) {
            ERRLOG("not found hardlink target path for inode: %d", fileHandle.m_file->m_inode);
            continue;
        }
        auto hardLinkCbData = GetSmbWriterCommonData(linkFileHandle);
        if (hardLinkCbData == nullptr) {
            return FAILED;
        }
        hardLinkCbData->linkTargetPath = targetPath;
        SendWriterRequest(linkFileHandle, hardLinkCbData, LibsmbEvent::LINK);

        DBGLOG("found target path: %s for file %s inode %d",
            targetPath.c_str(), fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_inode);
    }
    return SUCCESS;
}

int LibsmbHardlinkWriter::WriteData(FileHandle &fileHandle)
{
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::WRITE) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int LibsmbHardlinkWriter::CloseFile(FileHandle &fileHandle)
{
    if (fileHandle.m_file->dstIOHandle.smbFh == nullptr) {
        DBGLOG("OpenFile, dstIoHandle is nullptr:%s", fileHandle.m_file->m_fileName.c_str());
        return SUCCESS;
    }
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::CLOSE_DST) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int LibsmbHardlinkWriter::DeleteFile(FileHandle &fileHandle)
{
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::UNLINK) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int LibsmbHardlinkWriter::ServerCheck()
{
    /* If max no-space pending count is reached, abort the job */
    if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::NO_SPACE_ERR) >= DEFAULT_MAX_NOSPACE ||
        m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::NO_ACCESS_ERR) >= DEFAULT_MAX_NOACCESS) {
        if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::NO_SPACE_ERR) >= DEFAULT_MAX_NOSPACE) {
            ERRLOG("Threshold reached for DEFAULT_MAX_NOSPACE");
            m_failReason = BackupPhaseStatus::FAILED_NOSPACE;
        } else {
            ERRLOG("Threshold reached for DEFAULT_MAX_NOACCESS");
            m_failReason = BackupPhaseStatus::FAILED_NOACCESS;
        }
        m_failed = true;
        return FAILED;
    }
    /* Nas Server Check for Destination side */
    if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR) >= m_dstAdvParams->serverCheckMaxCount) {
        ERRLOG("Threshold reached calling dst servercheck");
        m_suspend = true;
        int ret = HandleConnectionException(m_asyncContext, m_params.dstSmbContextArgs, RECONNECT_CONTEXT_RETRY_TIMES);
        if (ret != SUCCESS) {
            ERRLOG("Stop and Abort read phase due to server inaccessible");
            m_failed = true;
            m_suspend = false;
            FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason, false);
            return FAILED;
        } else {
            INFOLOG("Server reachable");
            m_pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);
        }
        m_suspend = false;
    }

    return SUCCESS;
}

int64_t LibsmbHardlinkWriter::ProcessTimers()
{
    vector<FileHandle> fileHandles;
    int expiredCount =
        m_dstAdvParams->maxPendingAsyncReqCnt - m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING);
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles, expiredCount);
    if (delay > POLL_MAX_TIMEOUT) {
        delay = POLL_MAX_TIMEOUT;
    }
    for (FileHandle& fh : fileHandles) {
        if (IsWriterRequestReachThreshold()) {
            m_timer.Insert(fh, PENDING_PACKET_REACH_THRESHOLD_TIMER_MILLISECOND);
            continue;
        }
        DBGLOG("Process timer %s", fh.m_file->m_fileName.c_str());
        // 从timer里取出来的LINK状态是要覆盖的硬链接
        if (fh.m_file->GetDstState() == FileDescState::LINK) {
            LinkFile(fh);
        } else {
            ProcessFileDescState(fh);
        }
    }
    return delay;
}

void LibsmbHardlinkWriter::ProcessHardlinkMap()
{
    vector<vector<FileHandle>> fileHandles = m_hardlinkMap->GetAllFileHandlesAndClear();
    for (size_t i = 0; i < fileHandles.size(); ++i) {
        string targetPath;
        if (fileHandles[i].empty() || !fileHandles[i][0].m_file) {
            continue;
        }
        int ret = m_hardlinkMap->GetTargetPath(fileHandles[i][0].m_file->m_inode, targetPath);
        if (ret != 0) {
            ERRLOG("not found hardlink target path for inode: %d", fileHandles[i][0].m_file->m_inode);
            continue;
        }
        targetPath = m_params.dstRootPath + "/" + targetPath;
        DBGLOG("found target path, file, inode, size , %s, %s, %d, %d", targetPath.c_str(),
            fileHandles[i][0].m_file->m_fileName.c_str(), fileHandles[i][0].m_file->m_inode,
            fileHandles[i].size());

        for (size_t j = 0; j < fileHandles[i].size(); ++j) {
            if (!fileHandles[i][j].m_file) {
                ERRLOG("fileHandle ptr is null");
                continue;
            }
            DBGLOG("Process hardlink file : %d, %s", j, fileHandles[i][j].m_file->m_fileName.c_str());
            auto hardlinkCbData = GetSmbWriterCommonData(fileHandles[i][j]);
            if (hardlinkCbData == nullptr) {
                ERRLOG("hardlink cbData create failed!");
                continue;
            }
            hardlinkCbData->linkTargetPath = targetPath;
            SendWriterRequest(fileHandles[i][j], hardlinkCbData, LibsmbEvent::LINK);
        }
    }
}

/* check if we can take ThreadFunc into base class */
void LibsmbHardlinkWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibsmbHardlinkWriter main thread start!");
    int ret = 0;
    while (true) {
        if (ServerCheck() != SUCCESS) {
            break;
        }
        if (IsComplete()) {
            INFOLOG("Write phase is complete");
            break;
        }
        if (IsAbort()) {
            WARNLOG("Main thread abort");
            break;
        }

        if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) > m_dstAdvParams->maxPendingAsyncReqCnt) {
            DBGLOG("SmbCopyWriter PENDING packet(%d) reach threshold.",
                m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
            ret = m_asyncContext->Poll(DEFAULT_POLL_EXPIRED_TIME);
            if (ret < 0 && ProcessConnectionException() != SUCCESS) {
                break;
            }
            continue;
        }
        ProcessWriteEntries();
        ProcessTimers();
        ret = m_asyncContext->Poll(DEFAULT_POLL_EXPIRED_TIME);
        // ret < 0说明连接有问题，需要重连, 如果ProcessConnectionException也返回失败，说明重连失败
        if (ret < 0 && ProcessConnectionException() != SUCCESS) {
            break;
        }
        ClearWriteCache();
        ProcessHardlinkMap();
    }
    SmbDisconnectContexts();
    HandleComplete();
    INFOLOG("LibsmbHardlinkWriter main thread end!");
    return;
}

int LibsmbHardlinkWriter::ProcessConnectionException()
{
    ERRLOG("dst connection exception");
    int ret = HandleConnectionException(m_asyncContext, m_params.dstSmbContextArgs, RECONNECT_CONTEXT_RETRY_TIMES);
    if (ret != SUCCESS) {
        ERRLOG("Stop and Abort read phase due to server inaccessible");
        m_failed = true;
        FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason, false);
        return FAILED;
    }
    INFOLOG("Server reachable");
    m_pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);
    return SUCCESS;
}

void LibsmbHardlinkWriter::ProcessWriteEntries()
{
    while (!m_writeQueue->Empty()) {
        if (IsAbort() || IsWriterRequestReachThreshold()) {
            break;
        }
        FileHandle fileHandle;
        if (!m_writeQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
            break;
        }
        ProcessFileDescState(fileHandle);
    }
}

bool LibsmbHardlinkWriter::IsWriterRequestReachThreshold() const
{
    if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) > m_dstAdvParams->maxPendingAsyncReqCnt) {
        DBGLOG("SmbCopyWriter PENDING packet(%d) reach threshold.",
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
        return true;
    }
    return false;
}

void LibsmbHardlinkWriter::ProcessFileDescState(FileHandle fileHandle)
{
    FileDescState state = fileHandle.m_file->GetDstState();
    bool isDir = fileHandle.m_file->IsFlagSet(IS_DIR);
    DBGLOG("ProcessFileDescState name: %s, %d, %d, state = %d",
        fileHandle.m_file->m_fileName.c_str(), isDir, fileHandle.m_file->m_mode, state);
    if (state == FileDescState::LINK_DEL) {
        DeleteFile(fileHandle);
    }
    if (state == FileDescState::DST_CLOSED) {
        if (m_params.writeMeta) {
            DBGLOG("ProcessFileDescState write meta: %s, %d, %d, state = %d",
                fileHandle.m_file->m_fileName.c_str(), isDir, fileHandle.m_file->m_mode, state);
            WriteMeta(fileHandle);
        } else {
            LinkFiles(fileHandle);
            ++m_controlInfo->m_noOfFilesCopied;
            m_controlInfo->m_noOfBytesCopied += fileHandle.m_file->m_size;
        }
    }
    if (state == FileDescState::INIT) {
        if (fileHandle.m_file->GetSrcState() == FileDescState::READ_FAILED) {
            // 读失败，且writer还没open这个文件，那么就跳过这个文件
            fileHandle.m_file->SetDstState(FileDescState::END);
            DBGLOG("ProcessFileDescState read failed, set write state to END, name: %s",
                fileHandle.m_file->m_fileName.c_str());
            return;
        }
        if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
            DBGLOG("ProcessFileDescState put dir: %s to dirQueue", fileHandle.m_file->m_fileName.c_str());
            m_dirQueue->Push(fileHandle);
        } else if (fileHandle.m_block.m_size == 0 && fileHandle.m_block.m_seq == 0) {
            OpenFile(fileHandle);
        } else {
            m_writeCache[fileHandle.m_file->m_fileName].push_back(fileHandle);
        }
    }
    if (IsFileReadOrWriteFailed(fileHandle)) {
        // 读或写失败: 对于备份，删除这个文件；对于恢复，则不删除用户的文件
        IsBackupTask(m_params.backupType) ? DeleteFile(fileHandle) : CloseFile(fileHandle);
    }
    bool stateCheck = (state == FileDescState::DST_OPENED) || (state == FileDescState::PARTIAL_WRITED);
    if (stateCheck) {
        WriteData(fileHandle);
    }
    if (state == FileDescState::WRITED) {
        CloseFile(fileHandle);
    }

    return;
}

int LibsmbHardlinkWriter::LinkFiles(FileHandle &fileHandle)
{
    vector<FileHandle> linkFileHandles = m_hardlinkMap->GetLinks(fileHandle.m_file->m_inode);
    bool isDir = fileHandle.m_file->IsFlagSet(IS_DIR);
    DBGLOG("ProcessFileDescState state link: %s, %d, %d, links = %d", fileHandle.m_file->m_fileName.c_str(),
        isDir, fileHandle.m_file->m_mode, linkFileHandles.size());
    for (auto linkFileHandle : linkFileHandles) {
        string targetPath;
        int ret = m_hardlinkMap->GetTargetPath(fileHandle.m_file->m_inode, targetPath);
        if (ret != SUCCESS) {
            ERRLOG("not found hardlink target path for inode: %d", fileHandle.m_file->m_inode);
            continue;
        }
        auto hardlinkCbData = GetSmbWriterCommonData(linkFileHandle);
        if (hardlinkCbData == nullptr) {
            return FAILED;
        }
        hardlinkCbData->linkTargetPath = targetPath;
        SendWriterRequest(linkFileHandle, hardlinkCbData, LibsmbEvent::LINK);

        DBGLOG("found target path: %s for file %s inode %d",
            targetPath.c_str(), fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_inode);
    }
    return SUCCESS;
}

int LibsmbHardlinkWriter::LinkFile(FileHandle &fileHandle)
{
    bool isDir = fileHandle.m_file->IsFlagSet(IS_DIR);
    DBGLOG("ProcessFileDescState state link: %s, %d, %d", fileHandle.m_file->m_fileName.c_str(),
        isDir, fileHandle.m_file->m_mode);
    string targetPath;
    int ret = m_hardlinkMap->GetTargetPath(fileHandle.m_file->m_inode, targetPath);
    if (ret != SUCCESS) {
        ERRLOG("not found hardlink target path for inode: %d", fileHandle.m_file->m_inode);
        return FAILED;
    }
    auto hardlinkCbData = GetSmbWriterCommonData(fileHandle);
    if (hardlinkCbData == nullptr) {
        return FAILED;
    }
    hardlinkCbData->linkTargetPath = targetPath;
    SendWriterRequest(fileHandle, hardlinkCbData, LibsmbEvent::LINK);
    DBGLOG("found target path: %s for file %s inode %d",
        targetPath.c_str(), fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_inode);
    return SUCCESS;
}

void LibsmbHardlinkWriter::SyncThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibsmbHardlinkWriter mkdir thread start!");

    while (true) {
        if (IsMkdirComplete()) {
            INFOLOG("SyncThreadFunc thread is complete");
            break;
        }
        if (IsAbort()) {
            WARNLOG("Main thread abort");
            break;
        }
        FileHandle fileHandle;
        bool ret = m_dirQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS);
        if (ret) {
            auto cbData = GetSmbWriterCommonData(fileHandle);
            if (cbData == nullptr) {
                continue;
            }
            if (!fileHandle.m_file->IsFlagSet(IS_DIR)) {
                cbData->path = fileHandle.m_file->m_dirName;
            } else {
                cbData->path = fileHandle.m_file->m_fileName;
            }
            DBGLOG("SyncThreadFunc get file desc from write queue :%s, %s",
                fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_dirName.c_str());
            FileDescState state = fileHandle.m_file->GetDstState();
            if (state == FileDescState::DIR_DEL) {
                SendWriterSyncRequest(fileHandle, cbData, LibsmbEvent::DELETE);
            } else {
                SendWriterSyncRequest(fileHandle, cbData, LibsmbEvent::MKDIR);
            }
        }
    }
    SmbDisconnectSyncContexts();
    INFOLOG("LibsmbHardlinkWriter main thread end!");
    return;
}

void LibsmbHardlinkWriter::FillSmbWriterCommonData(SmbWriterCommonData *writerCommonData)
{
    writerCommonData->writeSmbContext = m_asyncContext;
    writerCommonData->mkdirSmbContext = m_syncContext;
    writerCommonData->writeQueue = m_writeQueue;
    writerCommonData->dirQueue = m_dirQueue;
    writerCommonData->blockBufferMap = m_blockBufferMap;
    writerCommonData->params = m_params;

    writerCommonData->timer = &m_timer;
    writerCommonData->controlInfo = m_controlInfo;
    writerCommonData->pktStats = m_pktStats;
    writerCommonData->hardlinkMap = m_hardlinkMap;
    writerCommonData->failureRecorder = m_failureRecorder;
}

SmbWriterCommonData* LibsmbHardlinkWriter::GetSmbWriterCommonData(FileHandle &fileHandle)
{
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    FillSmbWriterCommonData(cbData);
    cbData->fileHandle = fileHandle;
    return cbData;
}

void LibsmbHardlinkWriter::ClearWriteCache()
{
    if (m_writeCache.empty()) {
        return;
    }

    for (auto it = m_writeCache.begin(); it != m_writeCache.end();) {
        FileDescState state = it->second[0].m_file->GetDstState();
        if (!it->second.empty() &&(state == FileDescState::DST_OPENED)) {
            for (auto& i : it->second) {
                DBGLOG("push from cache %s", i.m_file->m_fileName.c_str());
                m_writeQueue->Push(i);
            }
            it = m_writeCache.erase(it);
        } else if (IsFileReadOrWriteFailed(it->second[0]) ||
            state == FileDescState::WRITE_SKIP || state == FileDescState::WRITED) {
            // ignore或者replace_older，存在不需要写的文件，需要把这些文件清掉
            DBGLOG("erase from cache %s", it->second[0].m_file->m_fileName.c_str());
            it = m_writeCache.erase(it);
        } else {
            it++;
        }
    }
}