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
#ifndef HCS_EVS_CLIENT_H
#define HCS_EVS_CLIENT_H

#include "log/Log.h"
#include "common/client/RestClient.h"
#include "protect_engines/hcs/utils/HCSTokenMgr.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "protect_engines/hcs/api/evs/model/ShowVolumeRequest.h"
#include "protect_engines/hcs/api/evs/model/ShowVolumeResponse.h"
#include "protect_engines/hcs/api/evs/model/VolumeDetail.h"
#include "protect_engines/hcs/api/ecs/model/GetServerDetailsRequest.h"
#include "protect_engines/hcs/api/ecs/model/GetServerDetailsResponse.h"
#include "protect_engines/hcs/api/ecs/model/GetServerListRequest.h"
#include "protect_engines/hcs/api/ecs/model/GetServerListResponse.h"

namespace HcsPlugin {
class EvsClient : RestClient {
public:
    EvsClient() {};
    virtual ~EvsClient() {};

    bool CheckParams(ModelBase& model) override;

    std::shared_ptr<ShowVolumeResponse> ShowVolumeDetail(ShowVolumeRequest &request);
    std::shared_ptr<ShowVolumeDetailResponse> ShowVolumeDetailList(ShowVolumeListRequest &request);

private:
    bool GetServerDetailsCheckParams(ModelBase &model);
};
}
#endif