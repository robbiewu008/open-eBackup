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
#ifndef HCS_ECS_CLIENT_H
#define HCS_ECS_CLIENT_H

#include "log/Log.h"
#include "common/client/RestClient.h"
#include "protect_engines/hcs/api/ecs/model/LockServerRequest.h"
#include "protect_engines/hcs/api/ecs/model/LockServerResponse.h"
#include "protect_engines/hcs/api/ecs/model/UnLockServerRequest.h"
#include "protect_engines/hcs/api/ecs/model/UnLockServerResponse.h"
#include "protect_engines/hcs/api/ecs/model/GetServerDetailsRequest.h"
#include "protect_engines/hcs/api/ecs/model/GetServerDetailsResponse.h"
#include "protect_engines/hcs/api/ecs/model/GetServerListRequest.h"
#include "protect_engines/hcs/api/ecs/model/GetServerListResponse.h"
#include "protect_engines/hcs/api/ecs/model/PowerOffServerRequest.h"
#include "protect_engines/hcs/api/ecs/model/PowerOffServerResponse.h"
#include "protect_engines/hcs/api/ecs/model/PowerOnServerRequest.h"
#include "protect_engines/hcs/api/ecs/model/PowerOnServerResponse.h"
#include "protect_engines/hcs/api/ecs/model/AttachServerVolumeRequest.h"
#include "protect_engines/hcs/api/ecs/model/AttachServerVolumeResponse.h"
#include "protect_engines/hcs/api/ecs/model/DetachServerVolumeRequest.h"
#include "protect_engines/hcs/api/ecs/model/DetachServerVolumeResponse.h"
#include "protect_engines/hcs/utils/HCSTokenMgr.h"
#include "protect_engines/hcs/utils/HcsOpServiceUtils.h"
#include "protect_engines/hcs/common/HcsMacros.h"

namespace HcsPlugin {
class EcsClient : RestClient {
public:
    EcsClient() {};
    ~EcsClient() {};
    bool CheckParams(ModelBase &model) override;
    std::shared_ptr<LockServerResponse> LockServer(LockServerRequest &request);
    std::shared_ptr<UnLockServerResponse> UnLockServer(UnLockServerRequest &request);
    std::shared_ptr<GetServerListResponse> GetServerList(GetServerListRequest &request);
    std::shared_ptr<GetServerListResponse> GetServerDetailList(GetServerListRequest &request);
    std::shared_ptr<GetServerDetailsResponse> GetServerDetails(GetServerDetailsRequest &request);
    std::shared_ptr<PowerOffServerResponse> PowerOffServer(PowerOffServerRequest &request);
    std::shared_ptr<PowerOnServerResponse> PowerOnServer(PowerOnServerRequest &request);
    std::shared_ptr<DetachServerVolumeResponse> DetachServerVolume(DetachServerVolumeRequest &request);
    std::shared_ptr<AttachServerVolumeResponse> AttachServerVolume(AttachServerVolumeRequest &request);
};
}

#endif