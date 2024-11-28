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
#include "BackupMgr.h"
#include "Copy.h"
#include "Delete.h"
#include "Hardlink.h"
#include "Dir.h"
#include "MergeSqliteDB.h"
#ifdef _NAS
#include "AntiRansomware.h"
#endif

namespace {
    constexpr auto BACKUP_MGR_MODULE = "BACKUP_MGR";
    constexpr int BACKUP_LOG_COUNT = 100;
    constexpr int BACKUP_LOG_MAX_SIZE = 30;
}

using namespace std;
using namespace FS_Backup;

unique_ptr<Backup> BackupMgr::CreateBackupInst(const BackupParams& backupParams)
{
    unique_ptr<Backup> backupInst = nullptr;
    if (!ValidateBackupParams(backupParams)) {
        return nullptr;
    }
    switch (backupParams.phase) {
        case BackupPhase::COPY_STAGE:
            backupInst = make_unique<Copy>(backupParams);
            break;
        case BackupPhase::DELETE_STAGE:
            backupInst = make_unique<Delete>(backupParams);
            break;
        case BackupPhase::HARDLINK_STAGE:
            backupInst = make_unique<Hardlink>(backupParams);
            break;
        case BackupPhase::DIR_STAGE:
            backupInst = make_unique<Dir>(backupParams);
            break;
#ifdef _NAS
        case BackupPhase::ANTI_STAGE:
            backupInst = make_unique<AntiRansomware>(backupParams);
            break;
#endif
        default:
            break;
    }
    return backupInst;
}

BackupPlatform BackupMgr::GetBackupPlatform(BackupIOEngine ioEngine)
{
    BackupPlatform platForm = BackupPlatform::UNKNOWN_PLATFORM;
    switch (ioEngine) {
        case BackupIOEngine::LIBNFS:
        case BackupIOEngine::POSIX:
        case BackupIOEngine::POSIXAIO:
        case BackupIOEngine::LIBAIO:
            platForm = BackupPlatform::UNIX;
            break;
        case BackupIOEngine::WINDOWSAIO:
        case BackupIOEngine::WIN32_IO:
        case BackupIOEngine::LIBSMB:
            platForm = BackupPlatform::WINDOWS;
            break;
        case BackupIOEngine::OBJECTSTORAGE:
            platForm = BackupPlatform::OBJECT;
            break;
        case BackupIOEngine::ARCHIVE_CLIENT:
        case BackupIOEngine::LIBS3IO:
        case BackupIOEngine::NFS_ANTI_ANSOMWARE:
        default:
            break;
    }
    return platForm;
}

bool BackupMgr::ValidateBackupParams(const BackupParams& backupParams)
{
    PrintBackupParams(backupParams);

    if (backupParams.backupType == BackupType::RESTORE &&
        backupParams.commonParams.restoreReplacePolicy == RestoreReplacePolicy::NONE) {
        ERRLOG("Restore policy is none");
        return false;
    }

    BackupPlatform srcPlatform = GetBackupPlatform(backupParams.srcEngine);
    BackupPlatform dstPlatform = GetBackupPlatform(backupParams.dstEngine);
    if (((srcPlatform == BackupPlatform::UNIX) && (dstPlatform == BackupPlatform::WINDOWS)) ||
        ((srcPlatform == BackupPlatform::WINDOWS) && (dstPlatform == BackupPlatform::UNIX))) {
        ERRLOG("platform validate failed");
        return false;
    }

    if ((backupParams.commonParams.writeExtendAttribute) && (dstPlatform == BackupPlatform::WINDOWS)) {
        ERRLOG("windows platform does not support extend attribute");
        return false;
    }

    if ((backupParams.commonParams.writeExtendAttribute || backupParams.commonParams.writeAcl) &&
        (srcPlatform == BackupPlatform::OBJECT)) {
        ERRLOG("object storage does not support extend attribute and acl");
        return false;
    }

    if ((backupParams.commonParams.backupDataFormat == BackupDataFormat::AGGREGATE) &&
        (backupParams.commonParams.maxAggregateFileSize < backupParams.commonParams.maxFileSizeToAggregate)) {
        ERRLOG("max aggr file size should large than max original file size");
        return false;
    }

    return true;
}

