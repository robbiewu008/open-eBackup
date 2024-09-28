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
#include "GetServerDetailsResponse.h"
#include <common/JsonHelper.h>
#include "log/Log.h"

using namespace VirtPlugin;

namespace {
const std::string MODULE_NAME = "GetServerDetailsResponse";
}
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
GetServerDetailsResponse::GetServerDetailsResponse() {}
 
GetServerDetailsResponse::~GetServerDetailsResponse() {}
 
bool GetServerDetailsResponse::Serial()
{
    if (m_body.empty()) {
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_body, m_serverDetails)) {
        ERRLOG("Failed to trans data from json string to struct.");
        return false;
    }
    Json::Value serverInfo;
    Json::Reader reader;
    if (!reader.parse(m_body, serverInfo)) {
        ERRLOG("Failed to trans data from json string to value.");
        return false;
    }
    if (serverInfo.isMember("server") && serverInfo["server"].isMember("addresses")) {
        Json::Value::Members keys = serverInfo["server"]["addresses"].getMemberNames();
        for (int j = 0; j < keys.size(); ++j) {
            std::string netName = keys[j];
            m_serverDetails.m_hostServerInfo.m_addresses.push_back(netName);
        }
    }
    return true;
}

ServerDetail GetServerDetailsResponse::GetServerDetails() const
{
    return m_serverDetails;
}
 
std::string GetServerDetailsResponse::GetServerStatus() const
{
    return m_serverDetails.m_hostServerInfo.m_status;
}
 
std::string GetServerDetailsResponse::GetOsExtstsVmState() const
{
    return m_serverDetails.m_hostServerInfo.m_oSEXTSTSvmState;
}
OPENSTACK_PLUGIN_NAMESPACE_END
