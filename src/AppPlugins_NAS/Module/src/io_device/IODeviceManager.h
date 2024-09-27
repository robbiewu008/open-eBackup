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
#ifndef IO_DEVICE_MANAGER_H
#define IO_DEVICE_MANAGER_H

#include <string>
#include <map>
#include "IODeviceInterface.h"
#include <memory>
#include "IODeviceDef.h"

namespace Module {
class IODeviceManager {
public:
    static IODeviceManager &GetInstance();

    bool RegisterIODevice(const IODeviceInfo &info,
                          std::function<std::shared_ptr<IODevice>(const IODeviceInfo &, OBJECT_TYPE)> creator);

    void UnregisterIODevice(const std::string &path_prefix);

    std::shared_ptr<IODevice> CreateIODeviceByPath(const std::string &path, OBJECT_TYPE fileType);

    IODeviceInfo GetDeviceInfo(const std::string &path);
    
    bool IsInitS3SDK();

private:
    struct IODeviceRegInfo {
        IODeviceInfo device_info;
        unsigned int ref_count;
        std::function<std::shared_ptr<IODevice>(const IODeviceInfo &path, OBJECT_TYPE)> creator;
    };
    IODeviceManager();
    virtual ~IODeviceManager();
    IODeviceManager(const IODeviceManager &);
    const IODeviceManager &operator=(const IODeviceManager &);

    obs_uri_style GetS3URLStyle(const std::string &path);
    bool CheckIPV4(const std::string& ip);
    bool CheckIPV6(const std::string& ip, bool& withBracket);
    bool CheckIP(const std::string &path);

    std::map<std::string, IODeviceRegInfo> m_IODeviceRegInfos;
    std::mutex m_Mutex;
    std::mutex m_InitS3Mutex;
    bool isInitS3SDK;
    bool m_needInitS3;
};
}
#endif  // IO_DEVICE_MANAGER_H
