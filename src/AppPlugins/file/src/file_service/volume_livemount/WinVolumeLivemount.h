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
#ifndef WIN_VOLUME_LIVEMOUNT_H
#define WIN_VOLUME_LIVEMOUNT_H

#include <vector>
#include <string>
#include "VolumeLivemount.h"
#include "ApplicationServiceDataType.h"

namespace FilePlugin {
class WinVolumeLivemount : public VolumeLivemount {
public:
    WinVolumeLivemount() = default;
    ~WinVolumeLivemount() override = default;
protected:
    bool PrepareBasicDirectory(const VolumeLivemountExtend& extendInfo) override;
    bool MountVolumes() override;
    bool MountShare() override;

private:
    std::vector<WinVolumeInfo> GetVolumesFromCopy();
    bool MountSingleVolume(const WinVolumeInfo& volume, const std::string& dstPath);
    std::string GetWinSystemDrive();
    void ProcessDstPath(std::string& dstPath) const;
    bool CheckTargetIsValid(const std::string& dstPath);

private:
    std::vector<VolumeLivemountDetail> m_livemountDetails;
    std::string m_volMetaPath;
    std::string m_nasShareDataMountTarget;
    std::string m_nasShareMetaMountTarget;
    std::string m_defaultMountPrefix;
};
}

#endif