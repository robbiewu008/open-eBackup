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
#ifndef BACKUP_H
#define BACKUP_H

#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include "EnumDefines.h"
#include "common/CleanMemPwd.h"
#include "BackupConstants.h"
#include "ArchiveClientBase.h"
#include "StreamHostFilePendingMap.h"
#include "log/BackupFailureRecorder.h"
#ifdef _OBS
#include "manager/CloudServiceManager.h"
#endif

#ifdef __cplusplus
extern "C" {
typedef struct BackupStats_S {
    uint64_t noOfDirToBackup    = 0;        /* Number of directories to be backed up */
    uint64_t noOfFilesToBackup  = 0;        /* Number of files to be backed up */
    uint64_t noOfBytesToBackup  = 0;        /* Number of bytes (in KB) to be backed up */
    uint64_t noOfDirCopied      = 0;        /* Number of directories copied */
    uint64_t noOfFilesCopied    = 0;        /* Number of files copied */
    uint64_t noOfBytesCopied    = 0;        /* Number of bytes (in KB) copied */
    uint64_t noOfDirFailed      = 0;        /* Number of directories copy failed */
    uint64_t noOfFilesFailed    = 0;        /* Number of files copy failed */
    uint64_t backupspeed        = 0;        /* Backup speed (in KBps) */
} BackupStatistics;
}
#endif

/* use this object to share statistics between different submodule, and task finish condition */
struct BackupControlInfo {
    std::atomic<uint64_t> m_noOfDirToBackup         {0};    /* No of directories to be backed up */
    std::atomic<uint64_t> m_noOfFilesToBackup       {0};    /* No of files to be backed up */
    std::atomic<uint64_t> m_noOfBytesToBackup       {0};    /* No of bytes (in KB) to be backed up */
    std::atomic<uint64_t> m_noOfDirToDelete         {0};    /* No of directories to be deleted */
    std::atomic<uint64_t> m_noOfFilesToDelete       {0};    /* No of files to be deleted */
    std::atomic<uint64_t> m_noOfDirCopied           {0};    /* No of directories copied */
    std::atomic<uint64_t> m_noOfFilesCopied         {0};    /* No of files copied */
    std::atomic<uint64_t> m_noOfBytesCopied         {0};    /* No of bytes (in KB) copied */
    std::atomic<uint64_t> m_noOfDirDeleted          {0};    /* No of directories deleted */
    std::atomic<uint64_t> m_noOfFilesDeleted        {0};    /* No of files deleted */
    std::atomic<uint64_t> m_noOfDirFailed           {0};    /* No of directories failed to be copied/deleted */
    std::atomic<uint64_t> m_noOfFilesFailed         {0};    /* No of files failed to be copied/deleted */
    std::atomic<uint64_t> m_noOfFilesRead           {0};    /* No of files read */
    std::atomic<uint64_t> m_noOfSubStreamFound      {0};    /* No of ADS file (NTFS substream) detected */
    std::atomic<uint64_t> m_noOfSubStreamRead       {0};    /* No of ADS file (NTFS substream) read */
    std::atomic<uint64_t> m_noOfSubStreamCopied     {0};    /* No of ADS file (NTFS substream) copied */
    std::atomic<uint64_t> m_noOfSubStreamBytesCopied{0};    /* No of ADS file (NTFS substream) bytes copied */
    std::atomic<uint64_t> m_noOfDirRead             {0};    /* No of directories read/pushed to aggregator */
    std::atomic<uint64_t> m_noOfFilesReadFailed     {0};    /* No of files failed to be read */
    std::atomic<uint64_t> m_noOfFilesWriteSkip      {0};    /* No of files skipped to write (Ignore replace policy) */
    time_t                m_startTime               {0};    /* Start time of backup phase */
    std::atomic<uint64_t> m_noOfSrcRetryCount       {0};    /* No of src side retry count */
    std::atomic<uint64_t> m_noOfDstRetryCount       {0};    /* No of dst side retry count */

