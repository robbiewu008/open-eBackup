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
#ifndef COMMON_BACKUP_JOB_H
#define COMMON_BACKUP_JOB_H

#ifndef WIN32
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
#endif
#include "CommonDBJob.h"
#include "trjsontostruct.h"
#include "trstructtojson.h"

namespace GeneralDB {
class CommonBackupJob : public GeneralDB::CommonDBJob, public std::enable_shared_from_this<CommonBackupJob> {
public:
    CommonBackupJob() {}
    ~CommonBackupJob() {}
    int PrerequisiteJob() override;
    int GenerateSubJob() override;
    int ExecuteSubJob() override;
    int PostJob() override;
    int GenerateSubJobManually() override;
private:
    int QueryBackupCopy();
    void QueryScanRepositories(std::shared_ptr<AppProtect::BackupJob> &job,
                               AppProtect::ScanRepositories &scanRepositories);
    mp_int32 ScanAndRecordFile(std::shared_ptr<AppProtect::BackupJob> &job);
    mp_int32 NormalGetScanRepositories(std::shared_ptr<AppProtect::BackupJob> &job,
                                       AppProtect::ScanRepositories &scanRepositories);
    void GetScanSavePath(const mp_string &path, AppProtect::ScanRepositories &scanRepositories);
    mp_int32 GetScanResult(std::shared_ptr<AppProtect::BackupJob> &job,
                           const AppProtect::ScanRepositories &scanRepositories);
    mp_int32 DoScan(std::shared_ptr<AppProtect::BackupJob> &job, const mp_string& savePath, AppProtect::RepositoryPath &repoPath);
    mp_int32 DoGenerateRecord(std::shared_ptr<AppProtect::BackupJob> &job,
        const mp_string &savePath, const mp_string& scanPath, int64_t repositoryType);
    mp_int32 ScanDirAndFile(mp_string &rootPath, std::vector<mp_string> &rootfolderpath,
                            std::vector<mp_string> &rootfilepath);
    mp_int32 GetFolderPath(mp_string &strFolder, std::vector<mp_string> &vecFolderPath);
    mp_int32 GetFilePath(mp_string &strFolder, std::vector<mp_string> &vecFolderPath);
#ifdef WIN32
    mp_string GetMountPublicPath();
    void PreHandleMountPublicPath(std::string &mount_public_path);
    mp_string GetSystemDiskChangedPathInWin(const mp_string& oriPath);
#endif
    mp_int32 CheckPathIsValid(const mp_string &filePath);
    size_t FindStrPos(const mp_string str, char c, int n);
    mp_int32 TruncateScanPath(std::shared_ptr<AppProtect::BackupJob> &job, mp_string &scanPath, int64_t repositoryType, bool isLogNotMeta = false);
    void TruncateStorageName(std::shared_ptr<AppProtect::BackupJob> &job, mp_string &StorageName, int &copy_format);

    bool IsScriptExist(const mp_string &appType, const mp_string &cmdType);
    void GetScanDirAndFileJobSwitch(Json::Value backupJobStr, bool &isNeedScan);
    int ExeQueryBackupCopySubJob(std::shared_ptr<AppProtect::BackupJob> &backupJobPtr);
    int ExecuteScanAndRecordFileSubJob(std::shared_ptr<AppProtect::BackupJob> &backupJobPtr);
    void AnalazeScanParam(const StorageRepository &repo, const Json::Value &repoJson, bool &isNeedScan);
    mp_void StartKeepAliveThread();
    mp_void StopKeepAliveThread();
    mp_void ReportSubJobRunning();
    std::atomic<bool> m_stopScanKeepAliveTheadFlag {false};
    std::shared_ptr<std::thread> m_scanKeepAliveTh;
};
}
#endif // COMMON_BACKUP_JOB_H