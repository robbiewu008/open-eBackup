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
#ifndef IO_DEVICE_DEF_H
#define IO_DEVICE_DEF_H
#include <string>
#include "IOCommonDef.h"
#ifndef WIN32
#include "eSDKOBS.h"
#endif
#include "common/CleanMemPwd.h"

namespace Module {
const std::string IODeviceModule = "IODevice";

enum IODeviceType {
    IODeviceFileSystem = 0,
    IODeviceS3System,
    IODeviceNotExist
};

struct IODeviceInfo {
#ifndef WIN32
    IODeviceInfo() : using_https(0), style(OBS_URI_STYLE_PATH)
    {}
#endif
    ~IODeviceInfo()
    {
        CleanMemoryPwd(password);
        CleanMemoryPwd(cert);
    }

    std::string user_name;    // login device user name
    std::string password;     // login device user password
    std::string path_prefix;  // access device path prefix
    std::string cert;
    bool using_https;
#ifndef WIN32
    obs_uri_style style;
#endif
    HTTP_PROXY_INFO HttpProxyInfo;
    SPEED_UP_INFO SpeedUpInfo;
};
}
#endif  // IO_DEVICE_DEF_H
