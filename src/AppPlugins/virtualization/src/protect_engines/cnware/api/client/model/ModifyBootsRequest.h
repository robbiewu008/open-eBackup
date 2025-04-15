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
#ifndef MODIFY_VM_BOOTS_REQUEST_H
#define MODIFY_VM_BOOTS_REQUEST_H

#include <string>
#include "CNwareRequest.h"

namespace CNwarePlugin {
struct BootItem {
    std::string bus;
    std::string dev;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(bus);
    SERIAL_MEMEBER(dev);
    END_SERIAL_MEMEBER
};

struct UpdateDomainBootRequest {
    uint32_t bootType = 0;
    std::vector<BootItem> boots {};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(bootType);
    SERIAL_MEMEBER(boots);
    END_SERIAL_MEMEBER
};

class ModifyBootsRequest : public CNwareRequest {
public:
    ModifyBootsRequest() {}
    ~ModifyBootsRequest() {}

    void SetDomainId(const std::string &domainId)
    {
        m_domainId = domainId;
    }

    std::string GetDomainId()
    {
        return m_domainId;
    }

    void SetUpdateDomainBootRequest(const UpdateDomainBootRequest &req)
    {
        m_req = req;
    }

    int32_t UpdateDomainBootRequestToJson()
    {
        if (m_req.boots.empty()) {
            Json::Value bodyJson;
            // boots为null时，接口会卡住，因此为空时不传入该参数
            bodyJson["bootType"] = m_req.bootType;
            Json::Value bootArray(Json::arrayValue);
            bodyJson["boots"] = bootArray;
            Json::FastWriter writer;
            m_body = writer.write(bodyJson);
            INFOLOG("Boot info: %s.", WIPE_SENSITIVE(m_body).c_str());
            return VirtPlugin::SUCCESS;
        }
        if (!Module::JsonHelper::StructToJsonString(m_req, m_body)) {
            ERRLOG("Convert update domain boots req to json string failed!");
            return VirtPlugin::FAILED;
        }
        return VirtPlugin::SUCCESS;
    }
private:
    std::string m_domainId;
    UpdateDomainBootRequest m_req;
};
};

#endif