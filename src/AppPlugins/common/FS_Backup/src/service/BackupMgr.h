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
#ifndef BACKUP_MGR_H
#define BACKUP_MGR_H

#include <memory>
#include "Backup.h"
#include "define/Defines.h"

namespace FS_Backup {
using PrintBackupAdvanceParams = void (*)(std::shared_ptr<BackupAdvanceParams>);

class AGENT_API BackupMgr {
public:
    static std::unique_ptr<Backup> CreateBackupInst(const BackupParams& backupParams);
    /**
    * 合并同目录下的所有备份子任务的sqlite，该接口耗时可能较长，插件另起线程调用，并等待完成
    *
    * @param metaPath 备份的元数据路径
    * @return isComplete 返回值，接口处理完成后设置true
    */
    static int MergedbFile(const std::string &metaPath, bool &isComplete);

private:
    static BackupPlatform GetBackupPlatform(BackupIOEngine ioEngine);
    static bool ValidateBackupParams(const BackupParams& backupParams);
    static void PrintBackupParams(const BackupParams &backupParams);
    static void PrintBackupParamsObject(std::shared_ptr<BackupAdvanceParams> backupAdvParams);
    static void PrintBackupParamsPosix(std::shared_ptr<BackupAdvanceParams> backupAdvParams);
    static void PrintBackupParamsWin32(std::shared_ptr<BackupAdvanceParams> backupAdvParams);
    static void PrintBackupParamsLibnfs(std::shared_ptr<BackupAdvanceParams> backupAdvParams);
    static void PrintBackupParamsLibsmb(std::shared_ptr<BackupAdvanceParams> backupAdvParams);
    static void PrintBackupParamsAntiRansomware(std::shared_ptr<BackupAdvanceParams> backupAdvParams);
    static void PrintBackupSrcParams(const BackupParams &backupParams);
    static void PrintBackupDstParams(const BackupParams &backupParams);
};
}

#ifdef __cplusplus
extern "C" {
// C语言接口，因此用C语言命名风格
typedef struct AntiParam_s {
    int antiType;           /* WORM =0 , ENTROPY =1 */
    uint64_t nfsAtime;     /* 存储元数据中的atime */
    char *nfsMode;        /* 类似："a+w" , "ugo-w", “a=w” */
} AntiParam;

typedef struct BackupConf_S {
    int reqID;                         /* */
    char *ip;                          /* nas share ip */
    char *sharePath;                   /* nas share */
    char *metaPath;                    /* Metadata path for control files of nas share */
    int phase;                         /* BACKUP_PHASE_ANTI_ANSOMWARE = 5 */
    AntiParam antiParam;
} BackupConf;

const int BACKUP_COMPLETED = 1;
const int BACKUP_INPROGRESS = 2;
const int BACKUP_FAILED = 3;

const int BACKUP_PHASE_COPY = 1;
const int BACKUP_PHASE_DELETE = 2;
const int BACKUP_PHASE_HARDLINK = 3;
const int BACKUP_PHASE_DIR = 4;
const int BACKUP_PHASE_ANTI_ANSOMWARE = 5;

/* use to init backup log */
AGENT_API extern void InitLog(const char* fullLogPath, int logLevel);

/* use to create a backup copy instance */
AGENT_API extern void* CreateBackupInst(
    const char* source, const char* destination, const char* metaPath, int phase, bool writeMeta);

#ifdef _NAS
/* use to create a backup copy instance for anti-ransomware phase*/
extern void* CreateAntiBackupInst(BackupConf backupConf);
#endif

/* use to configure read/write thread number in thread pool */
AGENT_API extern int ConfigureThreadPool(void* backupHandle, int readThreadNum, int writeThreadNum);

/* use to configure max memory */
AGENT_API int ConfigureMemory(void* backupHandle, int maxMemory);

/* use to enqueue the control file(file list) to a backup copy instance */
AGENT_API extern int Enqueue(void* backupHandle, const char* backupControlFile);

/* use to start a backup copy instance */
AGENT_API extern int Start(void* backupHandle);

/* use to get backup status from a running backup copy instance */
AGENT_API extern int GetStatus(void* backupHandle);

/* use to get backup statistics from a running backup copy instance */
AGENT_API extern void GetStats(void* backupHandle, BackupStatistics* backupStats);

/* use to destroy a backup copy instance */
AGENT_API extern void DestroyBackupInst(void* backupHandle);
};
#endif

#endif // BACKUP_MGR_H