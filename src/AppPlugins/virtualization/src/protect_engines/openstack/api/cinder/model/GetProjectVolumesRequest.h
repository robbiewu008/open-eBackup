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
#ifndef GET_PROJECT_VOLUMES_REQUEST_H
#define GET_PROJECT_VOLUMES_REQUEST_H
 
#include <string>
#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"

using VirtPlugin::ModelBase;
using VirtPlugin::ApiType;
using VirtPlugin::Scope;
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
class GetProjectVolumesRequest : public ModelBase {
public:
    GetProjectVolumesRequest();
    ~GetProjectVolumesRequest();
 
    virtual Scope GetScopeType() const override;
    ApiType GetApiType() override;

    int32_t GetVolumeOffSet() const;
    void SetVolumeOffset(int32_t offset);
    bool GetVolumeOffsetIsSet() const;

    int32_t GetVolumeLimit() const;
    void SetVolumeLimit(int32_t limit);
    bool GetVolumeLimitIsSet() const;
    void SetSnapShotId(const std::string &snapshotId);
    std::string GetSnapShotId() const;
    void SetVolumeStatus(const std::string &status);
    std::string GetVolumeStatus() const;
    
protected:
    int32_t m_offset;
    bool m_offstIsSet = false;
    int32_t m_limit;
    bool m_limitIsSet = false;
    std::string m_status = "";
    bool m_statusIsSet = false;
    std::string m_snapshotId = "";
    std::string m_volumeStatus = "";
};
 
OPENSTACK_PLUGIN_NAMESPACE_END
 
#endif