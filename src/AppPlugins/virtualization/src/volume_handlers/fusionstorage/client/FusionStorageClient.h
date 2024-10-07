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
#ifndef HCS_FUSION_STORAGE_CLIENT_H
#define HCS_FUSION_STORAGE_CLIENT_H

#include "common/client/RestClient.h"
#include "GetFusionStorageRequest.h"
#include "common/model/ResponseModel.h"

namespace HcsPlugin {
class FusionStorageClient : public RestClient {
public:
    FusionStorageClient() {};
    ~FusionStorageClient() {};

    bool CheckParams(ModelBase &model) override;

    std::shared_ptr<ResponseModel> GetToken(GetFusionStorageRequest &request, AuthObj &storageAuth);

private:
    std::string BuildQueryBody(GetFusionStorageRequest &request);
};
}

#endif