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
#ifndef GET_SNAPSHOT_REQUEST_H
#define GET_SNAPSHOT_REQUEST_H

#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"

using VirtPlugin::ModelBase;
using VirtPlugin::ApiType;
using VirtPlugin::Scope;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class GetSnapshotRequest : public ModelBase {
public:
    GetSnapshotRequest();
    virtual ~GetSnapshotRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    std::string GetSnapshotId() const;

    void SetSnapshotId(const std::string& value);
    bool SnapshotIdIsSet() const;
    void UnsetSnapshotId();
    void SetApiVersion(const std::string apiVersion);
    std::string GetApiVersion() const;
    void SetGroupSnapshotId(const std::string& groupSnapshotId);
    std::string GetGroupSnapshotId() const;

protected:
    std::string m_snapshotId; // 快照ID。
    bool m_snapshotIdIsSet;
    std::string m_apiVersion;
    std::string m_groupSnapshotId;
};
OPENSTACK_PLUGIN_NAMESPACE_END

#endif