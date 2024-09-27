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
#ifndef VIRTUALIZATION_PLUGIN_HCS_OPSERVICE_UTILS_H
#define VIRTUALIZATION_PLUGIN_HCS_OPSERVICE_UTILS_H

#include <string>
#include "define/Types.h"
#include "json/json.h"
#include "common/Constants.h"
#include "protect_engines/hcs/resource_discovery/HcsMessageInfo.h"
#include "common/token_mgr/TokenDetail.h"

HCS_PLUGIN_NAMESPACE_BEGIN

class HcsOpServiceUtils {
public:
    static HcsOpServiceUtils *GetInstance();
    HcsOpServiceUtils(const HcsOpServiceUtils &) = delete;
    HcsOpServiceUtils &operator=(const HcsOpServiceUtils &) = delete;
    virtual ~HcsOpServiceUtils();

    void GetAppEnv(const ApplicationEnvironment &appenv);
    bool GetOpServiceInfo();
    bool SetTokenInfo();
    std::string GetToken() const;
    std::string GetExpireAtTime() const;
    std::string GetDomain() const;
    std::string GetRegion() const;
    std::string GetProjectId() const;
    bool GetIsOpServiceEnv() const;

private:
    HcsOpServiceUtils();

private:
    ApplicationEnvironment m_appEnv;
    bool m_isOpServiceEnv { false };
    HcsOpServiceGetInfo m_hcsOpServiceGetInfo;
    HcsOpServiceTokenInfo m_hcsOpServiceTokenInfo;
};

HCS_PLUGIN_NAMESPACE_END
#endif