#ifdef _OBS
void BackupMgr::PrintBackupParamsObject(std::shared_ptr<BackupAdvanceParams> backupAdvParams)
{
    ObjectBackupAdvanceParams *advParams = dynamic_pointer_cast<ObjectBackupAdvanceParams>(backupAdvParams).get();
    INFOLOG("Data Path: %s", advParams->dataPath.c_str());
}
#endif

void BackupMgr::PrintBackupParamsPosix(std::shared_ptr<BackupAdvanceParams> backupAdvParams)
{
    HostBackupAdvanceParams *advParams = dynamic_pointer_cast<HostBackupAdvanceParams>(backupAdvParams).get();
    INFOLOG("Data Path: %s", advParams->dataPath.c_str());
}

void BackupMgr::PrintBackupParamsWin32(std::shared_ptr<BackupAdvanceParams> backupAdvParams)
{
    HostBackupAdvanceParams *advParams = dynamic_pointer_cast<HostBackupAdvanceParams>(backupAdvParams).get();
    INFOLOG("Data Path: %s", advParams->dataPath.c_str());
}

void BackupMgr::PrintBackupParamsLibnfs(std::shared_ptr<BackupAdvanceParams> backupAdvParams)
{
    LibnfsBackupAdvanceParams *advParams = dynamic_pointer_cast<LibnfsBackupAdvanceParams>(backupAdvParams).get();
    INFOLOG("Protected Nas Share Ip: %s", advParams->ip.c_str());
    INFOLOG("Protected Nas Share Path: %s", advParams->sharePath.c_str());
    INFOLOG("Backup Nas Share Ip: %s", advParams->ip.c_str());
    INFOLOG("Backup Nas Share Path: %s", advParams->sharePath.c_str());
    INFOLOG("NAS Protocol version: %s", advParams->protocolVersion.c_str());
    INFOLOG("Job Start Time: %d", advParams->jobStartTime);
    INFOLOG("Delete Job Start Time: %d", advParams->deleteJobStartTime);
    INFOLOG("Max Async Req Cnt: %d", advParams->maxPendingAsyncReqCnt);
    INFOLOG("Min Async Req Cnt: %d", advParams->minPendingAsyncReqCnt);
    INFOLOG("Max Write Req Cnt: %d", advParams->maxPendingWriteReqCnt);
    INFOLOG("Min Write Req Cnt: %d", advParams->minPendingWriteReqCnt);
    INFOLOG("Max Read Req Cnt: %d", advParams->maxPendingReadReqCnt);
    INFOLOG("Min Read Req Cnt: %d", advParams->minPendingReadReqCnt);
    INFOLOG("Max Server Check Sleep time: %d", advParams->serverCheckSleepTime);
    INFOLOG("Max Server Check retry: %d", advParams->serverCheckRetry);
}

void BackupMgr::PrintBackupParamsLibsmb(std::shared_ptr<BackupAdvanceParams> backupAdvParams)
{
    LibsmbBackupAdvanceParams *advParams = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(backupAdvParams).get();
    INFOLOG("Nas Share Ip: %s", advParams->server.c_str());
    INFOLOG("Nas Share Path: %s", advParams->share.c_str());
    INFOLOG("Nas Share domain: %s", advParams->domain.c_str());
    INFOLOG("Nas Share user: %s", advParams->user.c_str());
    INFOLOG("Nas Share krb5CcacheFile: %s", advParams->krb5CcacheFile.c_str());
    INFOLOG("Nas Share krb5ConfigFile: %s", advParams->krb5ConfigFile.c_str());
    INFOLOG("Nas Share encryption: %d", static_cast<int>(advParams->encryption));
    INFOLOG("Nas Share sign: %d", static_cast<int>(advParams->sign));
    INFOLOG("Nas Share timeout: %d", advParams->timeout);
    INFOLOG("Nas Share authType: %s", advParams->authType.c_str());
    INFOLOG("Nas Share version: %s", advParams->version.c_str());
}

