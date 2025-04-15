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
#ifndef APPPLUGIN_FILE_SNAPSHOTPROVIDER_H
#define APPPLUGIN_FILE_SNAPSHOTPROVIDER_H
#include "LvmSnapshot.h"

/**
 * Class providing interface for snapshot device volume.
 */
namespace FilePlugin {
class SnapshotProvider {
public:
    virtual ~SnapshotProvider() = default;
    /**
    * 根据文件集合和任务ID创建快照.
    *
    * @param filePath 打快照的文件
    * @param isCrossVolume 是否跨卷打快照
    * @return SnapshotResult  快照创建结果.
    */
    virtual SnapshotResult CreateSnapshot(const std::string& filePath, bool isCrossVolume,
        const std::string& snapshotPercent) = 0;

    /**
    * 根据指定文件查询对应的快照.
    *
    * @param filePath 查询快照的文件名.
    * @return SnapshotResult  快照查询结果.
    */
    virtual SnapshotResult QuerySnapshot(const std::string& filePath) = 0;

    /**
    * 根据指定文件查询对应的快照.
    *
    * @param volumeDevice 快照卷设备名. eg:/dev/vg/lv
    * @param mountPath 挂载路径.
    * @return bool  挂载结果.
    */
    virtual bool MountSnapshot(const std::string& volumeDevice, const std::string& mountPath) = 0;

    /**
   * 删除指定任务ID下所有快照
   *
   * @return SnapshotDeleteResult  快照删除结果.
   */
    virtual SnapshotDeleteResult DeleteAllSnapshots(const std::set<std::string>& snapshotInfos = {}) = 0;
};
}

#endif // APPPLUGIN_FILE_SNAPSHOTPROVIDER_H
