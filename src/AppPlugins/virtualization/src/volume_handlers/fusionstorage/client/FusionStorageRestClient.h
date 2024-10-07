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
#ifndef FUSION_STORAGE_REST_CLIENT_H
#define FUSION_STORAGE_REST_CLIENT_H

#include "common/model/ModelBase.h"
#include "common/client/RestClient.h"
#include "common/token_mgr/TokenDetail.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

class FusionStorageRestClient : public RestClient {
public:
    FusionStorageRestClient();
    ~FusionStorageRestClient();

    bool CheckParams(ModelBase &model) override;

protected:
    bool UpdateToken(ModelBase &model, std::string &tokenStr);
};

VIRT_PLUGIN_NAMESPACE_END  // namespace VirtPlugin

#endif  // _FUSION_STORAGE_API_FACTORY_H_
