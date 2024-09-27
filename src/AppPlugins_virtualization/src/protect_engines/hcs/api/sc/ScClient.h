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
#ifndef HCS_SC_CLIENT_H
#define HCS_SC_CLIENT_H

#include "common/client/RestClient.h"
#include "protect_engines/hcs/api/sc/model/QueryVdcListRequest.h"
#include "protect_engines/hcs/api/sc/model/QueryVdcListResponse.h"
#include "protect_engines/hcs/api/sc/model/QueryResourceListRequest.h"
#include "protect_engines/hcs/api/sc/model/QueryResourceListResponse.h"
#include "protect_engines/hcs/api/sc/model/QueryProjectDetailRequest.h"
#include "protect_engines/hcs/api/sc/model/QueryProjectDetailResponse.h"
#include "protect_engines/hcs/api/sc/model/GetVdcUserDetailRequest.h"
#include "protect_engines/hcs/api/sc/model/GetVdcUserDetailResponse.h"

namespace HcsPlugin {
class ScClient : public RestClient {
public:
    ScClient() {}
    ~ScClient() {}

    bool CheckParams(ModelBase &model) override;

    // 查询VDC列表，支持按照region查询，可以查询VDC下级列表，支持分页和排序。
    std::shared_ptr<QueryVdcListResponse> QueryVdcList(QueryVdcListRequest &request);

    // 查询指定VDC中的资源集集合
    std::shared_ptr<QueryResourceListResponse> QueryResourceList(QueryResourceListRequest &request);

    // 该接口提供根据Project ID获取Project的详细信息。
    std::shared_ptr<QueryProjectDetailResponse> QueryProjectDetail(QueryProjectDetailRequest &request);

    std::shared_ptr<GetVdcUserDetailResponse> GetVDCUserDetailInfo(GetVdcUserDetailRequest &request);
};
}

#endif