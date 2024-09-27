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
#ifndef CNWARE_DELETE_STORAGE_POOL_REQUEST_H
#define CNWARE_DELETE_STORAGE_POOL_REQUEST_H

#include <string>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {

class DelStoragePoolRequest : public CNwareRequest {
public:
    DelStoragePoolRequest() {}
    ~DelStoragePoolRequest() {}

    void SetPoolId(const std::string &poolId)
    {
        m_poolId = poolId;
    }

    void SetDelStoragePoolRequest(const DeleteStoragePoolReq &req)
    {
        m_deleteStoragePoolReq = req;
    }

    std::string GetPoolId()
    {
        return m_poolId;
    }

    int32_t DelStoragePoolRequestToJson()
    {
        if (!Module::JsonHelper::StructToJsonString(m_deleteStoragePoolReq, m_body)) {
            ERRLOG("Convert DelStoragePoolRequest to json string failed!");
            return VirtPlugin::FAILED;
        }
        return VirtPlugin::SUCCESS;
    }

private:
    std::string m_poolId;
    DeleteStoragePoolReq m_deleteStoragePoolReq;
};
};

#endif