    std::atomic<uint64_t> m_controlFileReaderProduce    {0};        /* total file and dir */
    std::atomic<uint64_t> m_readConsume                 {0};        /* total file and dir */
    std::atomic<uint64_t> m_readProduce                 {0};        /* total read block */
    std::atomic<uint64_t> m_aggregateConsume            {0};        /* total read block */
    std::atomic<uint64_t> m_aggregateProduce            {0};        /* total read block */
    std::atomic<uint64_t> m_aggregatedFiles             {0};        /* total agggregated files */
    std::atomic<uint64_t> m_archiveFiles                {0};        /* archive files produced by aggregator */
    /* total archive files push to read queue by aggregator for restore */
    std::atomic<uint64_t> m_unarchiveFiles              {0};
    /* total unaggregated files pushed to writer queue by aggregator for restore */
    std::atomic<uint64_t> m_unaggregatedFiles           {0};
    /* during restore if a zip/aggregated file read fails so related to that  normal files failed need to consider */
    std::atomic<uint64_t> m_unaggregatedFaildFiles      {0};
    std::atomic<uint64_t> m_emptyFiles                  {0};  /* empty files processed by aggregator for restore */
    std::atomic<uint64_t> m_writerConsume               {0};        /* total read block */
    std::atomic<bool> m_controlReaderPhaseComplete      {false};
    std::atomic<bool> m_readPhaseComplete               {false};
    std::atomic<bool> m_aggregatePhaseComplete          {false};
    std::atomic<bool> m_writePhaseComplete              {false};
    std::atomic<bool> m_controlReaderFailed             {false};
    std::atomic<bool> m_failed                          {false};
    std::atomic<BackupPhaseStatus> m_backupFailReason   {BackupPhaseStatus::FAILED};
    std::atomic<uint64_t> m_skipFileCnt                 {0};        /* mode is dd */
    std::atomic<uint64_t> m_skipDirCnt                  {0};        /* mode is dd or root directory */
    std::atomic<uint64_t> m_aggrRestoreInMemoryFhCnt    {0};        /* Total filehandle in each DirMap enties */

    std::shared_ptr<StreamHostFilePendingMap>  m_streamHostFilePendingMap { nullptr };

    std::atomic<uint64_t> m_writeTaskProduce { 0 };
    std::atomic<uint64_t> m_writeTaskConsume { 0 };
    std::atomic<uint64_t> m_readTaskProduce { 0 };
    std::atomic<uint64_t> m_readTaskConsume { 0 };
    std::chrono::steady_clock::time_point m_lastRecordTime { std::chrono::steady_clock::now() };
    std::atomic<uint64_t> m_lastFailedFileNum { 0 };
    std::atomic<uint64_t> m_lastFailedDirNum { 0 };
    std::atomic<uint64_t> m_lastCopyFileNum { 0 };
};

