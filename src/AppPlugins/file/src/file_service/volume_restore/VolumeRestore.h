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
#ifndef VOLUME_RESTORE_H
#define VOLUME_RESTORE_H

#include "constant/Defines.h"
#include "volume/VolumeCommonService.h"
#include "VolumeProtector.h"
#include "BasicJob.h"

namespace FilePlugin {

#define ENTER                                                                                                      \
    do {                                                                                                           \
        m_mainJobRequestId = GenerateHash(m_jobId);                                                                \
        INFOLOG("Enter %s, jobId: %s, subJobId: %s", m_jobCtrlPhase.c_str(), m_jobId.c_str(), m_subJobId.c_str()); \
    } while (0)

#define EXIT                                                                                                      \
    do {                                                                                                          \
        INFOLOG("Exit %s, jobId: %s, subJobId: %s", m_jobCtrlPhase.c_str(), m_jobId.c_str(), m_subJobId.c_str()); \
    } while (0)

const auto CURR_META = "latest";
constexpr uint64_t NUM0 = 0;
constexpr uint64_t NUM1 = 1;
constexpr uint64_t NUM2 = 2;
constexpr uint64_t NUM3 = 3;
const std::string DEFAULT_VOLUME_COPY_NAME = "volumeprotect";

struct RestoreInfo {
    std::string volumeName;
    std::string volumeId;
    std::string dataDstPath;
    uint32_t subTaskType;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeId, volumeId);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeName, volumeName);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dataDstPath, dataDstPath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(subTaskType, subTaskType);
    END_SERIAL_MEMEBER
};

struct CopyVolumeInfo {
    std::string volumeId;
    std::string dataDstPath;
    uint32_t subTaskType;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeId, uuid);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dataDstPath, volumePath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(subTaskType, subTaskType);
    END_SERIAL_MEMEBER
};

struct BareMetalRestoreParam {
    std::string enableBareMetalRestore;
    std::string restoreNonSystemVolume;
    std::string rebootSystemAfterRestore;
    std::string winVolumeRestoreType;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(enableBareMetalRestore, enable_bare_metal_restore);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(restoreNonSystemVolume, restore_non_system_volume);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(rebootSystemAfterRestore, reboot_system_after_restore);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(winVolumeRestoreType, win_volume_restore_type);
    END_SERIAL_MEMEBER
};

struct MountInfo {
    std::string mountPoint;
    std::string mountOper;
    std::string deviceName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountPoint, mountPoint);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountOper, mountOper);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceName, deviceName);
    END_SERIAL_MEMEBER
};

class VolumeRestore : public VolumeCommonService {
public:
    VolumeRestore(){};
    ~VolumeRestore(){};
    EXTER_ATTACK int PrerequisiteJob() override;
    EXTER_ATTACK int GenerateSubJob() override;
    EXTER_ATTACK int ExecuteSubJob() override;
    EXTER_ATTACK int PostJob() override;

protected:
    // init
    virtual bool InitJobInfo();
    virtual bool InitRepoInfo();
    virtual bool InitRestoreInfo();
    virtual bool InitBareMetalRestoreInfo();
    virtual bool InitInfo();

protected:
    virtual int PrerequisiteJobInner();
    virtual int GenerateSubJobInner();
    virtual int ExecuteSubJobInner();
    virtual int PostJobInner();
    virtual bool IsAbort() const;
    virtual void DeleteSharedResources() const;
    virtual void DeleteSubBackupResources() const;
    virtual void FillRestoreConfig(volumeprotect::task::VolumeRestoreConfig &restoreInfo);
    virtual bool CreateRestoreSubJob(RestoreInfo &restoreInfo, uint32_t stage);
    virtual void ReportBMRConfigurationLabel();
    virtual bool StartRestore(const volumeprotect::task::VolumeRestoreConfig &restoreConfig);

    virtual bool ScanVolumesToGenerateTask();
    virtual bool GenerateTearDownTask();
    virtual int ExecuteDataCopyVolume();
    virtual int ExecuteTearDownVolume();
    virtual bool CheckBMR(const BareMetalRestoreParam& param);

    virtual bool IsSystemVolume(const std::string &volumeName) const;
    virtual bool BareMetalRestore();
    virtual void HandleRestoreErrorCode();
    virtual void PostProcess();
    void KeepJobAlive();

protected:
    std::shared_ptr<AppProtect::RestoreJob> m_jobInfoPtr{nullptr};
    size_t m_mainJobRequestId{0};

    std::string m_sysInfoPath;
    std::string m_mountInfoFilePath;

    bool m_enableBareMetalRestore{false};
    bool m_restoreNonSystemVolume{false};
    bool m_rebootSystemAfterRestore{false};
    bool m_jobComplete{false};

    RestoreInfo m_restoreInfo;
    std::vector<RestoreInfo> m_restoreInfoSet;

    /* Used for reporting job progress log to agent */
    ActionResult m_logResult{};
    SubJobDetails m_logSubJobDetails{};
    LogDetail m_logDetail{};
    std::vector<LogDetail> m_logDetailList;
    static uint32_t m_numberOfSubTask;
};
}  // namespace FilePlugin

#endif