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
#include "DirControlFileReader.h"
#include "log/Log.h"
#include "ParserUtils.h"
#include "FSBackupUtils.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
const int SKIP = 1;
const int FINISH = 2;
const int NUMBER_ONE = 1;
const int QUEUE_TIMEOUT_MILLISECOND = 10;
}

DirControlFileReader::DirControlFileReader(const ReaderParams& readerParams)
    : m_backupParams(readerParams.backupParams),
    m_readQueue(readerParams.readQueuePtr),
    m_controlInfo(readerParams.controlInfo),
    m_blockBufferMap(readerParams.blockBufferMap)
{}

DirControlFileReader::DirControlFileReader(BackupParams& backupParams,
    shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
    shared_ptr<BackupControlInfo> controlInfo,
    shared_ptr<BlockBufferMap> blockBufferMap)
    : m_backupParams(backupParams),
      m_readQueue(readQueuePtr),
      m_controlInfo(controlInfo),
      m_blockBufferMap(blockBufferMap)
{
}

DirControlFileReader::~DirControlFileReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

BackupRetCode DirControlFileReader::Start()
{
    INFOLOG("DirControlFileReader start!");
    try {
        m_thread = std::thread(&DirControlFileReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    m_controlInfo->m_startTime = Module::ParserUtils::GetCurrentTimeInSeconds();
    return BackupRetCode::SUCCESS;
}

BackupRetCode DirControlFileReader::Abort()
{
    INFOLOG("DirControlFileReader abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode DirControlFileReader::Enqueue(string contrlFile)
{
    DBGLOG("DirControlFileReader enqueue: %s", contrlFile.c_str());
    m_controlFileQueue.push(contrlFile);
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus DirControlFileReader::GetStatus()
{
    return FSBackupUtils::GetControlFileReaderStatus(m_controlInfo, m_abort);
}

bool DirControlFileReader::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d", m_abort, m_controlInfo->m_failed.load());
        return true;
    }
    return false;
}

bool DirControlFileReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("DirControlFileReader check is complete: dirCount %llu", m_dirCount.load());
    }
    if (m_controlInfo->m_controlReaderPhaseComplete) {
        INFOLOG("DirControlFileReader complete: dirCount %llu", m_dirCount.load());
        return true;
    }
    return false;
}

/* Private methods */
void DirControlFileReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    string controlFile;
    controlFile = m_controlFileQueue.front();
    m_controlFileQueue.pop();
    m_metaFileVersion = FSBackupUtils::CheckMetaFileVersion(m_backupParams.scanAdvParams.metaFilePath);
    INFOLOG("DirControlFileReader start, get control file from control file queue: %s", controlFile.c_str());
    if (OpenControlFile(controlFile) != Module::SUCCESS) {
        ERRLOG("set control info controlReaderFailed");
        m_controlInfo->m_controlReaderFailed = true;
        m_controlInfo->m_controlReaderPhaseComplete = true;
        ERRLOG("DirControlFileReader main thread end after failure!");
        return;
    }
    ParentInfo parentInfo;
    while (true) {
        if (IsAbort() || IsComplete()) {
            break;
        }
        FileHandle fileHandle;
        fileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
        int res;
        if (m_metaFileVersion == META_VERSION_V10) {
            BackupMtimeCtrlEntry dirEntry {};
            res = ReadControlFileEntryAndProcessV10(dirEntry, fileHandle);
        } else {
            MtimeCtrlEntry dirEntry {};
            res = ReadControlFileEntryAndProcess(dirEntry, fileHandle);
        }
        if (res == FINISH) {
            DBGLOG("All directory readed in control file: %s", controlFile.c_str());
            break;
        }
        if (res == SKIP) {
            continue;
        }
        DBGLOG("Push file %s to read queue", fileHandle.m_file->m_fileName.c_str());
        while (!m_readQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            if (IsAbort()) {
                break;
            }
        }
        ++m_controlInfo->m_controlFileReaderProduce;
    }
    m_controlInfo->m_controlReaderPhaseComplete = true;
    IsComplete(); // use to print statistic
    INFOLOG("DirControlFileReader main thread end!");
}

