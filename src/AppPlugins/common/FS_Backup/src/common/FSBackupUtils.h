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
#ifndef UTILITIES_H
#define UTILITIES_H
#ifndef WIN32
#include <sys/time.h>
#endif
#include <securec.h>
#include <ctime>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "BackupConstants.h"
#include "BackupStructs.h"
#include "Backup.h"
#ifdef WIN32
#include <windows.h>
#endif

#ifndef UNREFERENCE_PARAM
#define UNREFERENCE_PARAM(x) ((void)(x))
#endif

#ifdef _MSVC_LANG
    namespace mem = std; // msvc pipeline use C++17
#else
#if __cplusplus > 201103L
    namespace mem = std; // C++14 defined make_unique
#else
    namespace mem = Module;
#endif
#endif

const uint16_t MAX_JOB_SCHDULER = 24;

namespace FSBackupUtils {
    std::string GetFileName(std::string filePath);
    std::string GetParentDir(const std::string& filePath); // split dir name using ctrl file path separator
    std::string GetParentDirV2(const std::string& filePath); // split dir name using platform-related path separator
    std::string JoinPath(const std::string& prev, const std::string& trail);
    bool Exists(const std::string& path);
    void RecurseCreateDirectory(const std::string& path);
    bool IsWindowDriverRoot(std::string pathNormalized);
    bool RemoveDir(const std::string& path);
    bool RemoveFile(const std::string& path);
    bool CreateFile(const std::string& path);
    bool CreateLink(const std::string& linkName, const std::string& targetFile);
    bool CreateSymLink(const std::string& symLinkName, const std::string& targetFileOrFolder);
    void RemoveLeadingSlashes(std::string &str);
    void RemoveTrailingSlashes(std::string &str);
    std::string GenerateRandomStr();
    time_t GetCurrentTime();
    int64_t GetMilliSecond();
    std::string FormatSpeed(uint64_t speed);
    std::string FloatToString(const float &val, const uint8_t &precisson = 1);
    std::string GetDateTimeString(uint64_t time);
#ifndef WIN32
    mode_t GetUmask();
#endif
    int GetFileWithSubDirListInDir(const std::string& dir,
        std::vector<std::string>& fileList, std::vector<std::string>& subDirList);
    std::string CheckMetaFileVersion(const std::string& dir);
    bool IsSpecialFile(uint32_t mode);
#ifdef _NAS
    int SetSrcFileHandleForNfs(FileHandle &fileHandle, int length, const char value[]);
#endif
    uint16_t GetHashIndexForSqliteTask(const std::string &word);
    void SetServerNotReachableErrorCode(const BackupType &backupType,
        BackupPhaseStatus &failReason, bool isReader = true);

    BackupPhaseStatus GetReaderStatus(
        std::shared_ptr<BackupControlInfo> controlInfo,
        bool abort,
        BackupPhaseStatus failReason = BackupPhaseStatus::FAILED);

    BackupPhaseStatus GetWriterStatus(
        std::shared_ptr<BackupControlInfo> controlInfo,
        bool abort,
        BackupPhaseStatus failedReason = BackupPhaseStatus::FAILED);

    BackupPhaseStatus GetControlFileReaderStatus(std::shared_ptr<BackupControlInfo> controlInfo, bool abort);
    BackupPhaseStatus GetAggregateStatus(std::shared_ptr<BackupControlInfo> ctrlInfo, bool abort);
    BackupPhaseStatus GetFailureStatus(const BackupPhaseStatus &readerStatus,
        const BackupPhaseStatus &writerStatus);
    bool IsCiticalError(const BackupPhaseStatus &status);

    bool IsSymLinkFile(const FileHandle& fileHandle);

    std::string ParseCustomizedErrorCode(uint32_t errorCode);
    std::string GetErrMsgByErrCode(uint32_t errorCode);

    void RecordFailureDetail(
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder,
        const std::pair<std::string, uint64_t>& errDetails);

    void RecordFailureDetail(
        std::shared_ptr<Module::BackupFailureRecorder> failureRecorder, std::string path, uint32_t errCode);

    std::string PathConcat(const std::string& forwardPath, const std::string& trailPath);
    bool Rename(const std::string &oldName, const std::string &newName);

    bool IsHandleMetaModified(const std::string &scannermode, const BackupDataFormat &format);
    bool OnlyGenerateSqlite(bool genSqlite);
    void MemoryTrim();
};

#endif // UTILITIES_H