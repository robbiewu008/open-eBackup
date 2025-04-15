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
#ifndef APPPLUGIN_FILE_DEVICEMOUNT_H
#define APPPLUGIN_FILE_DEVICEMOUNT_H
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <mutex>
#include "FsDevice.h"
#include "define/Defines.h"
#ifdef WIN32
#include <optional>
#endif

namespace FilePlugin {
class DeviceMount {
public:
    DeviceMount();
    // Initialize the list of all devices in the system.
    // Before calling other methods, you need to initialize it first.
    bool LoadDevice();
    /**
     * Get the file system type where the specified file or directory is located
     * @param file or path.
     * @return file system type，eg:ext4,xfs.
    */
    std::string GetFsType(const std::string file);

    /**
     * Get volumeId for path
     * @param file or path.
     * @return device volume id.
    */
    std::string GetVolumeId(const std::string& path);

#ifdef WIN32
    void GetWinVolumeId(std::string path, std::optional<uint64_t>& volumeId);
    void GetNetVolume();
#endif

#ifdef SOLARIS
    void ExecMountInfo(std::vector<std::string>& mountResult, std::map<std::string, std::string>& dfMap);
#endif
    /**
     * Get subvolumes in the specified directory.
     * @param path path.
     * @param excludeFsType Excluded file system types，eg:nfs,xfs.
     * @return Subvolume device list.
    */
    bool GetSubVolumes(std::string path, std::vector<std::shared_ptr<FsDevice>>& outputEntryList);
    /**
     * Get the device information of the file system
     * where the specified file or directory is located.
     * @param file file or path.
     * @return List of file system devices where the file is located.
    */
    std::shared_ptr<FsDevice> FindDevice(const std::string& file);
    /**
     * Check whether the path is valid mountpoint.
     * @param file path.
     * @return true or false
    */
    bool GetInValidMountPoints(std::set<std::string>& invalidMountPoints);
    bool CheckWhetherMountPoint(const std::string& path);
    ~DeviceMount();
    std::map<uint64_t, std::shared_ptr<FsDevice>> m_fsDeviceMap;
    std::map<uint64_t, std::shared_ptr<FsDevice>> m_fsDeviceMntMap;
    std::set<std::string> m_invalidMountPoints;
    std::mutex m_mtxLock {};
};
}

#endif // APPPLUGIN_FILE_DEVICEMOUNT_H