int DirControlFileReader::OpenControlFile(const string& controlFile)
{
    if (m_metaFileVersion == META_VERSION_V10) {
        return OpenControlFileV10(controlFile);
    }
    return OpenControlFileV20(controlFile);
}

int DirControlFileReader::ReadControlFileEntry(MtimeCtrlEntry& dirEntry)
{
    CTRL_FILE_RETCODE ret = m_mtimeCtrlParser->ReadEntry(dirEntry);
    if (ret == CTRL_FILE_RETCODE::READ_EOF) {
        return FINISH;
    }
    return Module::SUCCESS;
}

int DirControlFileReader::ProcessDirEntry(const MtimeCtrlEntry& dirEntry, const FileHandle& fileHandle)
{
    fileHandle.m_file->SetFlag(IS_DIR);
    fileHandle.m_file->m_dirName = dirEntry.m_absPath;
    fileHandle.m_file->m_onlyFileName = fileHandle.m_file->m_dirName.substr(
        fileHandle.m_file->m_dirName.find_last_of("/") + NUMBER_ONE,
        fileHandle.m_file->m_dirName.length() - NUMBER_ONE);
    fileHandle.m_file->m_mode = dirEntry.m_mode;
    fileHandle.m_file->m_atime = dirEntry.m_atime;
    fileHandle.m_file->m_mtime = dirEntry.m_mtime;
    fileHandle.m_file->m_ctime = dirEntry.m_ctime;
    fileHandle.m_file->m_btime = dirEntry.m_btime;
    fileHandle.m_file->m_uid = dirEntry.m_uid;
    fileHandle.m_file->m_gid = dirEntry.m_gid;
    fileHandle.m_file->m_fileAttr = dirEntry.m_attr;

    return Module::SUCCESS;
}

int DirControlFileReader::ProcessDirEntryV10(const BackupMtimeCtrlEntry& dirEntry, FileHandle& fileHandle)
{
    fileHandle.m_file->SetFlag(IS_DIR);
    fileHandle.m_file->m_dirName = dirEntry.m_absPath;
    fileHandle.m_file->m_onlyFileName = fileHandle.m_file->m_dirName.substr(
        fileHandle.m_file->m_dirName.find_last_of("/") + NUMBER_ONE,
        fileHandle.m_file->m_dirName.length() - NUMBER_ONE);
#ifndef WIN32
    fileHandle.m_file->m_mode = dirEntry.m_mode;
#endif
    fileHandle.m_file->m_uid = dirEntry.m_uid;
    fileHandle.m_file->m_gid = dirEntry.m_gid;
    fileHandle.m_file->m_atime = dirEntry.m_atime;
    fileHandle.m_file->m_mtime = dirEntry.m_mtime;
    fileHandle.m_file->m_ctime = dirEntry.m_ctime;
    fileHandle.m_file->m_btime = dirEntry.m_btime;
    fileHandle.m_file->m_fileAttr = dirEntry.m_attr;

    return Module::SUCCESS;
}

int DirControlFileReader::FillStatsFromControlHeader()
{
    // Fill stats using the statistics from control file header
    MtimeCtrlParser::Header header {};
    if (m_mtimeCtrlParser->GetHeader(header) != CTRL_FILE_RETCODE::SUCCESS) {
        DBGLOG("get control file header failed");
        return Module::FAILED;
    }
    m_dirCount = header.stats.noOfDirs;
    return Module::SUCCESS;
}