struct BackupStats {
    uint64_t noOfDirToBackup    = 0;        /* No of directories to be backed up */
    uint64_t noOfFilesToBackup  = 0;        /* No of files to be backed up */
    uint64_t noOfBytesToBackup  = 0;        /* No of bytes (in KB) to be backed up */
    uint64_t noOfDirToDelete    = 0;        /* No of directories to be deleted */
    uint64_t noOfFilesToDelete  = 0;        /* No of files to be deleted */
    uint64_t noOfDirCopied      = 0;        /* No of directories copied */
    uint64_t noOfFilesCopied    = 0;        /* No of files copied */
    uint64_t noOfBytesCopied    = 0;        /* No of bytes (in KB) copied */
    uint64_t skipFileCnt        = 0;        /* No of files skipped */
    uint64_t skipDirCnt         = 0;        /* No of dir skipped */
    uint64_t noOfDirDeleted     = 0;        /* No of directories deleted */
    uint64_t noOfFilesDeleted   = 0;        /* No of files deleted */
    uint64_t noOfDirFailed      = 0;        /* No of directories failed to be copied/deleted */
    uint64_t noOfFilesFailed    = 0;        /* No of files failed to be copied/deleted */
    uint64_t readConsume        = 0;        /* No of bloack reader consumed */
    uint64_t aggregateConsume   = 0;        /* No of bloack aggregater consumed */
    uint64_t writerConsume      = 0;        /* No of bloack writer consumed */
    uint64_t backupspeed        = 0;        /* Backup speed (in KBps) */
    time_t   startTime          = 0;        /* Start time of backup phase */
    uint64_t noOfSrcRetryCount  = 0;        /* No of src side retry count */
    uint64_t noOfDstRetryCount  = 0;        /* No of dst side retry count */
    uint64_t noOfFailureRecordsWritten = 0; /* No of backup failure records that have been written to file */
    uint64_t noOfFilesWriteSkip = 0;    /* No of files skipped to write (Ignore replace policy) */
    uint64_t noOfReadTaskProduce = 0;   /* No of read task produced */
    uint64_t noOfReadTaskConsume = 0;   /* No of read task consumed */
    uint64_t noOfWriteTaskProduce = 0;  /* No of write task produced */
    uint64_t noOfWriteTaskConsume = 0;  /* No of write task consumed */
    bool operator !=(const BackupStats& other)
    {
        return noOfDirCopied != other.noOfDirCopied ||
            noOfFilesCopied != other.noOfFilesCopied ||
            noOfBytesCopied != other.noOfBytesCopied ||
            skipFileCnt != other.skipFileCnt ||
            skipDirCnt != other.skipDirCnt ||
            noOfDirDeleted != other.noOfDirDeleted ||
            noOfFilesDeleted != other.noOfFilesDeleted ||
            noOfDirFailed != other.noOfDirFailed ||
            noOfFilesFailed != other.noOfFilesFailed ||
            readConsume != other.readConsume ||
            aggregateConsume != other.aggregateConsume ||
            writerConsume != other.writerConsume ||
            startTime != other.startTime ||
            noOfSrcRetryCount != other.noOfSrcRetryCount ||
            noOfDstRetryCount != other.noOfDstRetryCount ||
            noOfFailureRecordsWritten != other.noOfFailureRecordsWritten ||
            noOfDirToBackup != other.noOfDirToBackup ||
            noOfFilesToBackup != other.noOfFilesToBackup ||
            noOfBytesToBackup != other.noOfBytesToBackup ||
            noOfDirToDelete != other.noOfDirToDelete ||
            noOfFilesToDelete != other.noOfFilesToDelete ||
            noOfFilesWriteSkip != other.noOfFilesWriteSkip ||
            noOfReadTaskProduce != other.noOfReadTaskProduce ||
            noOfReadTaskConsume != other.noOfReadTaskConsume ||
            noOfWriteTaskProduce != other.noOfWriteTaskProduce ||
            noOfWriteTaskConsume != other.noOfWriteTaskConsume;
    }
};

struct BackupAdvanceParams {
    BackupAdvanceParams() {}
    virtual ~BackupAdvanceParams() {}
};

struct AsyncAdvanceParams : public BackupAdvanceParams {
    uint32_t maxPendingAsyncReqCnt = DEFAULT_MAX_REQ_COUNT;      /* Max Async requests pending */
    uint32_t minPendingAsyncReqCnt = DEFAULT_MIN_REQ_COUNT;      /* Min Async requests pending */
    uint32_t maxPendingWriteReqCnt = DEFAULT_MAX_WRITE_COUNT;    /* Max Async write requests pending */
    uint32_t minPendingWriteReqCnt = DEFAULT_MIN_WRITE_COUNT;    /* Min Async write requests pending */
    uint32_t maxPendingReadReqCnt = DEFAULT_MAX_READ_COUNT;      /* Max Async read requests pending */
    uint32_t minPendingReadReqCnt = DEFAULT_MIN_READ_COUNT;      /* Min Async read requests pending */
    uint32_t serverCheckMaxCount = DEFAULT_SERVER_CHECK_COUNT;   /* Max Retriable count to initiate Server check */
    uint32_t serverCheckSleepTime = DEFAULT_SERVER_CHECK_SLEEP;  /* Max sleep time inbetween server checks */
    uint32_t serverCheckRetry = DEFAULT_SERVER_CHECK_RETRY;      /* Max retries for server check */
};

