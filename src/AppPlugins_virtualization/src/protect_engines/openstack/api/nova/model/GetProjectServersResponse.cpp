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
#include "GetProjectServersResponse.h"
OPENSTACK_PLUGIN_NAMESPACE_BEGIN

GetProjectServersResponse::GetProjectServersResponse() {}
GetProjectServersResponse::~GetProjectServersResponse() {}

bool GetProjectServersResponse::Serial()
{
    if (m_body.empty()) {
        ERRLOG("GetProjectServersResponse body is empty.");
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_body, m_serverList)) {
        ERRLOG("Failed to trans data from json string to server list.");
        return false;
    }
    Json::Value serverInfo;
    Json::Reader reader;
    if (!reader.parse(m_body, serverInfo)) {
        ERRLOG("Failed to trans data from json string to value.");
        return false;
    }
    for (int i = 0; i < serverInfo["servers"].size(); ++i) {
        if (!serverInfo["servers"][i].isMember("addresses")) {
            continue;
        }
        Json::Value::Members keys = serverInfo["servers"][i]["addresses"].getMemberNames();
        std::string networks = "";
        for (auto it = keys.begin(); it != keys.end(); ++it) {
            std::string netName = *it;
            networks += netName + ";";
            Json::Value ipList = serverInfo["servers"][i]["addresses"][netName];
            for (int j = 0; j < ipList.size(); ++j) {
                m_serverList.m_serverList[i].m_addresses.push_back(ipList[j]["addr"].asString());
            }
        }
        m_serverList.m_serverList[i].m_networks = networks;
    }
    return true;
}

ServerList GetProjectServersResponse::GetProjectServerList()
{
    return m_serverList;
}

OPENSTACK_PLUGIN_NAMESPACE_END