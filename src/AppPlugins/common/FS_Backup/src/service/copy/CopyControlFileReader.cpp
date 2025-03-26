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
#include "CopyControlFileReader.h"
#include "log/Log.h"
#include "common/Thread.h"
#include "ParserUtils.h"
#include "XMetaParser.h"
#include "AdsParser.h"

using namespace std;
using namespace Module;

namespace {
#ifdef WIN32
    const std::string PATH_SEP = "\\";
#else
    const std::string PATH_SEP = "/";
#endif
    const std::string CTRL_FILE_PATH_SEPARATOR = "/"; /* Control file and XMeta always using slash as path separator */
    const int DIR_SKIP = 1;
    const int FILE_SKIP = 2;
    const int FINISH = 3;
    const int NUMBER_ONE = 1;
    const int QUEUE_TIMEOUT_MILLISECOND = 10;
    const int METAFILE_NAME_PREFIX_LEN = 10;
    const uint32_t KB_IN_BYTES = 1024;
    const string XMETA_FILENAME_PFX = "xmeta_file_";
    const int XMETA_FILENAME_LEN = 1024;
    const int MAX_FILEHANDLE_VECTOR_SIZE = 20000;
    const int READ_CONTROL_FILE_HEAD_TIMEOUT_SECONDS = 1800;
}

CopyControlFileReader::CopyControlFileReader(const ReaderParams& readerParams)
    : m_backupParams(readerParams.backupParams),
    m_readQueue(readerParams.readQueuePtr),
    m_aggregateQueue(readerParams.aggregateQueuePtr),
    m_controlInfo(readerParams.controlInfo),
    m_blockBufferMap(readerParams.blockBufferMap),
    m_failureRecorder(readerParams.failureRecorder)
{}

CopyControlFileReader::CopyControlFileReader(BackupParams& backupParams,
    shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
    shared_ptr<BackupQueue<FileHandle>> aggregateQueuePtr,
    shared_ptr<BackupControlInfo> controlInfo,
    shared_ptr<BlockBufferMap> blockBufferMap)
    : m_backupParams(backupParams),
      m_readQueue(readQueuePtr),
      m_aggregateQueue(aggregateQueuePtr),
      m_controlInfo(controlInfo),
      m_blockBufferMap(blockBufferMap)
{
}

CopyControlFileReader::~CopyControlFileReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_monitorthread.joinable()) {
        m_monitorthread.join();
    }
}

/* Public APIs */
BackupRetCode CopyControlFileReader::Start()
{
    INFOLOG("CopyControlFileReader start!");
    try {
        m_thread = std::thread(&CopyControlFileReader::ThreadFunc, this);
        m_monitorthread = std::thread(&CopyControlFileReader::MonitorReadControlHeader, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknown reason");
        return BackupRetCode::FAILED;
    }
    m_controlInfo->m_startTime = Module::ParserUtils::GetCurrentTimeInSeconds();
    return BackupRetCode::SUCCESS;
}

BackupRetCode CopyControlFileReader::Abort()
{
    INFOLOG("CopyControlFileReader abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode CopyControlFileReader::Enqueue(string contrlFile)
{
    INFOLOG("CopyControlFileReader enqueue: %s", contrlFile.c_str());
    m_controlFileQueue.push(contrlFile);
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus CopyControlFileReader::GetStatus()
{
    return FSBackupUtils::GetControlFileReaderStatus(m_controlInfo, m_abort);
}

bool CopyControlFileReader::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d", m_abort, m_controlInfo->m_failed.load());
        return true;
    }
    return false;
}

bool CopyControlFileReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("ControlFileReader check is complete: controlFileReaderProduce %d skip %llu total %llu, "
                "noOfFileEntriesReaded %llu",
            m_controlInfo->m_controlFileReaderProduce.load(), m_controlInfo->m_skipFileCnt.load(),
            (m_controlInfo->m_noOfFilesToBackup.load() + m_controlInfo->m_noOfDirToBackup.load()),
            m_noOfFileEntriesReaded);
    }
    if ((m_controlInfo->m_controlFileReaderProduce + m_controlInfo->m_skipFileCnt) ==
        (m_controlInfo->m_noOfFilesToBackup + m_controlInfo->m_noOfDirToBackup)) {
        INFOLOG("ControlFileReader complete: controlFileReaderProduce %d skip %llu total %llu",
            m_controlInfo->m_controlFileReaderProduce.load(), m_controlInfo->m_skipFileCnt.load(),
            (m_controlInfo->m_noOfFilesToBackup.load() + m_controlInfo->m_noOfDirToBackup.load()));
        return true;
    }
    return false;
}

void CopyControlFileReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    int ret;
    std::string controlFile = m_controlFileQueue.front();
    m_controlFileQueue.pop();
    INFOLOG("CopyControlFileReader start, get control file from control file queue: %s", controlFile.c_str());
    m_metaFileVersion = FSBackupUtils::CheckMetaFileVersion(m_backupParams.scanAdvParams.metaFilePath);
    if (OpenControlFile(controlFile) != Module::SUCCESS) {
        ERRLOG("Open control file failed!");
        m_controlInfo->m_controlReaderFailed = true;
        m_controlInfo->m_controlReaderPhaseComplete = true;
        return;
    }
    ParentInfo parentInfo {};
    while (true) {
        if (IsAbort() || IsComplete()) {
            break;
        }
        if (m_metaFileVersion == META_VERSION_V10) {
             // v1.0 read file entry
            ScannerBackupCtrlFileEntry fileEntry {};
            ScannerBackupCtrlDirEntry dirEntry {};
            ret = ReadControlFileEntryAndProcessV10(fileEntry, dirEntry, parentInfo);
        } else {
            // v2.0 read file entry
            CopyCtrlFileEntry fileEntry;
            CopyCtrlDirEntry dirEntry;
            ret = ReadControlFileEntryAndProcess(fileEntry, dirEntry, parentInfo);
        }
        if (ret == FINISH) {
            INFOLOG("ControlFileReader finish!");
            break;
        }
        if (ret == Module::FAILED) {
            continue;
        }
    }
    if (m_noOfFileEntriesReaded != m_controlInfo->m_noOfFilesToBackup) {
        m_controlInfo->m_controlReaderFailed = true;
        m_controlInfo->m_controlReaderPhaseComplete = true;
        ERRLOG("no of entries readed and no of entries in header missmatch! controlfile: %s, %u %u %u %u",
            controlFile.c_str(), m_noOfFileEntriesReaded, m_controlInfo->m_noOfFilesToBackup.load(),
            m_noOfDirEntriedReaded, m_controlInfo->m_noOfDirToBackup.load());
        return;
    }
    m_controlInfo->m_controlReaderPhaseComplete = true;
    INFOLOG("CopyControlFileReader main thread end ControlFileName = %s", controlFile.c_str());
    return;
}

