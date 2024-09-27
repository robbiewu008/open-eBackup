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
#ifndef HCS_GET_FUSION_STORAGE_REQUEST_H
#define HCS_GET_FUSION_STORAGE_REQUEST_H

#include "common/model/ModelBase.h"

namespace HcsPlugin {
using namespace VirtPlugin;

class GetFusionStorageRequest : public ModelBase {
public:
    GetFusionStorageRequest();
    ~GetFusionStorageRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    Scope GetTokenType() const;
    bool TokenTypeIsSet() const;

    std::string GetFusionStoragePort() const;
    void SetFusionStoragePort(const std::string &value);

    std::string GetFusionStorageUserName() const;
    void SetFusionStorageUserName(const std::string &value);

    std::string GetFusionStoragePassword() const;
    void SetFusionStoragePassword(const std::string &value);

protected:
    Scope m_tokenType;
    bool m_tokenTypeIsSet = false;
    std::string m_platformIp;
    bool m_platformIpIsSet = false;

private:
    std::string m_port;
    std::string m_userName;
    std::string m_password;
};
}

#endif