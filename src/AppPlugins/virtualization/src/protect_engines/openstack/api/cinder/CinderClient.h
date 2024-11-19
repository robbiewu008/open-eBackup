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
#ifndef OPENSTACK_CINDER_CLIENT_H
#define OPENSTACK_CINDER_CLIENT_H
 
#include "log/Log.h"
#include "common/client/RestClient.h"
#include "common/Structs.h"
#include "protect_engines/openstack/api/cinder/model/GetProjectVolumesRequest.h"
#include "protect_engines/openstack/api/cinder/model/GetProjectVolumesResponse.h"
#include "protect_engines/openstack/api/cinder/model/CreateSnapshotRequest.h"
#include "protect_engines/openstack/api/cinder/model/CreateSnapshotResponse.h"
#include "protect_engines/openstack/api/cinder/model/GetSnapshotRequest.h"
#include "protect_engines/openstack/api/cinder/model/GetSnapshotResponse.h"
#include "protect_engines/openstack/api/cinder/model/GroupTypeRequest.h"
#include "protect_engines/openstack/api/cinder/model/GroupTypeResponse.h"
#include "protect_engines/openstack/api/cinder/model/VolumeResponse.h"
#include "protect_engines/openstack/api/cinder/model/VolumeRequest.h"
#include "protect_engines/openstack/api/cinder/model/CreateVolumeTypeRequest.h"
#include "protect_engines/openstack/api/cinder/model/CreateVolumeTypeResponse.h"
#include "protect_engines/openstack/api/cinder/model/VolumeGroupRequest.h"
#include "protect_engines/openstack/api/cinder/model/VolumeGroupResponse.h"
#include "protect_engines/openstack/api/cinder/model/GetVolumeTypesRequest.h"
#include "protect_engines/openstack/api/cinder/model/GetVolumeTypesResponse.h"
#include "protect_engines/openstack/api/cinder/model/UpdateVolumeGroupRequest.h"
#include "protect_engines/openstack/api/cinder/model/UpdateVolumeGroupResponse.h"
#include "protect_engines/openstack/api/cinder/model/CreateGroupSnapShotRequest.h"
#include "protect_engines/openstack/api/cinder/model/CreateGroupSnapShotResponse.h"
#include "protect_engines/openstack/api/cinder/model/DeleteSnapshotResponse.h"
#include "protect_engines/openstack/api/cinder/model/DeleteSnapshotRequest.h"
#include "protect_engines/openstack/api/cinder/model/DeleteVolumeGroupResponse.h"
#include "protect_engines/openstack/api/cinder/model/DeleteVolumeGroupRequest.h"
#include "protect_engines/openstack/api/cinder/model/GetSnapshotListRequest.h"
#include "protect_engines/openstack/api/cinder/model/GetSnapshotListResponse.h"
#include "protect_engines/openstack/api/cinder/model/UpdateVolumeBootableRequest.h"
#include "protect_engines/openstack/api/cinder/model/UpdateVolumeBootableResponse.h"
#include "protect_engines/openstack/api/cinder/model/ActiveSnapConsistencyResponse.h"
#include "protect_engines/openstack/api/cinder/model/ActiveSnapConsistencyRequest.h"
#include "protect_engines/openstack/utils/OpenStackTokenMgr.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"

using VirtPlugin::RestClient;
using VirtPlugin::RequestInfo;
using VirtPlugin::SUCCESS;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class CinderClient : public RestClient {
public:
    CinderClient() {};
    ~CinderClient() {};
    bool CheckParams(ModelBase &model) override;
    std::shared_ptr<GetProjectVolumesResponse> GetProjectVolumes(GetProjectVolumesRequest &request);
    virtual std::shared_ptr<GetSnapshotResponse> GetSnapshot(GetSnapshotRequest &request);
    std::shared_ptr<GroupTypeResponse> CreateGroupType(GroupTypeRequest &request);
    std::shared_ptr<CreateSnapshotResponse> CreateSnapshot(CreateSnapshotRequest &request);
    std::shared_ptr<VolumeResponse> GetVolumeDetail(VolumeRequest &request);
    std::shared_ptr<VolumeResponse> CreateVolume(VolumeRequest &request);
    std::shared_ptr<VolumeResponse> DeleteVolume(VolumeRequest &request);
    std::shared_ptr<CreateVolumeTypeResponse> CreateVolumeType(CreateVolumeTypeRequest &request);
    std::shared_ptr<VolumeGroupResponse> CreateVolumeGroup(VolumeGroupRequest &request);
    std::shared_ptr<UpdateVolumeGroupResponse> UpdateVolumeGroup(UpdateVolumeGroupRequest &request);
    std::shared_ptr<CreateGroupSnapShotResponse> CreateGroupSnapShot(CreateGroupSnapShotRequest &request);
    std::shared_ptr<DeleteSnapshotResponse> DeleteSnapshot(DeleteSnapshotRequest &request);
    std::shared_ptr<DeleteVolumeGroupResponse> DeleteGroup(DeleteVolumeGroupRequest &request);
    std::shared_ptr<GetSnapshotResponse> GetGroupSnapshot(GetSnapshotRequest &request);
    std::shared_ptr<GetSnapshotListResponse> GetSnapshotList(GetSnapshotListRequest &request);
    std::shared_ptr<GroupTypeResponse> GetGroupType(GroupTypeRequest &request);
    std::shared_ptr<GroupTypeResponse> DeleteGroupType(GroupTypeRequest &request);
    std::shared_ptr<DeleteSnapshotResponse> DeleteGroupSnapShot(DeleteSnapshotRequest &request);
    std::shared_ptr<VolumeGroupResponse> GetVolumeGroupStatus(VolumeGroupRequest &request);
    std::shared_ptr<GetVolumeTypesResponse> GetVolumeTypes(GetVolumeTypesRequest &request);
    std::shared_ptr<UpdateVolumeBootableResponse> UpdateVolumeBootable(UpdateVolumeBootableRequest &request);
    std::shared_ptr<ActiveSnapConsistencyResponse> ActiveSnapConsistency(ActiveSnapConsistencyRequest &request);

protected:
    bool UpdateToken(ModelBase &model, std::string &tokenStr) override;
    virtual bool GetTokenEndPoint(ModelBase &request, std::string &tokenStr, std::string &endpoint);
    virtual void FormRequestInfo(ModelBase &srcRequest, ModelBase &dstRequest);

private:
    bool ConfirmSnapshotReady(GetSnapshotRequest &req, std::shared_ptr<CreateSnapshotResponse> response);
    int32_t SendRequest(RequestInfo &requestInfo, std::shared_ptr<ResponseModel> response, ModelBase &model);
};

OPENSTACK_PLUGIN_NAMESPACE_END
 
#endif