void BackupMgr::PrintBackupParamsAntiRansomware(std::shared_ptr<BackupAdvanceParams> backupAdvParams)
{
    NfsAntiRansomwareAdvanceParams *advParams {nullptr};
    advParams = dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupAdvParams).get();
    INFOLOG("Nas Share Ip: %s", advParams->ip.c_str());
    INFOLOG("Nas Share Path: %s", advParams->sharePath.c_str());
    INFOLOG("Atime To Modified: %d", advParams->atime);
    INFOLOG("Mode To Modified: %s", advParams->mode.c_str());
    INFOLOG("Anti-Ransomware Type: %d", advParams->backupAntiType);
    INFOLOG("NAS Protocol version: %s", advParams->protocolVersion.c_str());
    INFOLOG("Max Async Req Cnt: %d", advParams->maxPendingAsyncReqCnt);
    INFOLOG("Min Async Req Cnt: %d", advParams->minPendingAsyncReqCnt);
    INFOLOG("Max Write Req Cnt: %d", advParams->maxPendingWriteReqCnt);
    INFOLOG("Min Write Req Cnt: %d", advParams->minPendingWriteReqCnt);
    INFOLOG("Max Read Req Cnt: %d", advParams->maxPendingReadReqCnt);
    INFOLOG("Min Read Req Cnt: %d", advParams->minPendingReadReqCnt);
    INFOLOG("Max Server Check Sleep time: %d", advParams->serverCheckSleepTime);
    INFOLOG("Max Server Check retry: %d", advParams->serverCheckRetry);
}

void BackupMgr::PrintBackupSrcParams(const BackupParams &backupParams)
{
    INFOLOG("Backup Src Config Parameters - Advance: %d", static_cast<int>(backupParams.srcEngine));
    std::map<BackupIOEngine, PrintBackupAdvanceParams> srcAdvParams = {
        {BackupIOEngine::POSIX, PrintBackupParamsPosix},
        {BackupIOEngine::LIBNFS, PrintBackupParamsLibnfs},
        {BackupIOEngine::LIBSMB, PrintBackupParamsLibsmb},
        {BackupIOEngine::WIN32_IO, PrintBackupParamsWin32},
        {BackupIOEngine::NFS_ANTI_ANSOMWARE, PrintBackupParamsAntiRansomware},
#ifdef _OBS
        {BackupIOEngine::OBJECTSTORAGE, PrintBackupParamsObject},
#endif
        {BackupIOEngine::POSIXAIO, nullptr},
        {BackupIOEngine::WINDOWSAIO, nullptr},
        {BackupIOEngine::LIBAIO, nullptr},
        {BackupIOEngine::ARCHIVE_CLIENT, nullptr},
        {BackupIOEngine::LIBS3IO, nullptr}
    };

    auto it = srcAdvParams.find(backupParams.srcEngine);
    if (it == srcAdvParams.end()) {
        ERRLOG("UnKnown Engine");
        return;
    }
    if (it->second != nullptr) {
        it->second(backupParams.srcAdvParams);
    }
    return;
}

void BackupMgr::PrintBackupDstParams(const BackupParams &backupParams)
{
    INFOLOG("Backup Dst Config Parameters - Advance: %d", static_cast<int>(backupParams.dstEngine));
    std::map<BackupIOEngine, PrintBackupAdvanceParams> dstAdvParams = {
        {BackupIOEngine::POSIX, PrintBackupParamsPosix},
        {BackupIOEngine::LIBNFS, PrintBackupParamsLibnfs},
        {BackupIOEngine::LIBSMB, PrintBackupParamsLibsmb},
        {BackupIOEngine::WIN32_IO, PrintBackupParamsWin32},
        {BackupIOEngine::NFS_ANTI_ANSOMWARE, PrintBackupParamsAntiRansomware},
#ifdef _OBS
        {BackupIOEngine::OBJECTSTORAGE, PrintBackupParamsObject},
#endif
        {BackupIOEngine::POSIXAIO, nullptr},
        {BackupIOEngine::WINDOWSAIO, nullptr},
        {BackupIOEngine::LIBAIO, nullptr},
        {BackupIOEngine::ARCHIVE_CLIENT, nullptr},
        {BackupIOEngine::LIBS3IO, nullptr}
    };
    auto it = dstAdvParams.find(backupParams.dstEngine);
    if (it == dstAdvParams.end()) {
        ERRLOG("UnKnown Engine");
        return;
    }
    if (it->second != nullptr) {
        it->second(backupParams.dstAdvParams);
    }
    return;
}

