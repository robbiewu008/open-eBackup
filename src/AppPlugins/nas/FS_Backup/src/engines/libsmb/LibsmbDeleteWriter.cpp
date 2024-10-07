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
#include "LibsmbDeleteWriter.h"
#include "log/Log.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;
namespace {
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
}

LibsmbDeleteWriter::LibsmbDeleteWriter(const WriterParams &deleteWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : WriterBase(deleteWriterParams)
{
    m_failureRecorder = failureRecorder;
    m_dstAdvParams = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.dstAdvParams);
    FillContextParams(m_params.dstSmbContextArgs, m_dstAdvParams);
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_pktStats = make_shared<PacketStats>();
    m_params.dstRootPath = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.dstAdvParams)->rootPath;
}

LibsmbDeleteWriter::~LibsmbDeleteWriter()
{
    INFOLOG("LibsmbDeleteWriter Destruct");
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

int LibsmbDeleteWriter::SmbConnectContexts()
{
    m_deleteSmbContext = SmbConnectContext(m_params.dstSmbContextArgs);
    if (m_deleteSmbContext == nullptr) {
        ERRLOG("Smb connect contexts failed!");
        return FAILED;
    }
    return SUCCESS;
}

void LibsmbDeleteWriter::SmbContextDisconnect()
{
    INFOLOG("Write SmbDisconnectContexts");
    SmbDisconnectContext(m_deleteSmbContext);
}

BackupRetCode LibsmbDeleteWriter::Start()
{
    if (SmbConnectContexts() != SUCCESS) {
        return BackupRetCode::FAILED;
    }
    
    DBGLOG("LibsmbDeleteWrite call Start!");

    try {
        m_thread = thread(&LibsmbDeleteWriter::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        SmbContextDisconnect();
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        SmbContextDisconnect();
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbDeleteWriter::Abort()
{
    INFOLOG("LibsmbWriter Enter Abort");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbDeleteWriter::Destroy()
{
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibsmbDeleteWriter::GetStatus()
{
    return FSBackupUtils::GetWriterStatus(m_controlInfo, m_abort, BackupPhaseStatus::FAILED);
}

int LibsmbDeleteWriter::OpenFile(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int LibsmbDeleteWriter::WriteMeta(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int LibsmbDeleteWriter::WriteData(FileHandle &fileHandle)
{
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, m_params.dstRootPath);
    bool delStatIsDir = false;
    struct smb2_stat_64 st;
    int ret = m_deleteSmbContext->SmbStat64(smbPath.c_str(), &st);
    if (ret != SUCCESS) {
        if (ret == -ENOENT) {
            AddDeleteCounter(true, fileHandle.m_file->IsFlagSet(IS_DIR));
            DBGLOG("SmbStat64 file not exist: %s, ret: %d", smbPath.c_str(), ret);
            return SUCCESS;
        }
        AddDeleteCounter(false, fileHandle.m_file->IsFlagSet(IS_DIR));
        ERRLOG("SmbStat64 Failed: %s, ret: %d", smbPath.c_str(), ret);
        return FAILED;
    }
    if (st.smb2_type == SMB2_TYPE_DIRECTORY) {
        delStatIsDir = true;
    } else {
        delStatIsDir = false;
    }
    if (CompareTypeOfDeleteEntryAndBackupCopy(fileHandle, delStatIsDir) != BackupRetCode::SUCCESS) {
        return FAILED;
    }
    if (DeleteFileDirectoryLibSmb(fileHandle, delStatIsDir) != BackupRetCode::SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

void LibsmbDeleteWriter::AddDeleteCounter(bool isSuccess, bool isDir)
{
    if (isSuccess) {
        if (isDir) {
            ++m_deleteDir;
        } else {
            ++m_deleteFile;
        }
    } else {
        if (isDir) {
            ++m_deleteFailedDir;
        } else {
            ++m_deleteFailedFile;
        }
    }
}

BackupRetCode LibsmbDeleteWriter::CompareTypeOfDeleteEntryAndBackupCopy(FileHandle &fileHandle, bool delStatIsDir)
{
    if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
        if (!delStatIsDir) {
            ++m_deleteDir;
            DBGLOG("File present. Already deleted and created as file: %s.", fileHandle.m_file->m_fileName.c_str());
            return BackupRetCode::FAILED;
        }
    } else {
        if (delStatIsDir) {
            ++m_deleteFile;
            DBGLOG("Dir present. Already deleted and created as dir: %s.", fileHandle.m_file->m_fileName.c_str());
            return BackupRetCode::FAILED;
        }
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbDeleteWriter::DeleteFileDirectoryLibSmb(FileHandle &fileHandle, bool isDir)
{
    int ret = SUCCESS;
    if (isDir) {
        auto cbData = GetSmbWriterCommonData(fileHandle);
        if (cbData == nullptr) {
            return BackupRetCode::FAILED;
        }
        if (SendWriterSyncRequest(fileHandle, cbData, LibsmbEvent::DELETE) != 0) {
            return BackupRetCode::FAILED;
        }
        ++m_deleteDir;
        return BackupRetCode::SUCCESS;
    } else {
        std::string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
        ConcatRootPath(smbPath, m_params.dstRootPath);
        ret = m_deleteSmbContext->SmbUnlink(smbPath.c_str());
        if (ret == SUCCESS) {
            ++m_deleteFile;
            DBGLOG("Delete file success: %s.", smbPath.c_str());
        } else if (ret == -ENOENT) {
            ++m_deleteFile;
            WARNLOG("File: %s already deleted ", smbPath.c_str());
        } else {
            ++m_deleteFailedFile;
            DBGLOG("Delete file failed: %s ret: %d, err: %s", smbPath.c_str(), ret,
                   m_deleteSmbContext->SmbGetError().c_str());
            return BackupRetCode::FAILED;
        }
    }
    return BackupRetCode::SUCCESS;
}

int LibsmbDeleteWriter::CloseFile(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

bool LibsmbDeleteWriter::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        HandleComplete();
        return true;
    }
    return false;
}

bool LibsmbDeleteWriter::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("aggrComplete %d (deleteDir %d deleteFailedDir %d deleteFile %d deleteFailedFile %d) "
            "(controlFileReaderProduce %d)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_deleteDir.load(),
            m_deleteFailedDir.load(),
            m_deleteFile.load(),
            m_deleteFailedFile.load(),
            m_controlInfo->m_controlFileReaderProduce.load());
    }
    if ((m_controlInfo->m_aggregatePhaseComplete) &&
        ((m_deleteDir + m_deleteFailedDir + m_deleteFile + m_deleteFailedFile) ==
         (m_controlInfo->m_controlFileReaderProduce))) {
        INFOLOG("aggrComplete %d (deleteDir %d deleteFailedDir %d deleteFile %d deleteFailedFile %d) "
            "(controlFileReaderProduce %d)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_deleteDir.load(),
            m_deleteFailedDir.load(),
            m_deleteFile.load(),
            m_deleteFailedFile.load(),
            m_controlInfo->m_controlFileReaderProduce.load());
        HandleComplete();
        return true;
    }
    return false;
}

void LibsmbDeleteWriter::HandleComplete()
{
    INFOLOG("Complete LibsmbDeleteWriter");
    m_controlInfo->m_writePhaseComplete = true;
}

/* check if we can take ThreadFunc into base class */
void LibsmbDeleteWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibsmbDeleteWriter main thread start!");
    while (true) {
        if (IsComplete()) {
            break;
        }
        if (IsAbort()) {
            WARNLOG("LibsmbDeleteWriter main thread abort");
            break;
        }
        FileHandle fileHandle;
        bool ret = m_writeQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS);
        if (ret) {
            ++m_controlInfo->m_writerConsume;
            FileDescState state = fileHandle.m_file->GetDstState();
            if (state == FileDescState::INIT) {
                WriteData(fileHandle);
            }
        }
    }
    INFOLOG("LibsmbDeleteWriter main thread end!");
    SmbContextDisconnect();
    return;
}

void LibsmbDeleteWriter::FillSmbWriterCommonData(SmbWriterCommonData *writerCommonData)
{
    writerCommonData->writeSmbContext = m_deleteSmbContext;
    writerCommonData->mkdirSmbContext = m_deleteSmbContext;
    writerCommonData->writeQueue = m_writeQueue;
    writerCommonData->blockBufferMap = m_blockBufferMap;
    writerCommonData->params = m_params;
    writerCommonData->timer = &m_timer;
    writerCommonData->controlInfo = m_controlInfo;
    writerCommonData->pktStats = m_pktStats;
    writerCommonData->failureRecorder = m_failureRecorder;
}

SmbWriterCommonData* LibsmbDeleteWriter::GetSmbWriterCommonData(FileHandle &fileHandle)
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
