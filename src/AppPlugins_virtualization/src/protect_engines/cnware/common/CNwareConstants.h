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
#ifndef CNWARE_CONSTANTS_H
#define CNWARE_CONSTANTS_H

#include <vector>
#include <string>
#include "protect_engines/cnware/common/StorageStruct.h"

namespace CNwarePlugin {

enum PoolType {
    None,
    FCSAN,
    IPSAN,
    NAS,
    Distribute,
    Local,
    NVMe
};

enum PoolStatus {
    CREATING = 1,
    USED,
    UNUSED
};

const std::map<std::string, CNwareType> typeMap = {
{"CNwareHostPool", CNwareType::Pool},
{"CNwareHost",     CNwareType::Host},
{"CNwareCluster",  CNwareType::Cluster},
{"CNwareVm",       CNwareType::VM},
{"CNwareDisk",     CNwareType::Disk},
{"CNware",     CNwareType::All}
};
const std::string CNWARE_VERSION_V1 = "8.2";
const std::string PREALLOCATION_OFF = "off";  // 精简置备类型
const int CNWARE_ATTACH_LIMIT_V1 = 16; // CNware 8.2 版本
const int CNWARE_ATTACH_LIMIT_V2 = 60;
}
#endif // CNWARE_CONSTANTS_H