void BackupMgr::PrintBackupParams(const BackupParams &backupParams)
{
    INFOLOG("Backup Config Parameters - Common");
    INFOLOG("Source Engine: %d", (int)backupParams.srcEngine);
    INFOLOG("Destination Engine: %d", (int)backupParams.dstEngine);
    INFOLOG("Backup Phase: %d", (int)backupParams.phase);
    INFOLOG("Backup Type: %d", (int)backupParams.backupType);
    INFOLOG("Scan meta path: %s", backupParams.scanAdvParams.metaFilePath.c_str());
    INFOLOG("Replace Policy: %d", (int)backupParams.commonParams.restoreReplacePolicy);
    INFOLOG("Write sparse file: %d", (int)backupParams.commonParams.writeSparseFile);
    INFOLOG("RW BlockSz: %d", backupParams.commonParams.blockSize);
    INFOLOG("Max Total BlockBuffer Cnt: %d", backupParams.commonParams.maxBufferCnt);
    INFOLOG("Max Total BlockBuffer Size: %d", backupParams.commonParams.maxBufferSize);
    INFOLOG("Max Server Check Count: %d", backupParams.commonParams.maxErrorFiles);

    PrintBackupSrcParams(backupParams);
    PrintBackupDstParams(backupParams);
}

int BackupMgr::MergedbFile(const std::string &metaPath, bool &isComplete)
{
    auto instance = make_shared<MergeSqliteDB>(metaPath);
    return instance->MergeAlldbFilesUnderSameDir(isComplete);
}

void InitLog(const char* fullLogPath, int logLevel)
{
    unsigned int iLogLevel = static_cast<unsigned int>(logLevel);
    unsigned int iLogCount = BACKUP_LOG_COUNT;
    unsigned int iLogMaxSize = BACKUP_LOG_MAX_SIZE;
    std::string backupLogName = "backup.log";
    Module::CLogger::GetInstance().Init(backupLogName.c_str(),
        fullLogPath,
        iLogLevel,
        iLogCount,
        iLogMaxSize);
}

/* This method is only used to build backup engine for posix host backup */
void* CreateBackupInst(const char* source, const char* destination, const char* metaPath, int phase, bool writeMeta)
{
    string src = source;
    string dst = destination;
    string meta = metaPath;
    void* backupHandle = nullptr;
    switch (phase) {
        case BACKUP_PHASE_COPY:
            backupHandle = new (nothrow)Copy(src, dst, meta, writeMeta);
            break;
        case BACKUP_PHASE_DELETE:
            backupHandle = new (nothrow)Delete(src, dst, meta, writeMeta);
            break;
        case BACKUP_PHASE_HARDLINK:
            backupHandle = new (nothrow)Hardlink(src, dst, meta, writeMeta);
            break;
        case BACKUP_PHASE_DIR:
            backupHandle = new (nothrow)Dir(src, dst, meta, writeMeta);
            break;
        default:
            break;
    }
    return backupHandle;
}

