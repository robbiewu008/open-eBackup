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
#ifndef __INTERFACE_REQUEST__
#define __INTERFACE_REQUEST__

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
struct InterfaceReq {
    std::string networkStrategy;
    uint32_t model {1};
    std::string portGroupId;
    uint32_t networkType {0};
    bool isEnabled {false};
    bool isVhostDriver {false};
    std::string vlanId;
    uint32_t mtu;
    uint32_t queues;
    std::string ip;
    std::string vfName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(networkStrategy)
    SERIAL_MEMEBER(model)
    SERIAL_MEMEBER(portGroupId)
    SERIAL_MEMEBER(networkType)
    SERIAL_MEMEBER(isEnabled)
    SERIAL_MEMEBER(isVhostDriver)
    SERIAL_MEMEBER(vlanId)
    SERIAL_MEMEBER(mtu)
    SERIAL_MEMEBER(ip)
    SERIAL_MEMEBER(vfName)
    END_SERIAL_MEMEBER
};

class InterfaceRequest : public CNwareRequest {
public:
    InterfaceRequest() {};
    ~InterfaceRequest() {};

    bool SetInterfaceReq(const BridgeInterfaces &brigde, const int32_t &coreNums, const bool &isEnabled)
    {
        m_interface.networkStrategy = brigde.m_networkStrategy;
        m_interface.model = brigde.m_model;
        m_interface.portGroupId = brigde.m_portGroupId;
        m_interface.networkType = brigde.m_networkType;
        m_interface.isVhostDriver = brigde.m_isVhostDriver;
        m_interface.vlanId = brigde.m_vlanId;
        m_interface.mtu = brigde.m_mtu;
        m_interface.queues = brigde.m_queues;
        m_interface.ip = brigde.m_ip;
        m_interface.vfName = brigde.m_vfName;
        m_interface.isEnabled = isEnabled;
        m_mac = brigde.m_mac;
        Json::Value tempBody;
        if (!Module::JsonHelper::StructToJsonValue(m_interface, tempBody)) {
            ERRLOG("Convert snapinfo to json value failed!");
            return false;
        }
        if (coreNums > MIN_CPU_CORES_FOR_QUEUE || coreNums == 0) {
            INFOLOG("Have more than 1 cpu or cpu not set, config queue.");
            tempBody["queues"] = m_interface.queues > coreNums ? coreNums : m_interface.queues;
        }
        Json::FastWriter writer;
        m_body = writer.write(tempBody);
        DBGLOG("SetInterfaceReq. %s", m_body.c_str());
        return true;
    }

    std::string GetMac()
    {
        return std::move(m_mac);
    }

private:
    std::string m_mac;
    InterfaceReq m_interface;
};
};
#endif