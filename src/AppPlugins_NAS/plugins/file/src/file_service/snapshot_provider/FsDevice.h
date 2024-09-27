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
#ifndef APPPLUGIN_FILE_FSDEVICE_H
#define APPPLUGIN_FILE_FSDEVICE_H
#include <string>
#include <set>
namespace FilePlugin {
class FsDevice {
public:
    FsDevice();
    FsDevice(uint64_t idevNo, std::string imountPoint, std::string ifsType, std::string ideviceName);
    FsDevice(uint64_t idevNo, std::set<std::string> imountPoints, std::string ifsType, std::string ideviceName);

public:
    uint64_t devNo;                     /* 设备编号:device number */
    std::string mountPoint;             /* 设备挂载点 */
    std::set<std::string> mountPoints;  /* 设备挂载点 */
    std::string fsType;                 /* 设备文件类型：xfs, ext3, ext4,btrfs... */
    std::string deviceName;             /* 设备名：/dev/mapper/xxx */
    std::string lvPath {};              /* 逻辑卷路径：vg/lv */
    bool supportSnapCalled {false};     /* 是否调用lvs判断支持快照 */
    bool isSupportSnapshot {false};     /* 是否支持快照 */
};
}
#endif // APPPLUGIN_FILE_FSDEVICE_H
