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
#ifndef GET_FLAVORS_DETAIL_REQUEST_REQUEST_H
#define GET_FLAVORS_DETAIL_REQUEST_REQUEST_H
 
#include <string>
#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"

using VirtPlugin::ModelBase;
using VirtPlugin::ApiType;
using VirtPlugin::Scope;
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
class GetFlavorsDetailRequest : public ModelBase {
public:
    GetFlavorsDetailRequest();
    ~GetFlavorsDetailRequest();
 
    virtual Scope GetScopeType() const override;
    ApiType GetApiType() override;
};
 
OPENSTACK_PLUGIN_NAMESPACE_END
 
#endif