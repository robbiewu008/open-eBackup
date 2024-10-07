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
#ifndef BACKUP_FAILURE_RECORDER
#define BACKUP_FAILURE_RECORDER

#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include "define/Defines.h"

namespace Module {
/*
 * Used by NasPlugin/FilePlugin to store file/directory failed to backup
 * Folder hierarchical Structure:
 * slog/Plugin/FilePlugin/failurerecoder                                      ---- outputDirPath
 *                              |--${jobID}
 *                                      |--${subTaskID}_failure.csv
 *
 *  Each backup/restore job subTask can only have one BackupFailureRecorder instance, and an instance will only 
 *  produce at most one *.csv file. Each FS_Backup task process a BackupFailureRecorder instance, if no
 *  file/directory failed during the task, corresonding *.csv file will not be generated.
 *
 *  These *.csv file will be merged at the post job stage, processed by AppPlugin
 */

struct BackupFailureRecord {
    std::string     subTaskID;
    uint64_t        timestamp; /* unix timestamp in sec */
    std::string     path;
    std::string     reason;
};

class AGENT_API BackupFailureRecorder {
public:
    BackupFailureRecorder(
        const std::string&          outputDirRootPath,
        const std::string&          jobID,
        const std::string&          subjobID,
        size_t                      bufferMax,
        uint64_t                    recordMax);

    ~BackupFailureRecorder();

    void RecordFailure(
        const std::string&          path,
        const std::string&          reason);

    uint64_t NumWrited() const;
    
    uint64_t NumWriteFailed() const;

    bool Flush();
    
    static bool Merge(
        const std::string&          outputDirRootPath,
        const std::string&          jobID);

private:
    bool FlushInner();

    std::string RecordToString(const BackupFailureRecord& record) const;

    std::string EscapePath(const std::string& path) const;

    std::string SecTimestampToDateStr(uint64_t timestamp) const;

    std::string GetJobRecordRootPath() const;

    static std::vector<std::string> GetFileListInDirectory(const std::string& path);

    static bool ClearSubTaskRecordFile(const std::string& jobRecordRootPath);

private:
    /* immutable member */
    std::string                         m_outputDirRootPath; /* can set the log path as the root */
    std::string                         m_jobID;
    std::string                         m_subTaskID;
    size_t                              m_bufferMax; /* max num to keep one batch record in memory*/
    uint64_t                            m_recordMax; /* max num of records this instance can produce */
    std::string                         m_filepath; /* path of the csv file produced by this instance */

    /* mutable member */
    std::mutex                          m_mutex; /* protect concurrent write for multiple stage threads */
    std::vector<BackupFailureRecord>    m_buffer;
    uint64_t                            m_numWrited = 0; /* num of records written to file by this instance */
    uint64_t                            m_numWriteFailed = 0; /* num of the records written failed */
};

}

#endif