#ifdef _NAS
void* CreateAntiBackupInst(BackupConf backupConf)
{
    void* backupHandle = nullptr;
    BackupParams backupParams {};
    backupParams.phase = BackupPhase(backupConf.phase);
    backupParams.srcAdvParams = make_shared<NfsAntiRansomwareAdvanceParams>();
    backupParams.dstAdvParams = make_shared<NfsAntiRansomwareAdvanceParams>();
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->reqID = backupConf.reqID;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->backupAntiType
        = BackupAntiType(backupConf.antiParam.antiType);
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->atime
        = backupConf.antiParam.nfsAtime;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->mode
        = backupConf.antiParam.nfsMode;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->ip = backupConf.ip;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->sharePath = backupConf.sharePath;
#ifndef WIN32
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.srcAdvParams)->protocolVersion = NFS_V3;
#endif
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->reqID = backupConf.reqID;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->backupAntiType
        = BackupAntiType(backupConf.antiParam.antiType);
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->atime
        = backupConf.antiParam.nfsAtime;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->mode
        = backupConf.antiParam.nfsMode;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->ip = backupConf.ip;
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->sharePath = backupConf.sharePath;
#ifndef WIN32
    dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(backupParams.dstAdvParams)->protocolVersion = NFS_V3;

    if (backupConf.phase == BACKUP_PHASE_ANTI_ANSOMWARE) {
        backupParams.srcEngine = BackupIOEngine::NFS_ANTI_ANSOMWARE;
        backupParams.dstEngine = BackupIOEngine::NFS_ANTI_ANSOMWARE;
        backupHandle = new (nothrow)AntiRansomware(backupParams);
    } else {
        ERRLOG("Not included Phase.");
    }
#endif
    return backupHandle;
}
#endif

int ConfigureThreadPool(void* backupHandle, int readThreadNum, int writeThreadNum)
{
    Backup* handle = static_cast<Backup*>(backupHandle);
    dynamic_pointer_cast<HostBackupAdvanceParams>(handle->m_backupParams.srcAdvParams)->threadNum =
        readThreadNum;
    dynamic_pointer_cast<HostBackupAdvanceParams>(handle->m_backupParams.dstAdvParams)->threadNum =
        writeThreadNum;
    return 0;
}

int ConfigureMemory(void* backupHandle, int maxMemory)
{
    Backup* handle = static_cast<Backup*>(backupHandle);
    dynamic_pointer_cast<HostBackupAdvanceParams>(handle->m_backupParams.srcAdvParams)->maxMemory = maxMemory;
    return 0;
}

int Enqueue(void* backupHandle, const char* backupControlFile)
{
    if (backupHandle == nullptr) {
        return -1;
    }
    string controlFile = backupControlFile;
    Backup* handle = static_cast<Backup*>(backupHandle);
    BackupRetCode ret = handle->Enqueue(controlFile);
    if (ret == BackupRetCode::SUCCESS) {
        return 0;
    } else {
        return -1;
    }
}

int Start(void* backupHandle)
{
    if (backupHandle == nullptr) {
        return -1;
    }
    Backup* handle = static_cast<Backup*>(backupHandle);
    BackupRetCode ret = handle->Start();
    if (ret == BackupRetCode::SUCCESS) {
        return 0;
    } else {
        return -1;
    }
}

int GetStatus(void* backupHandle)
{
    if (backupHandle == nullptr) {
        return BACKUP_FAILED;
    }
    Backup* handle = static_cast<Backup*>(backupHandle);
    BackupPhaseStatus status = handle->GetStatus();
    if (status == BackupPhaseStatus::COMPLETED) {
        return BACKUP_COMPLETED;
    } else if (status == BackupPhaseStatus::INPROGRESS) {
        return BACKUP_INPROGRESS;
    } else {
        return BACKUP_FAILED;
    }
}

void GetStats(void* backupHandle, BackupStatistics* backupStats)
{
    if (backupHandle == nullptr) {
        return;
    }
    Backup* handle = static_cast<Backup*>(backupHandle);
    BackupStats stats = handle->GetStats();
    backupStats->noOfDirToBackup   = stats.noOfDirToBackup;
    backupStats->noOfFilesToBackup = stats.noOfFilesToBackup;
    backupStats->noOfBytesToBackup = stats.noOfBytesToBackup;
    backupStats->noOfDirCopied     = stats.noOfDirCopied;
    backupStats->noOfFilesCopied   = stats.noOfFilesCopied;
    backupStats->noOfBytesCopied   = stats.noOfBytesCopied;
    backupStats->noOfDirFailed     = stats.noOfDirFailed;
    backupStats->noOfFilesFailed   = stats.noOfFilesFailed;
    backupStats->backupspeed       = stats.backupspeed;
    return;
}

void DestroyBackupInst(void* backupHandle)
{
    if (backupHandle == nullptr) {
        return;
    }
    Backup* handle = static_cast<Backup*>(backupHandle);
    delete(handle);
    return;
}