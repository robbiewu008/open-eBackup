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
#ifndef OPENSTACK_TOKEN_MGR_H
#define OPENSTACK_TOKEN_MGR_H
#include <mutex>
#include "common/token_mgr/BaseTokenMgr.h"
#include "common/token_mgr/GetTokenResponse.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"

#include "curl_http/HttpStatus.h"

using VirtPlugin::BaseTokenMgr;
using VirtPlugin::ModelBase;
using VirtPlugin::GetTokenResponse;
using VirtPlugin::TokenInfo;

namespace OpenStackPlugin {
class OpenStackTokenMgr : public BaseTokenMgr {
public:
    OpenStackTokenMgr() {};
    ~OpenStackTokenMgr() {};
    static OpenStackTokenMgr& GetInstance();
    bool ReacquireToken(ModelBase &model, std::string &tokenValue) override;
    
protected:
    bool ParseEndpoint(ModelBase &model, const TokenInfo &tokenInfo, std::string &endPoint);
    std::string GetTokenKey(ModelBase &model);
    int GetTimeDiffWithGMT();
};
}

#endif