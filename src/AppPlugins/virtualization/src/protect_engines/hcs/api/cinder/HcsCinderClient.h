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
#ifndef HCS_CINDER_CLIENT_H
#define HCS_CINDER_CLIENT_H

#include "log/Log.h"
#include "protect_engines/hcs/utils/HCSTokenMgr.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "protect_engines/hcs/common/HcsConstants.h"
#include "protect_engines/hcs/api/cinder/model/ActiveSnapConsistencyRequest.h"
#include "protect_engines/hcs/api/cinder/model/ActiveSnapConsistencyResponse.h"
#include "protect_engines/hcs/api/cinder/model/ShowSnapshotListRequest.h"
#include "protect_engines/hcs/api/cinder/model/ShowSnapshotListResponse.h"
#include "protect_engines/openstack/api/cinder/CinderClient.h"
#include "protect_engines/openstack/api/cinder/model/GetSnapshotRequest.h"
#include "protect_engines/openstack/api/cinder/model/GetSnapshotResponse.h"

using namespace VirtPlugin;
using OpenStackPlugin::CinderClient;
using OpenStackPlugin::GetSnapshotResponse;
using OpenStackPlugin::GetSnapshotRequest;

class SnapshotProviderAuth {
public:
    std::string m_lunID;    // 快照 lunID
    std::string m_sn;       // 快照 sn

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lunID, lun_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sn, sn)
    END_SERIAL_MEMEBER
};

namespace HcsPlugin {
class HcsCinderClient : public CinderClient {
public:
    HcsCinderClient() {};
    virtual ~HcsCinderClient() {};

    bool CheckParams(ModelBase& model) override;
    std::shared_ptr<ActiveSnapConsistencyResponse> ActiveSnapConsistency(ActiveSnapConsistencyRequest &request);
    std::shared_ptr<GetSnapshotResponse> GetSnapshot(GetSnapshotRequest &request) override;
    std::shared_ptr<ShowSnapshotListResponse> ShowSnapshotList(ShowSnapshotListRequest &request);

protected:
    bool GetTokenEndPoint(ModelBase &request, std::string &tokenStr, std::string &endpoint) override;
    void FormRequestInfo(ModelBase &srcRequest, ModelBase &dstRequest) override;
};
}
#endif