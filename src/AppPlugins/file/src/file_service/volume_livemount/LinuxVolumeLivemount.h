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
#ifndef LINUX_VOLUME_LIVEMOUNT_H
#define LINUX_VOLUME_LIVEMOUNT_H

#include "VolumeLivemount.h"

namespace FilePlugin {

class LinuxVolumeLivemount : public VolumeLivemount {
public:
    LinuxVolumeLivemount() = default;
    ~LinuxVolumeLivemount() override = default;

protected:
    bool PrepareBasicDirectory(const VolumeLivemountExtend& extendInfo) override;
    bool MountVolumes() override;
    bool MountShare() override;
private:
    bool MountSingleVolumeCopy(const std::string& volumeName, const std::string& volumeDirPath);
};

}

#endif