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
#ifndef GET_SNAPSHOT_LIST_REQUEST_H
#define GET_SNAPSHOT_LIST_REQUEST_H
 
#include "common/model/ModelBase.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
 
using VirtPlugin::ModelBase;
using VirtPlugin::ApiType;
using VirtPlugin::Scope;
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
class GetSnapshotListRequest : public ModelBase {
public:
    GetSnapshotListRequest();
    virtual ~GetSnapshotListRequest();
 
    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;
    std::string GetVolumeId() const;
    void SetVolumeId(const std::string &volumeId);

private:
    std::string m_volumeId;
};
OPENSTACK_PLUGIN_NAMESPACE_END

#endif