struct CommonParams {
    std::string jobId;
    std::string copyId;
    std::string subJobId;
    std::string failureRecordRootPath;
    std::string metaPath;
    std::string sqliteLocalPath {};
    std::size_t reqID  {};              // Request ID corresponding to a subjob
    std::string trimReaderPrefix;
    std::string trimWriterPrefix;       // trim target path prefix for fine grained restore (only used for writer)
    std::string controlFile;

    bool        skipFailure             { true };
    bool        writeMeta               { true };
    bool        writeAcl                { false };
    bool        writeExtendAttribute    { false };
    bool        writeSparseFile         { false };
    bool        discardReadError        { false };
    bool        writeDisable            { false };
    bool        isReExecutedTask        { false };
    bool        genSqlite               { false }; // For not AGGREGATE backup, genrate sqlite (only for object storage)
    bool        useSubJobSqlite         { false };
    uint32_t    maxErrorFiles           { DEFAULT_ERROR_FILE_CNT };
    uint32_t    maxBufferCnt            { DEFAULT_QUEUE_LENGTH };
    uint32_t    maxBufferSize           { ::DEFAULT_QUEUE_SIZE };
    uint32_t    blockSize               { ::DEFAULT_BLOCK_SIZE };
    uint32_t    maxBlockNum             { 0 };
    uint32_t    aggregateThreadNum      { DEFAULT_SYNC_IO_THREAD_NUM };
    uint32_t    maxAggregateFileSize    { DEFAULT_MAX_AGGREGATE_FILE_SIZE };
    uint32_t    maxFileSizeToAggregate  { DEFAULT_MAX_FILE_SIZE_TO_AGGREGATE };
    uint64_t    maxFailureRecordsNum    { DEFAULT_MAX_FAILURE_RECORDS_NUM };

    std::vector<std::string> aggregateFileSet;
    BackupDataFormat        backupDataFormat        { BackupDataFormat::UNKNOWN_FORMAT };
    RestoreReplacePolicy    restoreReplacePolicy    { RestoreReplacePolicy::NONE };
    OrderOfRestore          orderOfFilenames        { OrderOfRestore::OFF };
    AdsProcessType          adsProcessType          { AdsProcessType::ADS_PROCESS_TYPE_FROM_SCANNER };
};

struct ScanAdvanceParams {
    std::string metaFilePath;
    bool useXmetaFileHandle = false;
};

struct HostBackupAdvanceParams : public BackupAdvanceParams {
    std::string         dataPath;
    uint32_t            threadNum { DEFAULT_SYNC_IO_THREAD_NUM };
    uint32_t            maxMemory { MEMORY_THRESHOLD_HIGH };
};

/* add these two struct name alias for Plugin compatible purpose, will remove later */
using PosixBackupAdvanceParams = HostBackupAdvanceParams;
using Win32BackupAdvanceParams = HostBackupAdvanceParams;

struct LibnfsBackupAdvanceParams : public AsyncAdvanceParams {
    time_t              jobStartTime = 0;                       /* Job Start Time */
    time_t              deleteJobStartTime = 0;                 /* Delete Job Start Time */
    std::string         ip {};                                  /* IP x.x.x.x */
    std::string         sharePath {};                           /* NAS share path */
    std::string         protocolVersion {};                     /* NFS protocol Version */
    std::string         dataPath {};                            /* The absolute mount path */
};

struct NfsAntiRansomwareAdvanceParams : public AsyncAdvanceParams {
    std::size_t                 reqID {};                       /* Request ID corresponding to a subjob */
    BackupAntiType              backupAntiType;                 /* WORM =0 , ENTROPY =1 */
    uint64_t                    atime;
    std::string                 mode;
    std::vector<std::string>    filterDirList;
    std::string                 entropyOutPath;
    std::string                 ip;
    std::string                 sharePath;
    std::string                 protocolVersion;
    bool                        isOnlyModifyAtime { false };
};

