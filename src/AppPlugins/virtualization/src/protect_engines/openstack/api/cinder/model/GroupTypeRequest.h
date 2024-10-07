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
#ifndef CREATE_GROUP_TYPE_REQUEST_H
#define CREATE_GROUP_TYPE_REQUEST_H

#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"
#include "GroupDetails.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"

using VirtPlugin::ModelBase;
using VirtPlugin::ApiType;
using VirtPlugin::Scope;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class GroupTypeRequest : public ModelBase {
public:
    GroupTypeRequest();
    virtual ~GroupTypeRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    GroupTypeRequestBodyMsg GetBody() const;
    bool BodyIsSet() const;
    void UnsetBody();
    void SetBody(const GroupTypeRequestBodyMsg& body);
    void SetGroupTypeId(const std::string &groupTypeId);
    std::string GetApiVersion() const;
    void SetApiVersion(const std::string apiVersion);
    std::string GetGroupTypeId() const;

protected:
    GroupTypeRequestBodyMsg m_body;
    bool m_bodyIsSet { false };
    std::string m_groupTypeId;
    std::string m_apiVersion;
};
OPENSTACK_PLUGIN_NAMESPACE_END

#endif