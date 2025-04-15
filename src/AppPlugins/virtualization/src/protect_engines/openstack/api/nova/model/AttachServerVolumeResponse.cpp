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
#include "AttachServerVolumeResponse.h"
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
AttachServerVolumeResponse::AttachServerVolumeResponse() {}
AttachServerVolumeResponse::~AttachServerVolumeResponse() {}
 
bool AttachServerVolumeResponse::Serial()
{
    if (m_body.empty()) {
        ERRLOG("AttachServerVolumeResponse body is empty.");
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_body, m_volume)) {
        ERRLOG("Failed to trans data from json string to server list.");
        return false;
    }
    return true;
}
 
AttachVolumeDetail AttachServerVolumeResponse::GetServerAttachVolume()
{
    return m_volume;
}
 
OPENSTACK_PLUGIN_NAMESPACE_END