struct LibsmbBackupAdvanceParams : public BackupAdvanceParams {
    /* domain: 域 */
    std::string domain;
    /* server: 服务端IP，支持IPv4/IPv6 */
    std::string server;
    /* share: 共享名称，用来访问CIFS共享 */
    std::string share;
    /* user: 访问共享的用户，CIFS共享可以针对不同的用户提供不同的权限 */
    std::string user;
    /* password: 用户密码 */
    std::string password;
    /* krb5CcacheFile: kerberose票据文件 */
    std::string krb5CcacheFile;
    /* krb5ConfigFile: kerberose配置文件 */
    std::string krb5ConfigFile;

    /* encryption: 是否加密连接，仅SMB3.0以上版本支持加密 */
    bool encryption = false;
    /* sign: 是否需要签名 */
    bool sign = false;
    /* timeout: 超时单位：秒，超时未响应的报文被丢弃并返回IO_TIMEOUT异常 */
    int timeout = 0;
    /* authType: 认证类型，支持NTLMSSP和KRB5两种 */
    std::string authType;
    /* version: 协议版本，包括3.1.1、3.02、3.0、2.1、2.02 */
    std::string version;

    uint32_t maxPendingAsyncReqCnt = 32;        /* Max Async requests pending */

    uint32_t minPendingAsyncReqCnt = DEFAULT_MIN_REQ_COUNT;      /* Min Async requests pending */
    uint32_t maxPendingWriteReqCnt = DEFAULT_MAX_WRITE_COUNT;    /* Max Async write requests pending */
    uint32_t minPendingWriteReqCnt = DEFAULT_MIN_WRITE_COUNT;    /* Min Async write requests pending */
    uint32_t maxPendingReadReqCnt = DEFAULT_MAX_READ_COUNT;      /* Max Async read requests pending */
    uint32_t minPendingReadReqCnt = DEFAULT_MIN_READ_COUNT;      /* Min Async  requests pending */

    uint32_t serverCheckMaxCount = DEFAULT_SERVER_CHECK_COUNT;       /* Max Retriable count to initiate Server check */
    
    uint32_t serverCheckSleepTime = DEFAULT_SERVER_CHECK_SLEEP;  /* Max sleep time inbetween server checks */
    uint32_t serverCheckRetry = DEFAULT_SERVER_CHECK_RETRY;      /* Max retries for server check */
    uint32_t maxOpenedFilehandleCount = 100;
    uint32_t pollExpiredTime = 100;

    std::string rootPath;

    ~LibsmbBackupAdvanceParams()
    {
        Module::CleanMemoryPwd(password);
    }
};

struct LibS3IOBackupAdvanceParams : public BackupAdvanceParams {
    ~LibS3IOBackupAdvanceParams()
    {
        Module::CleanMemoryPwd(ak);
        Module::CleanMemoryPwd(sk);
        Module::CleanMemoryPwd(proxyUser);
        Module::CleanMemoryPwd(proxyPassword);
    }

    /* s3 info */
    std::string         ak;
    std::string         sk;
    std::string         s3Path;
    std::string         cert;
    bool                useHttps { false };
    std::string         proxyHost;
    std::string         proxyPort;
    std::string         proxyUser;
    std::string         proxyPassword;
    bool                useProxy { false };
    std::string         speedUpMethod;
    bool                useSpeedUp { false };
    /* qos */
    uint64_t            uploadQos { 0 };
    uint64_t            downloadQos { 0 };
    /* file system info */
    std::string         localPath;
};

struct FailedRecordItem {
    uint16_t metaIndex;
    uint32_t errNum;
    uint64_t offset;
    std::string filePath;
    std::string errMsg;

    bool operator<(const FailedRecordItem& other) const
    {
        if (metaIndex != other.metaIndex) {
            return metaIndex < other.metaIndex;
        } else if (offset != other.offset) {
            return offset < other.offset;
        } else {
            return errNum < other.errNum;
        }
    }

    bool operator==(const FailedRecordItem& other) const
    {
        bool ret = metaIndex == other.metaIndex &&
            errNum == other.errNum &&
            offset == other.offset &&
            filePath == other.filePath;
        return ret;
    }
};

