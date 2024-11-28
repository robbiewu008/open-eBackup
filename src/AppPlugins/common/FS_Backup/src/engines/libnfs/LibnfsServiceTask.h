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
#ifndef LIBNFS_SERVICE_TASK_H
#define LIBNFS_SERVICE_TASK_H

#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <ftw.h>
#include <ctime>
#include "ThreadPool.h"
#include "log/Log.h"
#include "PacketStats.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "FileHandleCache.h"
#include "LibnfsCommonMethods.h"

class LibnfsParams {
public:
    time_t               jobStartTime            {0};        /* Start time of backup phase */
    time_t               deleteJobStartTime      {0};        /* Start time of delete phase */
    std::string          dstRootPath {};
    BackupParams         backupParams;
    std::shared_ptr<FileHandleCache> fileHandleCache { nullptr };
    std::shared_ptr<Module::NfsContextWrapper> nfsCtx = { nullptr };
    std::vector<FileHandle> fileHandleList {};
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder;
    void* writeObj = nullptr;
    bool *abort { nullptr };

    LibnfsParams& operator = (const LibnfsParams& rhs)
    {
        if (this != &rhs) {
            jobStartTime = rhs.jobStartTime;
            deleteJobStartTime = rhs.deleteJobStartTime;
            dstRootPath = rhs.dstRootPath;
            fileHandleCache = rhs.fileHandleCache;
            writeObj = rhs.writeObj;
            fileHandleList = rhs.fileHandleList;
            backupParams = rhs.backupParams;
            abort = rhs.abort;
            nfsCtx = rhs.nfsCtx;
            failureRecorder = rhs.failureRecorder;
        }

        return *this;
    }

    LibnfsParams() = default;

    LibnfsParams(const LibnfsParams& rhs)
    {
        jobStartTime = rhs.jobStartTime;
        deleteJobStartTime = rhs.deleteJobStartTime;
        dstRootPath = rhs.dstRootPath;
        fileHandleCache = rhs.fileHandleCache;
        writeObj = rhs.writeObj;
        fileHandleList = rhs.fileHandleList;
        backupParams = rhs.backupParams;
        nfsCtx = rhs.nfsCtx;
        abort = rhs.abort;
        failureRecorder = rhs.failureRecorder;
    }
};

class LibnfsServiceTask : public Module::ExecutableItem {
public:
    LibnfsServiceTask(LibnfsEvent event,
        std::shared_ptr<BackupControlInfo> controlInfo,
        FileHandle& fileHandle, const LibnfsParams& params,
        std::shared_ptr<PacketStats> pktStats)
        : m_event(event), m_controlInfo(controlInfo), m_fileHandle(fileHandle), m_params(params), m_pktStats(pktStats)
    {
        m_fileHandleCache = m_params.fileHandleCache;
        m_nfsCtx = m_params.nfsCtx;
        m_fileHandleList = m_params.fileHandleList;
        m_failureRecorder = m_params.failureRecorder;
    }
    virtual ~LibnfsServiceTask();
    void Exec() override;

private:
    void HandleWriteMeta();
    void HandleDelete();

    int HandleLstatStatus(const std::string &fullPath, struct stat &st, int &delStatIsDir);
    int CompareTypeOfDeleteEntryAndBackupCopy(int delStatIsDir) const;
    void DeleteFilesAndDirectory(std::string &fileName, int delStatIsDir);
    int DeleteDirectory(const std::string &filePath);
    int DeleteFile(std::string &fPath);
    void DetermineDirectoryOrFile(struct stat &st, int &delStatIsDir) const;
    static int UnlinkCb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

    void HandleDirSetMetaFailure(int status, FileHandle &fileHandle);
    int LookupRecursively(std::string targetFilePath);
    int LookupDir(std::string dirPath, std::string &parentDirPath, bool &lookupFailed);
    int LookupSync(std::string dirPath, std::string parentDirPath, uint16_t retryCnt);
    int HandleLookupSyncReqStatus(int status, uint16_t retryCnt, struct nfsfh *nfsfh,
        std::string fileName, std::shared_ptr<Module::NfsContextWrapper> nfs);
    void FillFileHandleCacheWithInvalidDirectoryFh(std::string dirPath);

    int HandleDirLstatStatus(int status, FileHandle &fileHandle, struct nfs_stat_64 st);
    bool IsDirAlreadyExistedInTargetFS(FileHandle &fileHandle, struct nfs_stat_64 st);
    void HandleDirLstatRetry(FileHandle &fileHandle);
    bool IsDirMetaSetRequired(FileHandle &fileHandle);

    void HandleDirMetaRetry(FileHandle &fileHandle);
    void HandleLstatRetry();

    LibnfsEvent m_event { LibnfsEvent::INVALID };
    std::shared_ptr<BackupControlInfo> m_controlInfo { nullptr };
    FileHandle m_fileHandle;
    LibnfsParams m_params;
    std::shared_ptr<Module::NfsContextWrapper> m_nfsCtx = { nullptr };
    std::shared_ptr<PacketStats> m_pktStats = nullptr;
    std::shared_ptr<FileHandleCache> m_fileHandleCache { nullptr };
    std::vector<FileHandle> m_fileHandleList {};
    std::shared_ptr<Module::BackupFailureRecorder> m_failureRecorder { nullptr };
};

#endif // LIBNFS_SERVICE_TASK_H