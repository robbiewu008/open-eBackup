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
#ifndef __PORTGROUP_RESPONSE_H__
#define __PORTGROUP_RESPONSE_H__

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {

class PortGroupResponse : public VirtPlugin::ResponseModel {
public:
    PortGroupResponse() {}
    ~PortGroupResponse() {}

    bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        Json::Value js;
        if (!Module::JsonHelper::JsonStringToJsonValue(m_body, js)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        for (auto &unit : js) {
            PortGroup pg;
            if (!Module::JsonHelper::JsonValueToStruct(unit, pg)) {
                ERRLOG("Convert %s failed.", WIPE_SENSITIVE(unit).c_str());
                return false;
            }
            m_data.emplace_back(pg);
        }
        return true;
    }

    std::vector<PortGroup> GetPortGroupInfo()
    {
        return m_data;
    }

private:
    std::vector<PortGroup> m_data;
};
}
#endif