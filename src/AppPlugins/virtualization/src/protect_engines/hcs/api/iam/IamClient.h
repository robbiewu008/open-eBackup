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
#ifndef HCS_IAM_CLIENT_H
#define HCS_IAM_CLIENT_H

#include "protect_engines/hcs/api/iam/model/GetTokenRequest.h"
#include "common/token_mgr/GetTokenResponse.h"
#include "protect_engines/hcs/api/iam/model/GetAdminProjectsRequest.h"
#include "protect_engines/hcs/api/iam/model/GetAdminProjectsResponse.h"
#include "common/client/RestClient.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "protect_engines/hcs/common/HcsCommonInfo.h"

namespace HcsPlugin {
class IamClient : public RestClient {
public:
    IamClient() {};
    ~IamClient() {};
    
    bool CheckParams(ModelBase &model) override;

    std::shared_ptr<GetTokenResponse> GetToken(GetTokenRequest &request);
    std::shared_ptr<GetTokenResponse> VerifyToken(GetTokenRequest &request);
    std::shared_ptr<GetTokenResponse> GetAdminTokenOfProject(GetTokenRequest &request, ModelBase &model);
    std::shared_ptr<GetAdminProjectsResponse> GetAdminOfProjects(GetAdminProjectsRequest &request);

private:
    std::string BuildQueryBody(GetTokenRequest &request, const Scope &tokenType);
};
}

#endif