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
#ifndef WINDOWS_VOLUME_FILE_GRANULAR_RESTORE_H
#define WINDOWS_VOLUME_FILE_GRANULAR_RESTORE_H

#include "WinVolumeFileGranularRestore.h"
#include "volume/VolumeCommonService.h"
#include "VolumeFileGranularRestore.h"
#include "HostCommonStruct.h"

namespace FilePlugin {

class WinVolumeFileGranularRestore : public VolumeFileGranularRestore {
public:
    WinVolumeFileGranularRestore() {};
    ~WinVolumeFileGranularRestore() override {};

protected:
    bool SetupMounts() override;
    bool GenerateSubTaskFromDCacheFCache(const std::string& volumeName) override;
    bool MountSingleVolumeForRest(const WinVolumeInfo& volume, const std::string& dstPath);
    bool InitSubJobInfo(SubJob &subJob, const std::string& ctrlPath, const std::string& volumeName) override;
    bool GenerateRestoreExecuteSubJob(const std::vector<std::string>& controlFileList,
        const std::string& volumeName) override;
    inline bool IsDir(const std::string& name) const;
    int ExecuteVolumeGranularTearDownSubJob() override;
    void FillGranularRestoreScanConfig(
        ScanConfig& scanConfig, const std::string& metaPath, const std::string& outputControlDirPath) override;
    void AddFilterRule(ScanConfig& scanConfig);
    std::string GetMountTargetRootPath(const std::string& volumeName);
    std::vector<WinVolumeInfo> GetVolumesFromCopyForRest();

private:
    void GetScanDriveLetter(const std::string& volumeName);
    std::string m_scanDriveLetter {};
};
}

#endif