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
#ifndef KEYSTONE_GET_PROJECTS_RESPONSE_H
#define KEYSTONE_GET_PROJECTS_RESPONSE_H

#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "protect_engines/openstack/common/Structs.h"
using VirtPlugin::ResponseModel;


OPENSTACK_PLUGIN_NAMESPACE_BEGIN
class GetDomainProjectsResponse : public ResponseModel {
public:
    GetDomainProjectsResponse();
    ~GetDomainProjectsResponse();

    bool Serial();
    ProjectList GetProjectList();

protected:
    ProjectList m_projects;
};
OPENSTACK_PLUGIN_NAMESPACE_END

#endif