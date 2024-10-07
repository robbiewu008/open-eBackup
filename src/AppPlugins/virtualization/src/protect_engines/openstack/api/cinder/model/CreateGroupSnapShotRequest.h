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
#ifndef CREATE_GROUP_SNAPSHOT_REQUEST_H
#define CREATE_GROUP_SNAPSHOT_REQUEST_H

#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"
#include "GroupDetails.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"

using VirtPlugin::ModelBase;
using VirtPlugin::ApiType;
using VirtPlugin::Scope;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class CreateGroupSnapShotRequest : public ModelBase {
public:
    CreateGroupSnapShotRequest();
    virtual ~CreateGroupSnapShotRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    GroupSnapShotRequestBodyMsg GetBody() const;
    bool BodyIsSet() const;
    void UnsetBody();
    void SetBody(const GroupSnapShotRequestBodyMsg& body);
    std::string GetApiVersion() const;
    void SetApiVersion(const std::string apiVersion);
    bool ApiVersionIsSet() const;

protected:
    GroupSnapShotRequestBodyMsg m_body;
    std::string m_apiVersion;
    bool m_apiVersionIsSet { false };
    bool m_bodyIsSet { false };
};
OPENSTACK_PLUGIN_NAMESPACE_END

#endif