int CopyControlFileReader::ProcessControlFileEntry(CopyCtrlDirEntry& dirEntry, CopyCtrlFileEntry& fileEntry,
    ParentInfo &parentInfo)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    int32_t ret;

    if (!dirEntry.m_dirName.empty()) {
        if (!m_priorityQueue.empty() || !m_priorityQueueReverse.empty()) {
            JudgeAndPushPriorityToReader();
        }
        ret = ProcessDirEntry(parentInfo, dirEntry, fileHandle);
        DBGLOG("ProcessDirectoryEntry: %s, %d, fileCount: %llu",
            dirEntry.m_dirName.c_str(), ret, fileHandle.m_file->m_fileCount);
        m_noOfDirEntriedReaded++;
        return PushDirToAggregator(ret, fileHandle);
    } else if (!fileEntry.m_fileName.empty()) {
        ret = ProcessFileEntry(parentInfo, fileEntry, fileHandle);
        DBGLOG("ProcessFileEntry: %s, %d", fileEntry.m_fileName.c_str(), ret);
        m_noOfFileEntriesReaded++;
        if (m_backupParams.commonParams.orderOfFilenames != OrderOfRestore::OFF) {
            return PushFileHandleToPrioity(ret, fileHandle); // push到優先級隊列
        } else {
            return PushFileHandleToReader(ret, fileHandle);
        }
    } else if (dirEntry.m_dirName.empty() && fileEntry.m_fileName.empty()) {
        ERRLOG("either dirname and filename are empty");
        return Module::SUCCESS; // nas dir may be empty
    }
    return Module::SUCCESS;
}

int CopyControlFileReader::ProcessControlFileEntryV10(ScannerBackupCtrlDirEntry& dirEntry,
    ScannerBackupCtrlFileEntry& fileEntry, ParentInfo& parentInfo)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    int ret;

    if (!dirEntry.m_dirName.empty()) {
        ret = ProcessDirEntryV10(parentInfo, dirEntry, fileHandle);
        DBGLOG("ProcessDirEntryV10: %s, %d", dirEntry.m_dirName.c_str(), ret);
        return PushDirToAggregator(ret, fileHandle);
    } else if (!fileEntry.m_fileName.empty()) {
        ret = ProcessFileEntryV10(parentInfo, fileEntry, fileHandle);
        DBGLOG("ProcessFileEntryV10: %s, %d", fileEntry.m_fileName.c_str(), ret);
    } else if (dirEntry.m_dirName.empty() && fileEntry.m_fileName.empty()) {
        ERRLOG("either dirname and filename are empty");
        return Module::SUCCESS;
    }

    return PushFileHandleToReader(ret, fileHandle);
}

int CopyControlFileReader::PushFileHandleToReader(const int& ret, FileHandle& fileHandle)
{
    if (fileHandle.m_file->GetSrcState() == FileDescState::AGGREGATED &&
        !IsWindowsStyleEngine(m_backupParams.srcEngine)) {
        return Module::SUCCESS;
    }

    if (ret == FILE_SKIP) {
        ++m_controlInfo->m_skipFileCnt;
        return Module::SUCCESS;
    } else if (ret == Module::FAILED) {
        ++m_controlInfo->m_controlFileReaderProduce;
        ++m_controlInfo->m_noOfFilesFailed;
        ++m_controlInfo->m_noOfFilesReadFailed;
        WARNLOG("Read meta or xmeta file failed!");
        return Module::FAILED;
    }
    bool isDir = fileHandle.m_file->IsFlagSet(IS_DIR);
#ifndef WIN32
    DBGLOG("Push to read queue %s %s  isDir %d S_ISDIR %d", fileHandle.m_file->m_dirName.c_str(),
        fileHandle.m_file->m_fileName.c_str(), isDir, S_ISDIR(fileHandle.m_file->m_mode));
#else
    DBGLOG("Push to read queue %s %s  isDir %d mode %u", fileHandle.m_file->m_dirName.c_str(),
        fileHandle.m_file->m_fileName.c_str(), isDir, fileHandle.m_file->m_mode);
#endif

    while (!m_readQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
        if (IsAbort()) {
            return Module::SUCCESS;
        }
    }
    ++m_controlInfo->m_controlFileReaderProduce;
    return Module::SUCCESS;
}

int CopyControlFileReader::JudgeAndPushPriorityToReader()
{
    if (m_backupParams.commonParams.orderOfFilenames == OrderOfRestore::ON_LEXICOGRAPHICAL_ORDER) {
        PushPriorityToReader(m_priorityQueue);
    } else {
        PushPriorityToReader(m_priorityQueueReverse);
    }
    return Module::SUCCESS;
}

template<typename CmpFile>
int CopyControlFileReader::PushPriorityToReader(std::priority_queue<FileHandle, std::vector<FileHandle>, CmpFile>& queue)
{
    while (!queue.empty()) {
        FileHandle fileHandle = queue.top();
        queue.pop();
        while (!m_readQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            if (IsAbort()) {
                return Module::SUCCESS;
            }
        }
    }
    return Module::SUCCESS;
}