struct FailedRecordItemHash {
    std::size_t operator()(const FailedRecordItem& item) const
    {
        std::size_t h1 = std::hash<uint16_t>()(item.metaIndex);
        std::size_t h2 = std::hash<uint32_t>()(item.errNum);
        std::size_t h3 = std::hash<uint64_t>()(item.offset);
        std::size_t h4 = std::hash<std::string>()(item.filePath);
        return h1 ^ (h2 << 1) ^ h3 ^ (h4 << 1);
    }
};

#ifdef _OBS
struct ObsBucket {
    std::string bucketName;
    std::vector<std::string> prefix;
    std::string delimiter;
    bool encodeEnable { false };
};

struct ObjectBackupAdvanceParams : public BackupAdvanceParams {

    std::string dataPath;  // 大文件恢复时，直接上传data里面的原文件，不分片读
    std::string cachePath; // 大文件恢复时，存放checkpoint
    uint32_t threadNum { DEFAULT_SYNC_IO_THREAD_NUM };
    uint32_t maxMemory { MEMORY_THRESHOLD_HIGH };
    bool saveMeta { true };
    std::string excludeMeta; // 排除哪些元数据不备份

    Module::StorageConfig authArgs;
    std::vector<ObsBucket> buckets; // 备份的桶及前缀(恢复时也需要填充)
    ObsBucket dstBucket; // 恢复时目标桶，可能包含前缀
    bool isfineGrainedRestore {false};  // 是否为细粒度恢复

    ~ObjectBackupAdvanceParams()
    {
        Module::CleanMemoryPwd(authArgs.verifyInfo.accessKey);
        Module::CleanMemoryPwd(authArgs.verifyInfo.secretKey);
        Module::CleanMemoryPwd(authArgs.verifyInfo.proxyUserName);
        Module::CleanMemoryPwd(authArgs.verifyInfo.proxyUserPwd);
    }
};
#endif

struct ArchiveRestoreAdvanceParams : public BackupAdvanceParams {
public:
    explicit ArchiveRestoreAdvanceParams(std::shared_ptr<ArchiveClientBase> client) : archiveClient(client) {}

    std::string         dataPath;
    std::string         jobId;
    std::string         copyId;
    std::string         fsId;
    int                 readSize {0}; // SDK config
    int                 maxResponseSize {0}; // SDK config
    std::shared_ptr<ArchiveClientBase> archiveClient {nullptr};
};

struct BackupParams {
    BackupIOEngine                          srcEngine       { BackupIOEngine::UNKNOWN_ENGINE };
    BackupIOEngine                          dstEngine       { BackupIOEngine::UNKNOWN_ENGINE };
    BackupPhase                             phase           { BackupPhase::UNKNOWN_STAGE };
    std::shared_ptr<BackupAdvanceParams>    srcAdvParams    {nullptr};
    std::shared_ptr<BackupAdvanceParams>    dstAdvParams    {nullptr};
    CommonParams                            commonParams    {};
    ScanAdvanceParams                       scanAdvParams   {};
    BackupType                              backupType      { ::BackupType::UNKNOWN_TYPE };
};

namespace FS_Backup {
class Backup {
public:
    explicit Backup(const BackupParams& backupParams) : m_backupParams(backupParams) {}

    Backup(const std::string& source, const std::string& destination, const std::string& metaPath)
    {
        m_source = source;
        m_destination = destination;
        m_backupParams.scanAdvParams.metaFilePath = metaPath;
    }

    virtual ~Backup() {}

    virtual BackupRetCode Start() = 0;
    virtual BackupRetCode Abort() = 0;
    virtual BackupRetCode Destroy() = 0;
    virtual BackupRetCode Enqueue(std::string controlFile) = 0;
    virtual BackupPhaseStatus GetStatus() = 0;
    virtual BackupStats GetStats() = 0;
    virtual std::unordered_set<FailedRecordItem, FailedRecordItemHash> GetFailedDetails() = 0;

public:
    BackupParams m_backupParams;

private:
    BackupModuleStatus m_backupStatus = BackupModuleStatus::BACKUP_DEFAULT;
    std::string m_source;
    std::string m_destination;
};
}
#endif