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
#ifndef __STORAGEPOOL_REQUEST_H__
#define __STORAGEPOOL_REQUEST_H__

namespace CNwarePlugin {

class StoragePoolRequest : public CNwareRequest {
public:
    explicit StoragePoolRequest(std::string hostId) : m_hostId(hostId) {}
    ~StoragePoolRequest() {}

    std::string GetHostId()
    {
        return m_hostId;
    }

private:
    std::string m_hostId;
};
} /* namespace CNwarePlugin */
#endif