int CopyControlFileReader::PushFileHandleToPrioity(const int& ret, FileHandle& fileHandle)
{
    if (fileHandle.m_file->GetSrcState() == FileDescState::AGGREGATED &&
        !IsWindowsStyleEngine(m_backupParams.srcEngine)) {
        return Module::SUCCESS;
    }

    if (ret == FILE_SKIP) {
        ++m_controlInfo->m_skipFileCnt;
        return Module::SUCCESS;
    } else if (ret == Module::FAILED) {
        ++m_controlInfo->m_controlFileReaderProduce;
        ++m_controlInfo->m_noOfFilesFailed;
        ++m_controlInfo->m_noOfFilesReadFailed;
        WARNLOG("Read meta or xmeta file failed!");
        return Module::FAILED;
    }
    bool isDir = fileHandle.m_file->IsFlagSet(IS_DIR);
#ifndef WIN32
    DBGLOG("Push to read queue %s %s  isDir %d S_ISDIR %d", fileHandle.m_file->m_dirName.c_str(),
        fileHandle.m_file->m_fileName.c_str(), isDir, S_ISDIR(fileHandle.m_file->m_mode));
#else
    DBGLOG("Push to read queue %s %s  isDir %d mode %u", fileHandle.m_file->m_dirName.c_str(),
        fileHandle.m_file->m_fileName.c_str(), isDir, fileHandle.m_file->m_mode);
#endif
    try {
        if (m_backupParams.commonParams.orderOfFilenames == OrderOfRestore::ON_LEXICOGRAPHICAL_ORDER) {
            m_priorityQueue.push(fileHandle);
            ++m_controlInfo->m_controlFileReaderProduce;
        } else {
            m_priorityQueueReverse.push(fileHandle);
            ++m_controlInfo->m_controlFileReaderProduce;
        }
    } catch (std::exception& e) {
        ERRLOG("push to priority_queue failed, message: %s", e.what());
    }
    if (IsComplete()) {
        JudgeAndPushPriorityToReader();
    }
    return Module::SUCCESS;
}

int CopyControlFileReader::PushDirToAggregator(const int& ret, FileHandle& fileHandle)
{
    if (ret == DIR_SKIP) {
        // hardlink source file is dd, match in this case
        ++m_controlInfo->m_skipDirCnt;
        return Module::SUCCESS;
    } else if (ret == Module::FAILED) {
        WARNLOG("Read meta or xmeta file failed!");
        ++m_controlInfo->m_controlFileReaderProduce;
        ++m_controlInfo->m_noOfDirRead;
        return Module::FAILED;
    }
    bool isDir = fileHandle.m_file->IsFlagSet(IS_DIR);
#ifndef WIN32
    DBGLOG("Push to aggrergate queue %s %s  isDir %d S_ISDIR %d", fileHandle.m_file->m_dirName.c_str(),
        fileHandle.m_file->m_fileName.c_str(), isDir, S_ISDIR(fileHandle.m_file->m_mode));
#else
    DBGLOG("Push to aggrergate queue %s %s  isDir %d mode %u", fileHandle.m_file->m_dirName.c_str(),
        fileHandle.m_file->m_fileName.c_str(), isDir, fileHandle.m_file->m_mode);
#endif
    PushFileHandleToAggregator(fileHandle);
    ++m_controlInfo->m_controlFileReaderProduce;
    m_controlInfo->m_noOfDirRead++;
    return Module::SUCCESS;
}

void CopyControlFileReader::PushFileHandleToAggregator(FileHandle& fileHandle)
{
    // 要支持ADS，需要在Reader读取每个文件和目录的ADS文件名，因此不能跳过Reader
    if (IsWindowsStyleEngine(m_backupParams.srcEngine)) {
        if (!fileHandle.m_file->IsFlagSet(IS_DIR)) {
            return;
        }
        while (!m_readQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            if (IsAbort()) {
                return;
            }
        }
        return;
    }

    if (m_backupParams.backupType == BackupType::RESTORE &&
        (m_backupParams.commonParams.backupDataFormat == BackupDataFormat::AGGREGATE)) {
        while (m_controlInfo->m_aggrRestoreInMemoryFhCnt >= MAX_FILEHANDLE_VECTOR_SIZE) {
            if (IsAbort()) {
                return;
            }
            Module::SleepFor(chrono::milliseconds(QUEUE_TIMEOUT_MILLISECOND));
        }
    }

    while (!m_aggregateQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
        if (IsAbort()) {
            return;
        }
    }
    ++m_controlInfo->m_readProduce;
    return;
}

int CopyControlFileReader::OpenControlFile(const std::string& controlFile)
{
    if (m_metaFileVersion == META_VERSION_V10) {
        return OpenControlFileV10(controlFile);
    }
    return OpenControlFileV20(controlFile);
}

int CopyControlFileReader::OpenControlFileV10(const std::string& controlFile)
{
    m_scannerBackupCtrl = std::make_unique<ScannerBackupCtrl>(controlFile);
    if (!m_scannerBackupCtrl) {
        ERRLOG("Create copy phase control file instance failed!");
        return Module::FAILED;
    }
    int ret = m_scannerBackupCtrl->Open(NAS_CTRL_FILE_OPEN_MODE_READ);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        ERRLOG("failed to open control file: %s", controlFile.c_str());
        return Module::FAILED;
    }
    if (FillStatsFromControlHeaderV10() != Module::SUCCESS) {
        ERRLOG("failed to fill stats from control file: %s", controlFile.c_str());
        return Module::FAILED;
    }
    INFOLOG("FillStatsFromControlHeaderV10 success!");
    return Module::SUCCESS;
}

int CopyControlFileReader::OpenControlFileV20(const std::string& controlFile)
{
    m_copyCtrlParser = std::make_unique<CopyCtrlParser>(controlFile);
    if (m_copyCtrlParser == nullptr) {
        return Module::FAILED;
    }
    CTRL_FILE_RETCODE retVal = m_copyCtrlParser->Open(CTRL_FILE_OPEN_MODE::READ);
    if (retVal != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("failed to open control file: %s", controlFile.c_str());
        return Module::FAILED;
    }

    if (FillStatsFromControlHeader() != Module::SUCCESS) {
        ERRLOG("failed to fill stats from control file: %s", controlFile.c_str());
        return Module::FAILED;
    }

    m_readControlHeaderComplete = true;
    return Module::SUCCESS;
}

