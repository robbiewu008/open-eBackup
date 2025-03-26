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
#ifndef VIRTUALIZATION_PLUGIN_HCS_TOKEN_MGR_H
#define VIRTUALIZATION_PLUGIN_HCS_TOKEN_MGR_H

#include <mutex>
#include "common/token_mgr/BaseTokenMgr.h"
#include "common/token_mgr/GetTokenResponse.h"
#include "common/model/ModelBase.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "protect_engines/hcs/api/iam/model/GetTokenRequest.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class HCSTokenMgr : public BaseTokenMgr {
public:
    HCSTokenMgr() {};
    ~HCSTokenMgr() {};
    static HCSTokenMgr& GetInstance();
    bool ReacquireToken(ModelBase &model, std::string &tokenValue) override;

protected:
    void ReplaceProjectId(const std::string apiType, const std::string &newId, std::string &url, bool isOpService = false);
    std::string GetTokenKey(ModelBase &model) override;
    bool ParseEndpoint(ModelBase &model, const TokenInfo &tokenInfo, std::string &endPoint) override;
    void InitGetTokenRequest(GetTokenRequest &getTokenRequeset, ModelBase &model);
};
}

#endif // VIRTUALIZATION_PLUGIN_HCS_TOKEN_MGR_H
