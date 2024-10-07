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
#ifndef HCS_POWER_ON_SERVER_REQUEST_H
#define HCS_POWER_ON_SERVER_REQUEST_H

#include <string>
#include "common/model/ModelBase.h"
#include "protect_engines/hcs/common/HcsCommonInfo.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "common/token_mgr/TokenDetail.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class PowerOnServerRequest : public ModelBase {
public:
    PowerOnServerRequest();
    ~PowerOnServerRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    /// <summary>
    /// 云服务器ID。
    /// </summary>
    std::string GetServerId() const;
    bool ServerIdIsSet() const;
    void UnsetServerId();
    void SetServerId(const std::string &value);

protected:
    std::string m_serverId;
    bool m_serverIdIsSet;
};
}

#endif