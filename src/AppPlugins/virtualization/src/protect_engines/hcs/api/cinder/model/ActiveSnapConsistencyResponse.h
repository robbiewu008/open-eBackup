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
#ifndef ACTIVE_SNAPSHOT_CONSISTENCY_RESPONSE_H
#define ACTIVE_SNAPSHOT_CONSISTENCY_RESPONSE_H

#include <string>
#include "common/model/ModelBase.h"
#include "protect_engines/hcs/common/HcsMacros.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class ActiveSnapConsistencyResponse : public ResponseModel {
public:
    ActiveSnapConsistencyResponse() {}
    virtual ~ActiveSnapConsistencyResponse() {}
};
}

#endif