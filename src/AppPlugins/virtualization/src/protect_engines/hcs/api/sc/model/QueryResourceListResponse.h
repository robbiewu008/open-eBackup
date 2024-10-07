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
#ifndef HCS_QUERY_RESOURCE_LIST_RESPONSE_H
#define HCS_QUERY_RESOURCE_LIST_RESPONSE_H

#include "common/model/ResponseModel.h"
#include "ResourceListDetail.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class QueryResourceListResponse : public ResponseModel {
public:
    QueryResourceListResponse() {}
    ~QueryResourceListResponse() {}

    bool Serial();
    ResourceListDetail GetResourceListDetail() const;
protected:
    ResourceListDetail m_resourceListDetail;
};
}

#endif