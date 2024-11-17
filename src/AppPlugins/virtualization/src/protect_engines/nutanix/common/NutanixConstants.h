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
#ifndef NUTANIX_CONSTANTS_H
#define NUTANIX_CONSTANTS_H

#include <vector>
#include <string>

namespace NutanixPlugin {

enum class PoolType {
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

enum class NutanixType {
    Pool,
    Cluster,
    Host,
    VM,
    Disk,
    All
};

const std::map<std::string, NutanixType> typeMap = {
    {"NutanixHostPool", NutanixType::Pool},
    {"NutanixHost",     NutanixType::Host},
    {"NutanixCluster",  NutanixType::Cluster},
    {"NutanixVm",       NutanixType::VM},
    {"NutanixDisk",     NutanixType::Disk},
    {"Nutanix",     NutanixType::All}
};

const int NUTANIX_ATTACH_LIMIT = 256;
const std::string BUS_TYPE_STR_SCSI = "scsi";
const std::string DEFAULT_AFFINITY_POLICY = "AFFINITY"; // 默认主机亲和策略
}
#endif // NUTANIX_CONSTANTS_H