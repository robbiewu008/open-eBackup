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
#ifndef OPENSTACK_GET_FLAVORS_DETAIL_RESPONSE_H
#define OPENSTACK_GET_FLAVORS_DETAIL_RESPONSE_H
 
#include <vector>
#include "common/JsonHelper.h"
#include "ServerDetail.h"
#include "common/model/ResponseModel.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "ServerDetail.h"
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
class GetFlavorsDetailResponse : public VirtPlugin::ResponseModel {
public:
    GetFlavorsDetailResponse();
    
    ~GetFlavorsDetailResponse();
    bool Serial();
    ServerFlavorList GetFlavorsDetail() const;
 
protected:
    ServerFlavorList m_flavors;
};
OPENSTACK_PLUGIN_NAMESPACE_END
 
#endif