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
#ifndef VSWITCHS_RESPONSE_H
#define VSWITCHS_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
 
class VswitchsResponse : public VirtPlugin::ResponseModel {
public:
    VswitchsResponse() {}
    ~VswitchsResponse() {}
 
    bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        Json::Value js;
        if (!Module::JsonHelper::JsonStringToJsonValue(m_body, js)) {
            ERRLOG("Convert VswitchsResponse failed.");
            return false;
        }
        for (auto &vs : js) {
            VswitchsRes res;
            if (!Module::JsonHelper::JsonValueToStruct(vs, res)) {
                ERRLOG("Convert VswitchsResponse struct failed.");
                return false;
            }
            m_taskRes.emplace_back(res);
        }
        return true;
    }
 
    std::vector<VswitchsRes> GetVswitchsRes()
    {
        return m_taskRes;
    }
private:
    std::vector<VswitchsRes> m_taskRes;
};
};

#endif