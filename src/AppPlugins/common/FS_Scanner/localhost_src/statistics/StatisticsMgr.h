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
#ifndef FS_SCANNER_STATISTICS_MGR_H
#define FS_SCANNER_STATISTICS_MGR_H

#include <mutex>
#include <atomic>
#include <memory>
#include <vector>
#include <type_traits>
#include "ScanConsts.h"
#include "log/Log.h"
#include "log/BackupFailureRecorder.h"
#include "ScanConfig.h"

enum class ScanClientType {
    SCAN_NFS = 1,
    SCAN_SMB = 2,
    SCAN_POSIX = 3,
    SCAN_WINDOWS = 4
};

constexpr auto STATSMODULE = "STATISTICS";
const int SCAN_NFS = 1;
const int SCAN_SMB = 2;
const int SCAN_POSIX = 3;
const int SCAN_WINDOWS = 4;
const int SCAN_NAS_SNAPDIFF = 5;
const int SCAN_OBJECT = 6;

const int MAX_COMMON_STATS_TYPE = 100;
const int MAX_SMB_STATS_TYPE = 10;
const int MAX_NFS_STATS_TYPE = 10;
const int MAX_POSIX_STATS_TYPE = 10;
const int MAX_OBJECT_STATS_TYPE = 10;
const int MAX_WINDOWS_STATS_TYPE = 10;

enum class CommStatsType : int {
    TOTAL_DIRS_TO_BACKUP = 0,
    TOTAL_FILES_TO_BACKUP = 1,
    TOTAL_DIRS_DELETED = 2,
    TOTAL_FILES_DELETED = 3,
    TOTAL_SIZE_TO_BACKUP = 4,
    TOTAL_CONTROL_FILES = 5,
    TOTAL_FAILED_DIRS = 6,
    TOTAL_FAILED_FILES = 7,
    FAIL_DIR_COUNT_MAX_PATH_LEN = 8,
    FAIL_FILE_COUNT_MAX_PATH_LEN = 9,
    TOTAL_DIR_DIFF_STARTED = 10,
    TOTAL_FILE_DIFF_STARTED = 11,
    TOTAL_DIRS_MODIFIED_COUNT = 12,
    TOTAL_DIRS_NEWLY_ADDED_COUNT = 13,
    TOTAL_DIR_DELETE_COUNT = 14,
    FILE_META_READ_TIME = 15,
    CTRL_META_READ_TIME = 16,
    DIR_META_READ_TIME = 17,
    FCACHE_READ_TIME = 18,
    DCACHE_READ_TIME = 19,

    READ_JOB_EXEC_COUNT = 20,
    WRITE_JOB_EXEC_COUNT = 21,
    READ_DIR_OBJECTS = 22,
    OPEN_DIR_REQUEST_COUNT = 23,
    OPEN_DIR_RESPONSE_COUNT = 24,
    OPEN_DIR_RETRY_COUNT = 25,
    OPEN_DIR_PENDING_REQ_COUNT = 26,
    OPEN_DIR_TOTAL_COUNT = 27,
    OPEN_DIR_RESUME_COUNT = 28,

    SCAN_JOB_COUNT = 29,
    SCAN_JOB_EXEC_COUNT = 30,
    READ_JOB_COUNT = 31,
    CHECK_TASK_RESULT_TIME = 32,
    CONTEXT_POLL_TIME = 33,

    TOTAL_DIRS = 34,
    TOTAL_FILES = 35,
    TOTAL_SIZE = 36,

    OPEN_DIR_ASYNC_TIME = 37,
    OPEN_DIR_TIME = 38,
    PROT_SERVER_FAIL_COUNT = 39,

    WRITE_QUEUE_DIRECTLY_PUSH_COUNT = 40,
    FILTER_DISCARD_DIR_COUNT = 41,
    OPEN_DIR_RESUME_RETRY_COUNT = 42,

    TOTAL_DIR_SKIP_COUNT  = 43,

    SMB_STAT_FILE_REQ_COUNT = 44,
    SMB_STAT_FILE_RESP_COUNT = 45,
    SMB_STAT_DIR_REQ_COUNT = 46,
    SMB_STAT_DIR_RESP_COUNT = 47,

