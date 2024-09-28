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
#include "DeleteControlFileReader.h"
#include "log/Log.h"
#include "ParserUtils.h"
#include "FSBackupUtils.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
const int NUMBER_ONE = 1;
const int SKIP = 1;
const int FINISH = 2;
const int QUEUE_TIMEOUT_MILLISECOND = 200;
#ifdef WIN32
    const string PATH_SEP = "\\";
#else
    const string PATH_SEP = "/";
#endif
    const std::string CTRL_FILE_PATH_SEPARATOR = "/"; /* Control file and XMeta always using slash as path separator */
}

DeleteControlFileReader::DeleteControlFileReader(const ReaderParams& readerParams)
    : m_backupParams(readerParams.backupParams),
    m_readQueue(readerParams.readQueuePtr),
    m_controlInfo(readerParams.controlInfo),
    m_blockBufferMap(readerParams.blockBufferMap) {}

DeleteControlFileReader::DeleteControlFileReader(BackupParams& backupParams,
    shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
    shared_ptr<BackupControlInfo> controlInfo,
    shared_ptr<BlockBufferMap> blockBufferMap)
    : m_backupParams(backupParams),
      m_readQueue(readQueuePtr),
      m_controlInfo(controlInfo),
      m_blockBufferMap(blockBufferMap) {}

DeleteControlFileReader::~DeleteControlFileReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

BackupRetCode DeleteControlFileReader::Start()
{
    INFOLOG("DeleteControlFileReader start!");
    try {
        m_thread = std::thread(&DeleteControlFileReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    m_controlInfo->m_startTime = ParserUtils::GetCurrentTimeInSeconds();
    return BackupRetCode::SUCCESS;
}

BackupRetCode DeleteControlFileReader::Abort()
{
    INFOLOG("DeleteControlFileReader abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode DeleteControlFileReader::Enqueue(string contrlFile)
{
    DBGLOG("DeleteControlFileReader enqueue: %s", contrlFile.c_str());
    m_controlFileQueue.push(contrlFile);
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus DeleteControlFileReader::GetStatus()
{
    DBGLOG("Enter DeleteControlFileReader GetStatus");
    return FSBackupUtils::GetControlFileReaderStatus(m_controlInfo, m_abort);
}

bool DeleteControlFileReader::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d", m_abort, m_controlInfo->m_failed.load());
        return true;
    }
    return false;
}

bool DeleteControlFileReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("DeleteControlFileReader check is complete: dirCount %llu fileCount %llu",
            m_fileCount.load(), m_dirCount.load());
    }
    if (m_controlInfo->m_controlReaderPhaseComplete) {
        INFOLOG("DeleteControlFileReader complete: dirCount %llu fileCount %llu",
            m_fileCount.load(), m_dirCount.load());
        return true;
    }
    return false;
}

void DeleteControlFileReader::HandleComplete()
{
    m_controlInfo->m_controlReaderPhaseComplete = true;
    INFOLOG("DeleteControlFileReader main thread end!");
}

/* Private methods */
void DeleteControlFileReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    string controlFile = m_controlFileQueue.front();
    m_controlFileQueue.pop();
    m_metaFileVersion = FSBackupUtils::CheckMetaFileVersion(m_backupParams.scanAdvParams.metaFilePath);
    INFOLOG("DeleteControlFileReader start, get control file from control file queue: %s", controlFile.c_str());
    if (OpenControlFile(controlFile) != Module::SUCCESS) {
        ERRLOG("set control info controlReaderFailed");
        m_controlInfo->m_controlReaderFailed = true;
        HandleComplete();
        return;
    }
    ParentInfo parentInfo;
    while (true) {
        if (IsAbort() || IsComplete()) {
            break;
        }
        string fileName;
        FileHandle fileHandle;
        fileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
        int retVal;
        if (m_metaFileVersion == META_VERSION_V10) {
            BackupDeleteCtrlEntry deleteEntry {};
            retVal = ReadControlFileEntryAndProcessV10(deleteEntry, fileName, fileHandle, parentInfo);
        } else {
            DeleteCtrlEntry deleteEntry;
            retVal = ReadControlFileEntryAndProcess(deleteEntry, fileName, fileHandle, parentInfo);
        }
        if (retVal == FINISH) {
            INFOLOG("All directory readed in control file: %s", controlFile.c_str());
            m_controlInfo->m_controlReaderPhaseComplete = true;
            break;
        }
        if (retVal == SKIP) {
            continue;
        }
        DBGLOG("Push file %s to read queue :", fileHandle.m_file->m_fileName.c_str());
        while (!m_readQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            if (IsAbort()) {
                break;
            }
        }
        ++m_controlInfo->m_controlFileReaderProduce;
    }
    HandleComplete();
    IsComplete(); // use for print statistic
    return;
}

int DeleteControlFileReader::OpenControlFile(const string& controlFile)
{
    if (m_metaFileVersion == META_VERSION_V10) {
        return OpenControlFileV10(controlFile);
    }
    return OpenControlFileV20(controlFile);
}

int DeleteControlFileReader::ReadControlFileEntry(DeleteCtrlEntry& deleteEntry, string& fileName)
{
    CTRL_FILE_RETCODE ret = m_deleteCtrlParser->ReadEntry(deleteEntry, fileName);
    if (ret == CTRL_FILE_RETCODE::READ_EOF) {
        return FINISH;
    }
    return Module::SUCCESS;
}

