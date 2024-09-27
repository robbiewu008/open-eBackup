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
#ifndef ADD_NETWORK_CARD_REQ_H
#define ADD_NETWORK_CARD_REQ_H
 
#include <string>
#include "../CNwareRequest.h"
#include "../BuildNewVMRequest.h"

namespace CNwarePlugin {
class AddNetworkCardRequest : public CNwareRequest {
public:
    AddNetworkCardRequest() {}
    ~AddNetworkCardRequest() {}
 
    void SetDomainId(const std::string &domainId)
    {
        m_domainId = domainId;
    }
 
    std::string GetDomainId()
    {
        return m_domainId;
    }

    void SetAddBridgeInterfaceRequest(const AddBridgeInterfaceRequest &addReq)
    {
        m_addInterfaceReq = addReq;
    }

    int32_t AddNetworkCardReqToJsonString()
    {
        if (!Module::JsonHelper::StructToJsonString(m_addInterfaceReq, m_body)) {
            ERRLOG("Convert AddDiskReq to string failed.");
            return VirtPlugin::FAILED;
        }
        return VirtPlugin::SUCCESS;
    }

private:
    std::string m_domainId;
    AddBridgeInterfaceRequest m_addInterfaceReq;
};
};
 
# endif