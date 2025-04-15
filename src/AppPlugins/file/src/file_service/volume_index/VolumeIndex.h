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
#ifndef VOLUME_INDEX_H
#define VOLUME_INDEX_H

#include "JobCommonInfo.h"
#include "framework/inc/job_mgr/JobMgr.h"
#include "BasicJob.h"
#include "define/Defines.h"
#include "volume/VolumeCommonService.h"
#include "define/Types.h"
#include "common/JsonHelper.h"
#include "DirCacheParser.h"
#include "Scanner.h"
#include "ScanMgr.h"
#include "ScanConsts.h"

#ifdef WIN32
#include "WinVolumeRestore.h"
#endif
namespace FilePlugin {
enum class VolumeIndexType {
    VOLUME_INDEX_TYPE_FULL,
    VOLUME_INDEX_TYPE_INC
};

class VolumeIndex : public VolumeCommonService {
public:
    VolumeIndex() {};
    virtual ~VolumeIndex() {};
    EXTER_ATTACK int PrerequisiteJob() override;
    EXTER_ATTACK int GenerateSubJob() override;
    EXTER_ATTACK int ExecuteSubJob() override;
    EXTER_ATTACK int PostJob() override;

    enum class SCANNER_TASK_STATUS {
        SCANNER_TASK_STATUS_INIT        = 0,
        SCANNER_TASK_STATUS_INPROGRESS  = 1,
        SCANNER_TASK_STATUS_SUCCESS     = 2,
        SCANNER_TASK_STATUS_ABORTED     = 3,
        SCANNER_TASK_STATUS_FAILED      = 4,
    };

    virtual bool PrepareBasicDirectory();
    virtual bool MountNasShare();
    virtual bool MountVolumes();
    void SetScanHashType();
    void MonitorScanner();
    void CleanIndexMounts();
    void ReportJob(AppProtect::SubJobStatus::type status);
    void FillScanConfigMetaPath(const std::string& pathId);

    static void ScannerCtrlFileCallBack(void *usrData, const std::string &ControlFilePath);
    static void ScannerHardLinkCallBack(void *usrData, const std::string &ControlFilePath);
    static void BackupDirMTimeCallBack(void *usrData, const std::string &ControlFilePath);
    static void BackupDelCtrlCallBack(void *usrData, const std::string &ControlFilePath);

    std::shared_ptr<AppProtect::BuildIndexJob> m_indexPara = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_dataRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_cacheRepo = nullptr;
    std::unique_ptr<Scanner> m_scanner { nullptr };
    ScanConfig m_scanConfig {};

    std::vector<std::string> m_volumesMountPaths;

    std::string m_volumesMountRecordRoot;
    std::string m_nasShareMountTarget;
    std::vector<std::string> m_mountedRecords;
    std::string m_volumesMountTargetRoot;

    // cache/copyId/filemeta
    std::string m_cacheFsPath;
    // cache/preCopyId/filemeta
    std::string m_preCacheFsPath;

protected:
    // 索引任务StorageRepositoryExtendInfo结构体
    struct StorageRepositoryExtendInfo {
        bool isCurrentCopyRepo {};
        int timestamp {};
        std::string copyId;
        std::string fileSystemId;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(isCurrentCopyRepo, isCurrentCopyRepo);
        SERIAL_MEMBER_TO_SPECIFIED_NAME(timestamp, timestamp);
        SERIAL_MEMBER_TO_SPECIFIED_NAME(copyId, copyId);
        SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystemId, fileSystemId);
        END_SERIAL_MEMEBER
    };

    struct RfiGeneratationParam {
        std::string copyId;
        std::vector<std::string> rfiFiles;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(copyId, copyId)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(rfiFiles, rfiFiles)
        END_SERIAL_MEMEBER
    };

    int PrerequisiteJobInner() const;
    int GenerateSubJobInner();
    int ExecuteSubJobInner();
    int PostJobInner();

    std::shared_ptr<AppProtect::BuildIndexJob> GetJobInfoBody();

    int IdentifyRepos();
    int IdentifyRepo(AppProtect::StorageRepository& repo);
    int SetupMetaPath(const StorageRepository& repo);
    void PrintRepo(const StorageRepository& repo) const;
    
    void PrepareForVolumeIncIndex();
    int ProcessVolumeIndex();
    int ProcessVolumeIncIndex();
    int GenerateRfiWithTwoQueues(const std::vector<std::string>& curDirList,
        const std::vector<std::string>& prevDirList);
    int GenerateRfiWithSingleQueue(std::queue<std::string>& curQueue, std::queue<std::string>& prevQueue);
    void CheckIsLastScan(const std::queue<std::string>& curQueue, const std::queue<std::string>& preQueue);
    int GenerateIncRfiInPath(const std::string& prevPath, const std::string& curPath);
    int GenerateFullRfiInPath(const std::string& path, bool isPre);
    virtual bool CopyPreMetaFileToWorkDir() const;

    virtual void ProcessVolumeScan();
    void LoadUmountRecords();
    void UmountVolumesFromRecords();
    void UmountVolumeFromRecord(const std::string& mountRecordJsonPath);

    bool StartScanner();
    std::shared_ptr<Module::DirCacheParser> CreateDcacheObj(const std::string& fname) const;

    void FillScanConfigForGenerateRfi(const std::string& prevDcachePath,
        const std::string& curDcachePath, bool isLastScan, bool isPre);
    virtual void FillScanConfigForScan();
    
    static void GeneratedCopyCtrlFileCb(void *usrData, std::string ctrlFile);
    static void GeneratedHardLinkCtrlFileCb(void *usrData, std::string ctrlFile);
    static void GenerateRfiCtrlFileCb(void *usrData, RfiCbStruct cbParam);
    void FillMonitorScannerVarDetails(SCANNER_TASK_STATUS& scanTaskStatus,
        AppProtect::SubJobStatus::type& jobStatus);

    std::shared_ptr<AppProtect::StorageRepository> m_preRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_curRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_metaRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_preMetaRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_indexRepo = nullptr;
    std::shared_ptr<StorageRepositoryExtendInfo> m_preRepoExtendInfo = nullptr;
    std::shared_ptr<StorageRepositoryExtendInfo> m_curRepoExtendInfo = nullptr;

    std::string m_metaFsPath;
    std::string m_preMetaFsPath;

    SCANNER_STATUS m_scanStatus {};
    bool m_isLastScan {false};
    VolumeIndexType m_indexType { VolumeIndexType::VOLUME_INDEX_TYPE_FULL };

    std::atomic<bool> m_isPreparing {false};
    std::string m_dataPathRoot;
    std::vector<std::string> m_volumeMountRecordJsonList;
};
}

#endif