    SMB_ACCESS_DIR_REQ_COUNT = 48,
    SMB_ACCESS_DIR_RESP_COUNT = 49,
    SMB_QUERY_DIR_REQ_COUNT = 50,
    SMB_QUERY_DIR_RESP_COUNT = 51,
    SMB_CLOSE_DIR_REQ_COUNT = 52,
    SMB_CLOSE_DIR_RESP_COUNT = 53,

    SMB_INCOMPLETE_STAT_FILES = 54,

    SCAN_TIME = 55,
    META_WRITE_TIME = 56,
    MERGE_DIRCACHE_TIME = 57,
    DIFF_TIME = 58,
    SCAN_CLEANUP_TIME = 59,
    SCAN_AND_WRITE_TIME = 60,
    SCAN_START_TIME = 61,
    OPENDIR_LAST_FAILED_TIME = 62,
    RATELIMIT_TIMER = 63,
    ENTRY_MAY_FAIL_TO_ARCHIVE = 64, // file/dir may failed to archive

    DCACHE_READ_COUNT = 65,  //  obs logbackup dcache read count

    MAX_COMMON_STATS_TYPE
};
using uCommStatsType = std::underlying_type<CommStatsType>::type;

/* NFSStatisticsType, SMBStatisticsType parameters are just for demo,
   exact parmeter can be placed based on specific Statistics information
   Index will start from 0, as all parameters will be set in Array */
enum class NFSStatisticsType {
    NFS_STATS_TYPE_1 = 0,
    NFS_STATS_TYPE_2 = 1,

    MAX_NFS_STATS_TYPE
};

enum class SMBStatisticsType {
    SMB_STATS_TYPE_1 = 0,
    SMB_STATS_TYPE_2 = 1
};

class ProtocolStatistics {
public:
    ProtocolStatistics() {}
    virtual ~ProtocolStatistics() {}
    virtual void IncrProtoStatsByType(const int& idx, const uint64_t& incVal = 1) = 0;
    virtual void SetProtoStatsByType(const int& idx, const uint64_t& newVal) = 0;
    virtual uint64_t GetProtoStatsByType(const int& protoStatsType) = 0;
};

class StatisticsMgr {
    std::atomic_uint64_t m_commonStatistics[MAX_COMMON_STATS_TYPE];
    std::unique_ptr<ProtocolStatistics> m_ProtocolStatsObj = NULL;

public:
    StatisticsMgr();
    StatisticsMgr(const std::string& failureRecordRootPath, const std::string& jobId, const std::string& subJobId);
    ~StatisticsMgr();

    // Common Statistics for all Protocol
    void IncrCommStatsByType(CommStatsType statsType, const uint64_t& incrValue = 1);
    void DecrCommStatsByType(CommStatsType statsType, const uint64_t& incrValue = 1);
    void SetCommStatsByType(CommStatsType statsType, const uint64_t newValue);
    uint64_t GetCommStatsByType(CommStatsType statsType);

    void IncrCommonStatsByRange(const uint64_t* commonStats, CommStatsType startType, CommStatsType endType);
    void IncrAllCommStats(const uint64_t* commonStats);
    void IncrCommStatsByIdxList(const uint64_t* commonStats, std::vector<CommStatsType>& vecIdxList);
    std::atomic_uint64_t* GetAllCommonStats();

    // API to Set & Get Protocol Specific Stats
    void SetProtoStatsFactoryObject(const int idx);
    void IncrementProtoStatsByType(const int& protoStatsType, const uint64_t& incVal = 1);
    void SetProtocolStatsByType(const int& protoStatsType, const uint64_t& newVal);
    uint64_t GetProtoStatsByType(const int& protoStatsType);
    void RecordErrMessage(const int& linuxErrCode, const std::string& errCode, const std::string& errMessage);

    // use for index, count complete threads. if all complete, that is true complete.
    std::atomic<uint8_t> m_completeThreads {0};
    // used record file/directory failed to scan
    std::shared_ptr<Module::BackupFailureRecorder> m_failureRecorder = nullptr;
    ErrRecorder m_errRecorder;
 
private:
    std::mutex m_mutex;
};

#endif
