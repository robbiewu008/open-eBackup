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
#ifndef DELETE_VOLUME_GROUP_REQUEST_H
#define DELETE_VOLUME_GROUP_REQUEST_H
 
#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"
#include "GroupDetails.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
 
using VirtPlugin::ModelBase;
using VirtPlugin::ApiType;
using VirtPlugin::Scope;
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
class DeleteVolumeGroupRequest : public ModelBase {
public:
    DeleteVolumeGroupRequest();
    virtual ~DeleteVolumeGroupRequest();
 
    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;
 
    DeleteVolumeGroupRequestBodyMsg GetBody() const;
    void SetBody(const DeleteVolumeGroupRequestBodyMsg& body);
    void SetApiVersion(const std::string apiVersion);
    std::string GetApiVersion() const;
    void SetGroupId(const std::string& groupId);
    std::string GetGroupId();
 
protected:
    DeleteVolumeGroupRequestBodyMsg m_body;
    std::string m_apiVersion;
    std::string m_groupId;
};
OPENSTACK_PLUGIN_NAMESPACE_END
 
#endif