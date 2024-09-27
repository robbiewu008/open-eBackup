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
#ifndef MIGRATION_REQUEST_H
#define MIGRATION_REQUEST_H

#include <string>
#include "../CNwareRequest.h"
#include "common/Constants.h"

namespace CNwarePlugin {

class MigrationRequest : public CNwareRequest {
public:
    MigrationRequest() {}
    ~MigrationRequest() {}

    void AddMigVol(const std::string &bus, const std::string &dev,
        const std::string &preallocation, const std::string &destStoragePoolId)
    {
        MigrationVol migVol;
        migVol.m_bus = bus;
        migVol.m_dev = dev;
        migVol.m_preallocation = preallocation;
        migVol.m_destStoragePoolId = destStoragePoolId;
        m_volList.emplace_back(migVol);
        return;
    }

    bool SetMigReq(const std::string &destHostId)
    {
        MigrationVolReq reqBody;
        reqBody.m_destHostId = destHostId;
        reqBody.m_migrateVols = m_volList;
        m_hostId = destHostId;
        if (!Module::JsonHelper::StructToJsonString(reqBody, m_body)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return false;
        }
        return true;
    }

private:
    std::vector<MigrationVol> m_volList;
    std::string m_hostId;
};
};
#endif