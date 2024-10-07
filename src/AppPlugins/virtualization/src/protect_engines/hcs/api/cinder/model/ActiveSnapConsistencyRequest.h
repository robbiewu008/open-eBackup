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
#ifndef ACTIVE_SNAPSHOT_CONSISTENCY_REQUEST_H
#define ACTIVE_SNAPSHOT_CONSISTENCY_REQUEST_H

#include <string>
#include "common/model/ModelBase.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "common/token_mgr/TokenDetail.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class ActiveSnapConsistencyRequest : public ModelBase {
public:
    ActiveSnapConsistencyRequest();
    virtual ~ActiveSnapConsistencyRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    void SetVolUUID(const std::string &uuid);
    std::string GetVolUUID();
    void SetSnapProviderLocationList(const std::vector<std::string> &snapProvicerLocation);
    std::vector<std::string> GetSnapProviderLocationList();

protected:
    std::string m_volUUID;
    std::vector<std::string> m_snapProviderLocationList;
};
}

#endif