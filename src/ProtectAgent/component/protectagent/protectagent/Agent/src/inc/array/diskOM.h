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
#ifndef __AGENT_DISK_OM_H__
#define __AGENT_DISK_OM_H__

#include <vector>
#include <map>

#include "array/storage_defines.h"

// oecan mobility used
class DiskOM {
public:
    static mp_int32 IsSSD(media_type_t& mediaType, mp_string& strDeviceName, mp_string& strDeviceNum);
    static mp_int32 GetDiskSize(mp_string& strDevieName, mp_uint64& iDiskSize);
private:
    static mp_int32 GetDiskHWType(std::vector<mp_string>& vecResult, media_type_t& mediaType,
    mp_string& strDeviceName, mp_string& strDeviceNum);
};

#endif

