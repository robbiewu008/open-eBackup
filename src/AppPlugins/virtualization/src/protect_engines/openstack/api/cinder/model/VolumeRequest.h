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
#ifndef OPENSTACK_VOLUME_REQUEST_H
#define OPENSTACK_VOLUME_REQUEST_H

#include <string>
#include "common/model/ModelBase.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "common/token_mgr/TokenDetail.h"
#include "VolumeDetail.h"

using VirtPlugin::ApiType;
using VirtPlugin::Scope;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class VolumeRequest : public VirtPlugin::ModelBase {
public:
    VolumeRequest();
    ~VolumeRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;
    // add use
    void SetNewVolumeInfo(const VolumeRequestBody& confInfo);
    bool BuildCreateBody(std::string& reqBody);
    // delete, no need function call
    // update, no needs
    // query use
    std::string GetVolumeId() const;
    void SetVolumeId(const std::string& volumeId);
    void SetBody(std::string body);
    std::string GetBody();

protected:
    std::string m_projectId;
    VolumeRequestBody m_newVolumeConfInfo;
    std::string m_volumeId;
    std::string m_body;
};

OPENSTACK_PLUGIN_NAMESPACE_END

#endif