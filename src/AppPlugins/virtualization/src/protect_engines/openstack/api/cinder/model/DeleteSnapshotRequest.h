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
#ifndef OPENSTACK_DELETE_SNAPSHOT_REQUEST_H
#define OPENSTACK_DELETE_SNAPSHOT_REQUEST_H
 
#include <string>
#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
 
using VirtPlugin::ModelBase;
using VirtPlugin::ApiType;
using VirtPlugin::Scope;
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
class DeleteSnapshotRequest : public ModelBase {
public:
    DeleteSnapshotRequest();
    virtual ~DeleteSnapshotRequest();
 
    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;
 
    std::string GetSnapshotId() const;
    bool SnapshotIdIsSet() const;
    void UnsetSnapshotId();
    void SetSnapshotId(const std::string& id);
    void SetGroupSnapshotId(const std::string& groupSnapshotId);
    std::string GetGroupSnapshotId() const;
    void SetApiVersion(const std::string &apiVersion);
    std::string GetApiVersion() const;
 
protected:
    std::string m_snapshotId;
    bool m_snapshotIdIsSet { false };
    std::string m_groupSnapshotId;
    std::string m_apiVersion;
};
 
OPENSTACK_PLUGIN_NAMESPACE_END
 
#endif // DELETE_SNAPSHOT_REQUEST_H