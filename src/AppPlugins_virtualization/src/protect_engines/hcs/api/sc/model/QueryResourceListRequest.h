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
#ifndef HCS_GET_RESOURCE_REQUEST_H
#define HCS_GET_RESOURCE_REQUEST_H

#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class QueryResourceListRequest : public ModelBase {
public:
    QueryResourceListRequest();
    ~QueryResourceListRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    int32_t GetStart() const;
    void SetStart(int32_t offset);
    bool GetStartIsSet() const;
    int32_t GetLimit() const;
    void SetLimit(int32_t limit);
    bool GetLimitIsSet() const;

    // vdc 管理员id
    std::string GetVdcManagerId() const;
    bool VdcManagerIdIsSet() const;
    void SetVdcManagerId(const std::string &value);

    std::string GetQueryAllProjectStr() const;
    void SetQueryAllProject(const bool value);
    bool GetQueryAllProjectIsSet() const;
protected:
    std::string m_vdcManagerId;
    bool m_vdcManagerIdIsSet;

    int32_t m_offset = 0;
    bool m_offstIsSet = false;
    int32_t m_limit = 100;
    bool m_limitIsSet = false;

    bool m_allProjectQueryIsSet = false;
    std::string m_allProjectQuery;
};
}

#endif