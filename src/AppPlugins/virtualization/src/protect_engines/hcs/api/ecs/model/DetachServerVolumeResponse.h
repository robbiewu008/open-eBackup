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
#ifndef HCS_DETACH_SERVER_VOLUME_RESPONSE_H
#define HCS_DETACH_SERVER_VOLUME_RESPONSE_H

#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "protect_engines/hcs/common/HcsMacros.h"

using namespace VirtPlugin;

HCS_PLUGIN_NAMESPACE_BEGIN

class DetachServerVolumeResponse : public ResponseModel {
public:
    DetachServerVolumeResponse() {};
    ~DetachServerVolumeResponse() {};
};

HCS_PLUGIN_NAMESPACE_END

#endif