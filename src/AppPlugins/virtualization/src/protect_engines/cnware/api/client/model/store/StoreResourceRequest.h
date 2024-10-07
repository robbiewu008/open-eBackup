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
#ifndef STORAGE_RESOURCE_REQUEST_H
#define STORAGE_RESOURCE_REQUEST_H

#include <string>
#include "../CNwareRequest.h"
#include "common/Constants.h"

namespace CNwarePlugin {

class StoreResourceRequest : public CNwareRequest {
public:
    StoreResourceRequest() {}
    ~StoreResourceRequest() {}

    bool SetStoreResourceReq(std::string name, std::string storeId)
    {
        StoreResourceReq req;
        req.m_name = name;
        req.m_storeId = storeId;
        if (!Module::JsonHelper::StructToJsonString(req, m_body)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return false;
        }
        return true;
    }
};
};
#endif