void CopyControlFileReader::MonitorReadControlHeader()
{
    int timeCount = 0;
    while (!m_readControlHeaderComplete) {
        std::this_thread::sleep_for(std::chrono::seconds(NUMBER_ONE));
        ++timeCount;
        if (timeCount >= READ_CONTROL_FILE_HEAD_TIMEOUT_SECONDS) {
            m_controlInfo->m_controlReaderFailed = true;
            ERRLOG("read control header timeout");
            return;
        }
    }
    return;
}

int CopyControlFileReader::ReadControlFileEntryAndProcess(CopyCtrlFileEntry& fileEntry, CopyCtrlDirEntry& dirEntry,
    ParentInfo& parentInfo)
{
    CTRL_FILE_RETCODE ret = m_copyCtrlParser->ReadEntry(fileEntry, dirEntry);
    if (ret == CTRL_FILE_RETCODE::READ_EOF) {
        if (m_backupParams.commonParams.orderOfFilenames != OrderOfRestore::OFF) {
            INFOLOG("Ordered restore file push fh to read Que");
            JudgeAndPushPriorityToReader();
        }
        return FINISH;
    }
    return ProcessControlFileEntry(dirEntry, fileEntry, parentInfo);
}

int CopyControlFileReader::ReadControlFileEntryAndProcessV10(ScannerBackupCtrlFileEntry& fileEntry,
    ScannerBackupCtrlDirEntry& dirEntry, ParentInfo& parentInfo)
{
    NAS_CTRL_FILE_RETCODE ret = m_scannerBackupCtrl->ReadEntry(fileEntry, dirEntry);
    if (ret == NAS_CTRL_FILE_RET_READ_EOF) {
        if (!m_scannerBackupCtrl) {
            m_scannerBackupCtrl->Close(NAS_CTRL_FILE_OPEN_MODE_READ);
        }
        if (!m_scannerBackupMeta) {
            m_scannerBackupMeta->Close(NAS_CTRL_FILE_OPEN_MODE_READ);
        }
        return FINISH;
    }
    return ProcessControlFileEntryV10(dirEntry, fileEntry, parentInfo);
}