int DirControlFileReader::OpenControlFileV10(const std::string& controlFile)
{
    m_scannerMtimeCtrl = std::make_unique<BackupMtimeCtrl>(controlFile);
    if (!m_scannerMtimeCtrl) {
        ERRLOG("create backup mtime control instance failed!");
        return Module::FAILED;
    }

    INFOLOG("open control file : %s", controlFile.c_str());
    int ret = m_scannerMtimeCtrl->Open(NAS_CTRL_FILE_OPEN_MODE_READ);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        ERRLOG("failed to open control file: %s", controlFile.c_str());
        return Module::FAILED;
    }
    INFOLOG("open control file success! %s", controlFile.c_str());
    if (FillStatsFromControlHeaderV10() != Module::SUCCESS) {
        ERRLOG("failed to fill stats from control file : %s", controlFile.c_str());
        return Module::FAILED;
    }
    INFOLOG("FillStatsFromControlFileHeader success!");
    return Module::SUCCESS;
}

int DirControlFileReader::OpenControlFileV20(const std::string& controlFile)
{
    m_mtimeCtrlParser = std::make_unique<MtimeCtrlParser>(controlFile);
    if (m_mtimeCtrlParser == nullptr) {
        return Module::FAILED;
    }

    DBGLOG("open control file: %s", controlFile.c_str());
    CTRL_FILE_RETCODE ret = m_mtimeCtrlParser->Open(CTRL_FILE_OPEN_MODE::READ);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("failed to open control file: %s", controlFile.c_str());
        return Module::FAILED;
    }
    DBGLOG("open control file success! :_ %s", controlFile.c_str());

    if (FillStatsFromControlHeader() != Module::SUCCESS) {
        ERRLOG("failed to fill stats from control file: %s", controlFile.c_str());
        return Module::FAILED;
    }
    DBGLOG("FillStatsFromControlHeader file success!");

    return Module::SUCCESS;
}

int DirControlFileReader::FillStatsFromControlHeaderV10()
{
    BackupMtimeCtrlHeader header {};
    if (m_scannerMtimeCtrl == nullptr) {
        ERRLOG("m_scannerMtimeCtrl is nullptr");
        return Module::FAILED;
    }
    if (m_scannerMtimeCtrl->GetHeader(header) != NAS_CTRL_FILE_RET_SUCCESS) {
        ERRLOG("Get Header Failed!");
        return Module::FAILED;
    }
    m_dirCount = header.stats.noOfDirs;
    return Module::SUCCESS;
}

int DirControlFileReader::ReadControlFileEntryAndProcess(MtimeCtrlEntry& dirEntry, FileHandle& fileHandle)
{
    int ret = ReadControlFileEntry(dirEntry);
    if (ret == FINISH) {
        return FINISH;
    }
    // 不写根路径的权限
    if (!dirEntry.m_absPath.empty() && dirEntry.m_absPath != "." &&
        dirEntry.m_absPath != "/") {
        ProcessDirEntry(dirEntry, fileHandle);
        DBGLOG("ProcessDirEntry: %s, %d", dirEntry.m_absPath.c_str(), ret);
    } else {
        DBGLOG("Skip dir %s", dirEntry.m_absPath.c_str());
        return SKIP;
    }
    return Module::SUCCESS;
}

int DirControlFileReader::ReadControlFileEntryAndProcessV10(BackupMtimeCtrlEntry& dirEntry, FileHandle& fileHandle)
{
    NAS_CTRL_FILE_RETCODE ret = m_scannerMtimeCtrl->ReadEntry(dirEntry);
    if (ret == NAS_CTRL_FILE_RET_READ_EOF) {
        return FINISH;
    }
    // 不写根路径的权限
    if (!dirEntry.m_absPath.empty() && dirEntry.m_absPath != "." &&
        dirEntry.m_absPath != "/") {
        ProcessDirEntryV10(dirEntry, fileHandle);
        DBGLOG("ProcessDirEntry: %s, %d", dirEntry.m_absPath.c_str(), ret);
    } else {
        DBGLOG("Skip dir %s", dirEntry.m_absPath.c_str());
        return SKIP;
    }
    return Module::SUCCESS;
}
