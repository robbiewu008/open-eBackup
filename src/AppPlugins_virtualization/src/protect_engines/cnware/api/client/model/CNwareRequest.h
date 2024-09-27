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
#ifndef VIRTUALIZATION_PLUGIN_CNWAREREQUEST_H
#define VIRTUALIZATION_PLUGIN_CNWAREREQUEST_H

#include <string>
#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"

using VirtPlugin::ApiType;
using VirtPlugin::Scope;
using AppProtect::Authentication;
using VirtPlugin::ModelBase;

namespace CNwarePlugin {

class CNwareRequest : public ModelBase {
public:
    CNwareRequest() {};
    ~CNwareRequest() {};
    ApiType GetApiType();
    Scope GetScopeType() const;
    virtual bool BuildBody(std::string &body);
    std::string GetBody();
    void SetIpPort(std::string port);
    std::string GetPort();
    void SetResourceId(const std::string &id);
    std::string GetResourceId();

public:
    std::string url;

protected:
    std::string m_id;
    std::string m_port;
    std::string m_body;
    std::string m_url;
    std::string m_method;
};
};

#endif // VIRTUALIZATION_PLUGIN_CNWAREREQUEST_H