int CopyControlFileReader::OpenMetaControlFile(const std::string& metaFile)
{
    if (m_backupParams.backupType == BackupType::RESTORE) {
        m_metaParser = std::make_unique<MetaParser>(metaFile);
    } else {
        m_metaParser = std::make_unique<MetaParser>(metaFile, true);
    }
    if (m_metaParser == nullptr) {
        return Module::FAILED;
    }
    CTRL_FILE_RETCODE retVal = m_metaParser->Open(CTRL_FILE_OPEN_MODE::READ);
    if (retVal != CTRL_FILE_RETCODE::SUCCESS) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int CopyControlFileReader::OpenMetaControlFileV10(const std::string& metaFile)
{
    m_scannerBackupMeta = std::make_unique<NasScanner::ScannerBackupMeta>(metaFile);
    if (!m_scannerBackupMeta) {
        ERRLOG("Create backup meta control file instance failed!");
        return Module::FAILED;
    }
    int ret = m_scannerBackupMeta->Open(NAS_CTRL_FILE_OPEN_MODE_READ);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        ERRLOG("Open meta control file failed");
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int CopyControlFileReader::OpenXMetaControlFile(const std::string& metaFile)
{
    m_xMetaParser = std::make_unique<XMetaParser>(metaFile);
    if (m_xMetaParser == nullptr) {
        ERRLOG("m_xMetaParser nulptr");
        return Module::FAILED;
    }
    CTRL_FILE_RETCODE retVal = m_xMetaParser->Open(CTRL_FILE_OPEN_MODE::READ);
    if (retVal != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("m_xMetaParser Open failed. Ret: %d: ", retVal);
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int CopyControlFileReader::ReadDirectoryMeta(DirMeta& dirMeta, uint64_t offset)
{
    CTRL_FILE_RETCODE ret = m_metaParser->ReadDirectoryMeta(dirMeta, offset);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int CopyControlFileReader::ReadFileMeta(FileMeta& fileMeta, uint64_t offset)
{
    CTRL_FILE_RETCODE ret = m_metaParser->ReadFileMeta(fileMeta, offset);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

string CopyControlFileReader::ParseAcl(const std::vector<XMetaField> &xmetalist)
{
    for (const XMetaField &xmeta : xmetalist) {
        if (xmeta.m_xMetaType == XMETA_TYPE::XMETA_TYPE_ACL) {
            return xmeta.m_value;
        }
    }
    return "";
}

int CopyControlFileReader::ReadDirectoryXMeta(FileHandle& fileHandle, const DirMeta& dirMeta)
{
    vector<XMetaField> entryList {};
    if (m_xMetaParser->ReadXMeta(entryList, dirMeta.m_xMetaFileOffset) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Failed to read xmeta file");
        return Module::FAILED;
    }
    fileHandle.m_file->m_obsKey = ParserUtils::ParseObjectKey(entryList);
    if (m_backupParams.commonParams.writeAcl) {
        /* For Windows/SMB */
        fileHandle.m_file->m_securityDescriptor = ParserUtils::ParseSecurityDescriptor(entryList);
        /* For Linux/NFS */
        fileHandle.m_file->m_defaultAclText     = ParserUtils::ParseDefaultAcl(entryList);
        if (dirMeta.type == static_cast<uint16_t>(MetaType::OBJECT)) {
            /* For object storage */
            fileHandle.m_file->m_aclText = ParseAcl(entryList);
        } else {
            /* For Linux/NFS */
            fileHandle.m_file->m_aclText = ParserUtils::ParseAccessAcl(entryList);
        }
    }
    if (m_backupParams.commonParams.writeExtendAttribute) {
        fileHandle.m_file->m_xattr          = ParserUtils::ParseXattr(entryList);
    }
    if (m_backupParams.commonParams.writeSparseFile) {
        fileHandle.m_file->m_sparse         = ParserUtils::ParseSparseInfo(entryList);
    }

    return Module::SUCCESS;
}

int CopyControlFileReader::ReadFileXMeta(FileHandle& fileHandle, const FileMeta& fileMeta)
{
    vector<XMetaField> entryList {};
    if (m_xMetaParser->ReadXMeta(entryList, fileMeta.m_xMetaFileOffset) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Failed to read xmeta file");
        return Module::FAILED;
    }
    // 设置一下ads stream num
    int count = std::count_if(entryList.begin(), entryList.end(), [](const XMetaField& entry) {
        return entry.m_xMetaType == Module::XMETA_TYPE::XMETA_TYPE_ADS_STREAM_NAME;
    });
    fileHandle.m_file->m_numOfStreams = count;
#ifdef _NAS
    for (auto entry : entryList) {
        if (entry.m_xMetaType == XMETA_TYPE::XMETA_TYPE_NFSFH && m_backupParams.scanAdvParams.useXmetaFileHandle) {
            int retCode = FSBackupUtils::SetSrcFileHandleForNfs(fileHandle, entry.m_value.length(),
                entry.m_value.c_str());
            if (retCode != Module::SUCCESS) {
                return Module::FAILED;
            }
        }
    }
#endif

    fileHandle.m_file->m_obsKey = ParserUtils::ParseObjectKey(entryList);

    if (m_backupParams.commonParams.writeAcl) {
        if (fileMeta.type == static_cast<uint16_t>(MetaType::OBJECT)) {
            /* For object storage */
            fileHandle.m_file->m_aclText = ParseAcl(entryList);
        } else {
            /* For Linux/NFS */
            fileHandle.m_file->m_aclText = ParserUtils::ParseAccessAcl(entryList);
        }
        /* For Windows/SMB */
        fileHandle.m_file->m_securityDescriptor = ParserUtils::ParseSecurityDescriptor(entryList);
    }
    if (m_backupParams.commonParams.writeSparseFile) {
        fileHandle.m_file->m_sparse = ParserUtils::ParseSparseInfo(entryList);
    }
    if (m_backupParams.commonParams.writeExtendAttribute) {
        fileHandle.m_file->m_xattr = ParserUtils::ParseXattr(entryList);
    }

#ifdef WIN32
    /* reuse the m_attr to store some XMeta pair for Windows Backup. These XMeta will only used for file */
    std::string win32SymlinkTarget = ParserUtils::ParseSymbolicLinkTargetPath(entryList);
    if (!win32SymlinkTarget.empty()) {
        fileHandle.m_file->m_xattr.emplace_back(EXTEND_ATTR_KEY_WIN32_SYMBOLIC_TARGET, win32SymlinkTarget);
    }
    std::string win32JunctionTarget = ParserUtils::ParseJunctionPointTargetPath(entryList);
    if (!win32JunctionTarget.empty()) {
        fileHandle.m_file->m_xattr.emplace_back(EXTEND_ATTR_KEY_WIN32_JUNCTION_TARGET, win32JunctionTarget);
    }
#endif
    return Module::SUCCESS;
}

string CopyControlFileReader::GetMetaFile(std::string metaFileName)
{
    return m_backupParams.scanAdvParams.metaFilePath + PATH_SEP + metaFileName;
}

string CopyControlFileReader::GetXMetaFile(uint64_t xMetaFileIndex)
{
    return m_backupParams.scanAdvParams.metaFilePath + PATH_SEP + XMETA_FILENAME_PFX + to_string(xMetaFileIndex);
}

string CopyControlFileReader::GetOpenedMetaFileName()
{
    if (m_metaParser == nullptr) {
        return "";
    }
    return m_metaParser->GetFileName();
}

string CopyControlFileReader::GetOpenedMetaFileNameV10()
{
    if (m_scannerBackupMeta == nullptr) {
        return "";
    }
    return m_scannerBackupMeta->GetFileName();
}

string CopyControlFileReader::GetOpenedXMetaFileName()
{
    if (m_xMetaParser == nullptr) {
        return "";
    }
    return m_xMetaParser->GetFileName();
}

int CopyControlFileReader::ProcessDirEntry(ParentInfo &parentInfo, const CopyCtrlDirEntry& dirEntry,
    FileHandle& fileHandle)
{
    parentInfo.dirName = dirEntry.m_dirName;
    if (dirEntry.m_mode == Module::CTRL_ENTRY_MODE_DATA_DELETED) {
        return DIR_SKIP;
    }
    if (parentInfo.metaFileName.empty()) {
        parentInfo.metaFileName = GetMetaFile(dirEntry.m_metaFileName);
        if (OpenMetaControlFile(parentInfo.metaFileName) != Module::SUCCESS) {
            m_failureRecorder->RecordFailure(dirEntry.m_dirName, "Failed to open meta file!");
            return Module::FAILED;
        }
    }

    if (parentInfo.metaFileName != GetMetaFile(dirEntry.m_metaFileName)) {
        parentInfo.metaFileName = GetMetaFile(dirEntry.m_metaFileName);
        if (OpenMetaControlFile(parentInfo.metaFileName) != Module::SUCCESS) {
            m_failureRecorder->RecordFailure(dirEntry.m_dirName, "Failed to open meta file!");
            return Module::FAILED;
        }
    }
    DirMeta dirMeta;
    if (ReadDirectoryMeta(dirMeta, dirEntry.metaFileOffset) != Module::SUCCESS) {
        m_failureRecorder->RecordFailure(dirEntry.m_dirName, "Failed to read meta file!");
        return Module::FAILED;
    }
    fileHandle.m_file->m_fileName = dirEntry.m_dirName;
    fileHandle.m_file->m_scannermode = dirEntry.m_mode;
    fileHandle.m_file->m_fileCount = dirEntry.m_fileCount;

    FillDirMetaData(fileHandle, dirMeta);

    if (dirMeta.m_xMetaFileOffset != 0) {
        string xMetaFileName = GetXMetaFile(dirMeta.m_xMetaFileIndex);
        if (GetOpenedXMetaFileName() != xMetaFileName) {
            if (OpenXMetaControlFile(xMetaFileName) != Module::SUCCESS) {
                m_failureRecorder->RecordFailure(dirEntry.m_dirName, "Failed to open xmeta file!");
                ERRLOG("Failed to open xmeta file");
                return Module::FAILED;
            }
        }

        if (ReadDirectoryXMeta(fileHandle, dirMeta) != Module::SUCCESS) {
            m_failureRecorder->RecordFailure(dirEntry.m_dirName, "Failed to read xmeta file!");
            return Module::FAILED;
        }
    }

    return Module::SUCCESS;
}

int CopyControlFileReader::ProcessDirEntryV10(ParentInfo &parentInfo, const ScannerBackupCtrlDirEntry& dirEntry,
    FileHandle& fileHandle)
{
    parentInfo.dirName = dirEntry.m_dirName;
    if (dirEntry.m_mode == Module::CTRL_ENTRY_MODE_DATA_DELETED) {
        return DIR_SKIP;
    }
    if (parentInfo.metaFileName.empty()) {
        parentInfo.metaFileName = GetMetaFile(dirEntry.m_metaFileName);
        if (OpenMetaControlFileV10(parentInfo.metaFileName) != Module::SUCCESS) {
            return Module::FAILED;
        }
    }
    if (dirEntry.m_dirName == ".") {
        return DIR_SKIP;
    }
    if (parentInfo.metaFileName != GetMetaFile(dirEntry.m_metaFileName)) {
        parentInfo.metaFileName = GetMetaFile(dirEntry.m_metaFileName);
        if (OpenMetaControlFileV10(parentInfo.metaFileName) != Module::SUCCESS) {
            return Module::FAILED;
        }
    }

    NasScanner::DirectoryMeta dirMeta {};
    int ret = m_scannerBackupMeta->ReadDirectoryMeta(dirMeta, dirEntry.metaFileReadLen, dirEntry.metaFileOffset);
    if (ret == NAS_CTRL_FILE_RET_FAILED) {
        ERRLOG("ReadDirectoryMeta failed for : %s", dirEntry.m_dirName.c_str());
        return Module::FAILED;
    }

    fileHandle.m_file->m_fileName = dirEntry.m_dirName;
    fileHandle.m_file->m_scannermode = dirEntry.m_mode;
    fileHandle.m_file->m_fileCount = dirEntry.m_fileCount;

    FillDirMetaDataV10(fileHandle, dirMeta);
    return Module::SUCCESS;
}

int CopyControlFileReader::ProcessFileEntry(ParentInfo& parentInfo, const CopyCtrlFileEntry& fileEntry,
    FileHandle& fileHandle)
{
    if (fileEntry.m_mode == Module::CTRL_ENTRY_MODE_DATA_DELETED) {
        return FILE_SKIP;
    }
    fileHandle.m_file->m_fileName = parentInfo.dirName + CTRL_FILE_PATH_SEPARATOR + fileEntry.m_fileName;
    if (fileEntry.m_metaFileName.empty()) {
        m_failureRecorder->RecordFailure(fileHandle.m_file->m_fileName, "Failed to read meta file name!");
        return Module::FAILED;
    }
    if (GetOpenedMetaFileName() != GetMetaFile(fileEntry.m_metaFileName)) {
        parentInfo.metaFileName = GetMetaFile(fileEntry.m_metaFileName);
        if (OpenMetaControlFile(parentInfo.metaFileName) != Module::SUCCESS) {
            m_failureRecorder->RecordFailure(fileHandle.m_file->m_fileName, "Failed to open meta file!");
            return Module::FAILED;
        }
    }
    FileMeta fileMeta;
    if (ReadFileMeta(fileMeta, fileEntry.metaFileOffset) != Module::SUCCESS) {
        m_failureRecorder->RecordFailure(fileHandle.m_file->m_fileName, "Failed to read meta file!");
        return Module::FAILED;
    }
    FillFileMetaData(fileHandle, fileMeta, fileEntry);
    if (fileMeta.m_xMetaFileOffset != 0) {
        string xMetafilename = GetXMetaFile(fileMeta.m_xMetaFileIndex);
        if (GetOpenedXMetaFileName() != xMetafilename) {
            if (OpenXMetaControlFile(xMetafilename) != Module::SUCCESS) {
                ERRLOG("Failed to open xmeta file, %s", xMetafilename.c_str());
                m_failureRecorder->RecordFailure(fileHandle.m_file->m_fileName, "Failed to open xmeta file!");
                return Module::FAILED;
            }
        }

        /* Get fh from XMetaFile */
        if (ReadFileXMeta(fileHandle, fileMeta) != Module::SUCCESS) {
            m_failureRecorder->RecordFailure(fileHandle.m_file->m_fileName, "Failed to read xmeta file!");
            return Module::FAILED;
        }
    }
    uint32_t mode = fileHandle.m_file->m_mode;
    // if restore and aggregae enabled and file size <= maxFileSizeToAggregate
    // KB_IN_BYTES 1024 multiplication need to remove once PM changes
    if ((m_backupParams.backupType == BackupType::RESTORE) &&
        (m_backupParams.commonParams.backupDataFormat == BackupDataFormat::AGGREGATE) &&
        (fileHandle.m_file->m_size <= (m_backupParams.commonParams.maxFileSizeToAggregate * KB_IN_BYTES)) &&
        (!FSBackupUtils::IsSpecialFile(mode))) {
        // as this is aggreagted file during backup, so data will be there in zip file
        // so make it the nomal file FileDisk as readed or add new state as AGGREGATED
        // so reader module just pass it to the aggregator module
        fileHandle.m_file->SetSrcState(FileDescState::AGGREGATED);
        DBGLOG(" FileDescState::AGGREGATED. FileName: %s, size: %llu", fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_file->m_size);
        PushFileHandleToAggregator(fileHandle);
    }
    return Module::SUCCESS;
}

int CopyControlFileReader::ProcessFileEntryV10(ParentInfo& parentInfo, const ScannerBackupCtrlFileEntry& fileEntry,
    FileHandle& fileHandle)
{
    if (fileEntry.m_mode == Module::CTRL_ENTRY_MODE_DATA_DELETED) {
        return FILE_SKIP;
    }
    if (fileEntry.m_metaFileName.empty()) {
        return Module::FAILED;
    }
    if (GetOpenedMetaFileNameV10() != GetMetaFile(fileEntry.m_metaFileName)) {
        parentInfo.metaFileName = GetMetaFile(fileEntry.m_metaFileName);
        if (OpenMetaControlFileV10(parentInfo.metaFileName) != Module::SUCCESS) {
            return Module::FAILED;
        }
    }

    NasScanner::FileMeta fileMeta;
    int ret = m_scannerBackupMeta->ReadFileMeta(fileMeta, fileEntry.metaFileReadLen, fileEntry.metaFileOffset);
    if (ret == NAS_CTRL_FILE_RET_FAILED) {
        ERRLOG("Read file meta failed for : %s, %d, %d", fileEntry.m_fileName.c_str(), fileEntry.metaFileReadLen,
            fileEntry.metaFileOffset);
        return Module::FAILED;
    }

    fileHandle.m_file->m_fileName = parentInfo.dirName + CTRL_FILE_PATH_SEPARATOR + fileEntry.m_fileName;
    fileHandle.m_file->m_scannermode = fileEntry.m_mode;

    FillFileMetaDataV10(fileHandle, fileMeta);
    uint32_t mode = fileHandle.m_file->m_mode;
    // if restore and aggregae enabled and file size <= maxFileSizeToAggregate
    // KB_IN_BYTES 1024 multiplication need to remove once PM changes
    if ((m_backupParams.backupType == BackupType::RESTORE) &&
        (m_backupParams.commonParams.backupDataFormat == BackupDataFormat::AGGREGATE) &&
        (fileHandle.m_file->m_size <= (m_backupParams.commonParams.maxFileSizeToAggregate * KB_IN_BYTES)) &&
        (!FSBackupUtils::IsSpecialFile(mode))) {
        // as this is aggreagted file during backup, so data will be there in zip file
        // so make it the nomal file FileDisk as readed or add new state as AGGREGATED
        // so reader module just pass it to the aggregator module
        fileHandle.m_file->SetSrcState(FileDescState::AGGREGATED);
        DBGLOG(" FileDescState::AGGREGATED. File: %s, size: %llu, mode: %d", fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_file->m_size, mode);
        PushFileHandleToAggregator(fileHandle);
    }

    return Module::SUCCESS;
}

void CopyControlFileReader::FillDirMetaData(FileHandle& fileHandle, const DirMeta& dirMeta)
{
    string fullPath = fileHandle.m_file->m_fileName;
    string parentDirName = FSBackupUtils::GetParentDir(fullPath);
    fileHandle.m_file->m_dirName = parentDirName;
    if (parentDirName != PATH_SEP) { // smb控制文件的根目录是/, 需要特殊处理，posix和nfs控制文件根目录是.，不影响
        FSBackupUtils::RemoveTrailingSlashes(parentDirName);
    }
    fileHandle.m_file->m_onlyFileName = fullPath.substr(parentDirName.length() + NUMBER_ONE,
        fullPath.length() - parentDirName.length() - NUMBER_ONE);
    fileHandle.m_file->SetFlag(IS_DIR);
    fileHandle.m_file->m_size    = dirMeta.m_size;
    fileHandle.m_file->m_atime   = dirMeta.m_atime;
    fileHandle.m_file->m_btime   = dirMeta.m_btime;
    fileHandle.m_file->m_mtime   = dirMeta.m_mtime;
    fileHandle.m_file->m_ctime   = dirMeta.m_ctime;
    fileHandle.m_file->m_inode   = dirMeta.m_inode;
    fileHandle.m_file->m_mode    = dirMeta.m_mode;
    fileHandle.m_file->m_uid     = dirMeta.m_uid;
    fileHandle.m_file->m_gid     = dirMeta.m_gid;
    fileHandle.m_file->m_fileAttr = dirMeta.m_attr;
    if (dirMeta.type == static_cast<uint16_t>(MetaType::OBJECT)) {
        fileHandle.m_file->m_type = FileType::OBJECT;
    }
    if (!fileHandle.m_file->m_aclText.empty() || !fileHandle.m_file->m_securityDescriptor.empty()) {
        fileHandle.m_file->SetFlag(ACL_EXIST);
    }
#ifndef WIN32
    if (fileHandle.m_file->m_fileName == ".") {
        DBGLOG("This is root dir. Set the dir flag in mode");
        fileHandle.m_file->m_mode = S_IFDIR;
    }
#endif
    return;
}

void CopyControlFileReader::FillDirMetaDataV10(FileHandle& fileHandle, const NasScanner::DirectoryMeta& dirMeta)
{
    string fullPath = fileHandle.m_file->m_fileName;
    string parentDirName = FSBackupUtils::GetParentDir(fullPath);
    fileHandle.m_file->m_dirName = parentDirName;
    if (parentDirName != PATH_SEP) { // smb控制文件的根目录是/, 需要特殊处理，posix和nfs控制文件根目录是.，不影响
        FSBackupUtils::RemoveTrailingSlashes(parentDirName);
    }
    fileHandle.m_file->m_onlyFileName = fullPath.substr(parentDirName.length() + NUMBER_ONE,
        fullPath.length() - parentDirName.length() - NUMBER_ONE);
    fileHandle.m_file->SetFlag(IS_DIR);
    fileHandle.m_file->m_inode   = dirMeta.m_inode;
    fileHandle.m_file->m_size    = dirMeta.m_size;
    fileHandle.m_file->m_atime   = dirMeta.m_atime;
    fileHandle.m_file->m_mtime   = dirMeta.m_mtime;
    fileHandle.m_file->m_ctime   = dirMeta.m_ctime;
    fileHandle.m_file->m_mode    = dirMeta.m_mode;
    fileHandle.m_file->m_uid     = dirMeta.m_uid;
    fileHandle.m_file->m_gid     = dirMeta.m_gid;
    if (!dirMeta.m_aclText.empty()) {
        fileHandle.m_file->SetFlag(ACL_EXIST);
    }
}

void CopyControlFileReader::FillFileMetaData(FileHandle& fileHandle, const FileMeta& fileMeta,
    const CopyCtrlFileEntry& fileEntry)
{
    fileHandle.m_file->m_scannermode = fileEntry.m_mode;
    try {
        fileHandle.m_file->m_metaFileIndex = stoi(fileEntry.m_metaFileName.substr(METAFILE_NAME_PREFIX_LEN));
    } catch (...) {
        WARNLOG("invalid convert: %s, %s", fileEntry.m_metaFileName.c_str(), fileHandle.m_file->m_fileName.c_str());
    }
    fileHandle.m_file->m_metaFileReadLen = fileEntry.metaFileReadLen;
    fileHandle.m_file->m_metaFileOffset = fileEntry.metaFileOffset;

    string fullPath = fileHandle.m_file->m_fileName;
    string parentDirName = FSBackupUtils::GetParentDir(fullPath);
    fileHandle.m_file->m_dirName = parentDirName;
    if (parentDirName != PATH_SEP) { // 当根目录是分隔符时，就不需要去掉目录最后的分隔符了
        FSBackupUtils::RemoveTrailingSlashes(parentDirName);
    }
    fileHandle.m_file->m_onlyFileName = fullPath.substr(parentDirName.length() + 1,
                                                        fullPath.length() - parentDirName.length() - 1);
    FSBackupUtils::RemoveLeadingSlashes(fileHandle.m_file->m_onlyFileName);
    fileHandle.m_file->ClearFlag(IS_DIR);
    fileHandle.m_file->m_inode   = fileMeta.m_inode;
    fileHandle.m_file->m_size    = fileMeta.m_size;
    fileHandle.m_file->m_rdev    = fileMeta.m_rdev;
    fileHandle.m_file->m_atime   = fileMeta.m_atime;
    fileHandle.m_file->m_btime   = fileMeta.m_btime;
    fileHandle.m_file->m_mtime   = fileMeta.m_mtime;
    fileHandle.m_file->m_ctime   = fileMeta.m_ctime;
    fileHandle.m_file->m_mode    = fileMeta.m_mode;
    fileHandle.m_file->m_uid     = fileMeta.m_uid;
    fileHandle.m_file->m_gid     = fileMeta.m_gid;
    fileHandle.m_file->m_nlink   = fileMeta.m_nlink;
    fileHandle.m_file->m_fileAttr = fileMeta.m_attr;
    fileHandle.m_file->m_xMetaFileIndex = fileMeta.m_xMetaFileIndex;
    fileHandle.m_file->m_xMetaFileOffset = fileMeta.m_xMetaFileOffset;
    if (fileMeta.type == static_cast<uint16_t>(MetaType::OBJECT)) {
        fileHandle.m_file->m_type = FileType::OBJECT;
    }

    if (!fileHandle.m_file->m_aclText.empty() || !fileHandle.m_file->m_securityDescriptor.empty()) {
        fileHandle.m_file->SetFlag(ACL_EXIST);
    }

    return;
}

int CopyControlFileReader::FillFileMetaDataV10(FileHandle& fileHandle, const NasScanner::FileMeta& fileMeta)
{
    string fullPath = fileHandle.m_file->m_fileName;
    string parentDirName = FSBackupUtils::GetParentDir(fullPath);
    if (parentDirName != PATH_SEP) {
        FSBackupUtils::RemoveTrailingSlashes(parentDirName);
    }
    fileHandle.m_file->m_dirName = parentDirName;
    fileHandle.m_file->m_onlyFileName = fullPath.substr(parentDirName.length() + 1,
        fullPath.length() - parentDirName.length() - 1);
    FSBackupUtils::RemoveLeadingSlashes(fileHandle.m_file->m_onlyFileName);
    fileHandle.m_file->ClearFlag(IS_DIR);
    fileHandle.m_file->m_atime   = fileMeta.m_atime;
    fileHandle.m_file->m_mtime   = fileMeta.m_mtime;
    fileHandle.m_file->m_ctime   = fileMeta.m_ctime;
    fileHandle.m_file->m_size    = fileMeta.m_size;
    fileHandle.m_file->m_rdev    = fileMeta.m_rdev;
    fileHandle.m_file->m_inode   = fileMeta.m_inode;
    fileHandle.m_file->m_mode    = fileMeta.m_mode;
    fileHandle.m_file->m_uid     = fileMeta.m_uid;
    fileHandle.m_file->m_gid     = fileMeta.m_gid;
    fileHandle.m_file->m_nlink   = fileMeta.m_nlink;
    fileHandle.m_file->m_aclText = fileMeta.m_aclText;

    if (!fileHandle.m_file->m_aclText.empty()) {
        fileHandle.m_file->SetFlag(ACL_EXIST);
    }
#ifdef _NAS
    if (fileMeta.m_fh.len > 0) {
        int retVal = FSBackupUtils::SetSrcFileHandleForNfs(fileHandle, fileMeta.m_fh.len, fileMeta.m_fh.value);
        if (retVal != Module::SUCCESS) {
            return Module::FAILED;
        }
    }
#endif
    return Module::SUCCESS;
}

int CopyControlFileReader::FillStatsFromControlHeader()
{
    CopyCtrlParser::Header header;

    if (m_copyCtrlParser == nullptr) {
        ERRLOG("copyCtrlParser is nullptr");
        return Module::FAILED;
    }

    if (m_copyCtrlParser->GetHeader(header) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Get control file header failed!");
        return Module::FAILED;
    }

    m_controlInfo->m_noOfFilesToBackup += header.stats.noOfFiles;
    m_controlInfo->m_noOfDirToBackup += header.stats.noOfDirs;
    m_controlInfo->m_noOfBytesToBackup += header.stats.dataSize;

    return Module::SUCCESS;
}

int CopyControlFileReader::FillStatsFromControlHeaderV10()
{
    ScannerBackupCtrlHeader ctrlHeader {};
    if (!m_scannerBackupCtrl) {
        ERRLOG("m_scannerBackupCtrl is nullptr");
        return Module::FAILED;
    }

    if (m_scannerBackupCtrl->GetHeader(ctrlHeader) != NAS_CTRL_FILE_RET_SUCCESS) {
        ERRLOG("Get control file header failed!");
        return Module::FAILED;
    }

    m_controlInfo->m_noOfDirToBackup += ctrlHeader.stats.noOfDirs;
    m_controlInfo->m_noOfFilesToBackup += ctrlHeader.stats.noOfFiles;
    m_controlInfo->m_noOfBytesToBackup += ctrlHeader.stats.dataSize;

    return Module::SUCCESS;
}

bool CopyControlFileReader::IsWindowsStyleEngine(const BackupIOEngine& engine) const
{
    // check if backup engine support ADS (WIN32_IO or LIBSMB)
    return engine == BackupIOEngine::LIBSMB || engine == BackupIOEngine::WIN32_IO;
}