int DeleteControlFileReader::FillStatsFromControlHeader()
{
    // Fill stats using the statistics from control file header
    DeleteCtrlParser::Header header {};
    if (m_deleteCtrlParser->GetHeader(header) != CTRL_FILE_RETCODE::SUCCESS) {
        DBGLOG("get control file header failed");
        return Module::FAILED;
    }
    m_dirCount = header.stats.noOfDelDirs;
    m_fileCount = header.stats.noOfDelFiles;
    return Module::SUCCESS;
}

int DeleteControlFileReader::OpenControlFileV10(const std::string& controlFile)
{
    m_scannerDeleteCtrl = std::make_unique<BackupDeleteCtrl>(controlFile);
    if (!m_scannerDeleteCtrl) {
        ERRLOG("Create delete phase control file instance failed!");
        return Module::FAILED;
    }

    int ret = m_scannerDeleteCtrl->Open(NAS_CTRL_FILE_OPEN_MODE_READ);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        ERRLOG("failed to open control file: %s", controlFile.c_str());
        return Module::FAILED;
    }
    DBGLOG("open control file sucess! %s", controlFile.c_str());

    if (FillStatsFromControlHeaderV10() != Module::SUCCESS) {
        ERRLOG("failed to fill stats from control file : %s", controlFile.c_str());
        return Module::FAILED;
    }
    DBGLOG("FillStatsFromControlHeaderV10 file success!");
    return Module::SUCCESS;
}

int DeleteControlFileReader::OpenControlFileV20(const std::string& controlFile)
{
    m_deleteCtrlParser = std::make_unique<DeleteCtrlParser>(controlFile);
    if (m_deleteCtrlParser == nullptr) {
        return Module::FAILED;
    }

    DBGLOG("open control file: %s", controlFile.c_str());
    CTRL_FILE_RETCODE retVal = m_deleteCtrlParser->Open(CTRL_FILE_OPEN_MODE::READ);
    if (retVal != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("failed to open control file: %s", controlFile.c_str());
        return Module::FAILED;
    }
    DBGLOG("open control file success! _ %s", controlFile.c_str());

    if (FillStatsFromControlHeader() != Module::SUCCESS) {
        ERRLOG("failed to fill stats from control file: %s", controlFile.c_str());
        return Module::FAILED;
    }
    DBGLOG("FillStatsFromControlHeader file success!");

    return Module::SUCCESS;
}

int DeleteControlFileReader::FillStatsFromControlHeaderV10()
{
    BackupDeleteCtrlHeader header {};
    if (m_scannerDeleteCtrl != nullptr) {
        if (m_scannerDeleteCtrl->GetHeader(header) != NAS_CTRL_FILE_RET_SUCCESS) {
            ERRLOG("GetControlFile header failed!");
            return Module::FAILED;
        }
    }
    m_dirCount = header.stats.noOfDelDirs;
    m_fileCount = header.stats.noOfDelFiles;
    return Module::SUCCESS;
}

int DeleteControlFileReader::ReadControlFileEntryAndProcess(DeleteCtrlEntry& deleteEntry, string& fileName,
    FileHandle& fileHandle, ParentInfo& parentInfo)
{
    int retVal = ReadControlFileEntry(deleteEntry, fileName);
    if (retVal == FINISH) {
        return FINISH;
    }
    DBGLOG("Read from control file : %s, %s, %d", deleteEntry.m_absPath.c_str(),
        fileName.c_str(), deleteEntry.m_isDel);
    if (!deleteEntry.m_absPath.empty()) {
        parentInfo.dirName = deleteEntry.m_absPath;
        if (deleteEntry.m_isDel) {
            fileHandle.m_file->m_dirName = FSBackupUtils::GetParentDir(deleteEntry.m_absPath);
            fileHandle.m_file->m_onlyFileName = FSBackupUtils::GetFileName(deleteEntry.m_absPath);
            fileHandle.m_file->m_fileName = deleteEntry.m_absPath;
            fileHandle.m_file->SetFlag(IS_DIR);
        } else {
            // isDel is false means do not delete dir
            return SKIP;
        }
    } else if (!fileName.empty()) {
        fileHandle.m_file->ClearFlag(IS_DIR);
        fileHandle.m_file->m_dirName = parentInfo.dirName;
        fileHandle.m_file->m_fileName = parentInfo.dirName + CTRL_FILE_PATH_SEPARATOR + fileName;
        fileHandle.m_file->m_onlyFileName = fileName;
        fileHandle.m_file->m_obsKey = fileName; // delete_control 里面保存的就是原始的对象key
    }
    return retVal;
}

int DeleteControlFileReader::ReadControlFileEntryAndProcessV10(BackupDeleteCtrlEntry& deleteEntry, string& fileName,
    FileHandle& fileHandle, ParentInfo& parentInfo)
{
    NAS_CTRL_FILE_RETCODE ret = m_scannerDeleteCtrl->ReadEntry(deleteEntry, fileName);
    if (ret == NAS_CTRL_FILE_RET_READ_EOF) {
        return FINISH;
    }
    DBGLOG("Read from control file : %s, %s, %d",
        deleteEntry.m_absPath.c_str(), fileName.c_str(), deleteEntry.m_isDel);
    if (!deleteEntry.m_absPath.empty()) {
        parentInfo.dirName = deleteEntry.m_absPath;
        if (deleteEntry.m_isDel) {
            fileHandle.m_file->m_fileName = deleteEntry.m_absPath;
            fileHandle.m_file->SetFlag(IS_DIR);
        } else {
            // isDel is false means do not delete dir
            return SKIP;
        }
    } else if (!fileName.empty()) {
        fileHandle.m_file->m_fileName = parentInfo.dirName + CTRL_FILE_PATH_SEPARATOR + fileName;
        fileHandle.m_file->ClearFlag(IS_DIR);
    }
    return ret;
}
