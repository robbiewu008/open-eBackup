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
#ifndef FUSION_STORAGE_REST_APIREQUEST_H
#define FUSION_STORAGE_REST_APIREQUEST_H

#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"

VIRT_PLUGIN_NAMESPACE_BEGIN
class FusionStorageRestApiRequest : public ModelBase {
public:
    FusionStorageRestApiRequest();
    ~FusionStorageRestApiRequest();

    // 设置scopeValue类型
    virtual Scope GetScopeType() const override;

    // 设置api类型
    virtual ApiType GetApiType() override;

    Scope GetTokenType() const;
    void SetTokenType(const Scope &value);

    std::string GetMgrPort() const;
    void SetMgrPort(const std::string &value);

    std::string GetToken() const;
    void SetToken(const std::string &value);
protected:
    Scope m_tokenType;
    std::string m_mgrPort;
    std::string m_token;
};

VIRT_PLUGIN_NAMESPACE_END  // namespace VirtPlugin

#endif  // _FUSION_STORAGE_API_FACTORY_H_