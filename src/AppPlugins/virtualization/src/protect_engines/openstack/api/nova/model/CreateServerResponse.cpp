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
#include "CreateServerResponse.h"
#include <common/JsonHelper.h>
#include "log/Log.h"

using namespace VirtPlugin;

namespace {
const std::string MODULE_NAME = "CreateServerResponse";
}
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
CreateServerResponse::CreateServerResponse() {}
 
CreateServerResponse::~CreateServerResponse() {}
 
bool CreateServerResponse::Serial()
{
    if (m_body.empty()) {
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_body, m_serverDetail)) {
        ERRLOG("Failed to trans data from json string to struct.");
        return false;
    }
    return true;
}

ServerDetail CreateServerResponse::GetServerDetail() const
{
    return m_serverDetail;
}
 
OPENSTACK_PLUGIN_NAMESPACE_END
