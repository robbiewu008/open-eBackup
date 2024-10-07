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
#ifndef CNWARE_GET_DISK_INFO_ON_STORAGE_REQUEST_H
#define CNWARE_GET_DISK_INFO_ON_STORAGE_REQUEST_H

#include <string>
#include "CNwareRequest.h"

namespace CNwarePlugin {
class GetDiskInfoOnStorageRequest : public CNwareRequest {
public:
    GetDiskInfoOnStorageRequest() {}
    ~GetDiskInfoOnStorageRequest() {}

    void SetVolId(const std::string &volId)
    {
        m_volId = volId;
    }

    std::string GetVolId()
    {
        return m_volId;
    }

private:
    std::string m_volId;
};
};

#endif