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
#ifndef HCS_QUERY_VDC_LIST_REQUEST_H
#define HCS_QUERY_VDC_LIST_REQUEST_H

#include "common/model/ModelBase.h"
#include "common/token_mgr/TokenDetail.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class QueryVdcListRequest : public ModelBase {
public:
    QueryVdcListRequest();
    ~QueryVdcListRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    int32_t GetStart() const;
    bool StartIsSet() const;
    void SetStart(const int32_t &value);

    int32_t GetLimit() const;
    bool LimitIsSet() const;
    void SetLimit(const int32_t &value);

    std::string GetUpperVdcId() const;
    bool UpperVdcIdIsSet() const;
    void SetUpperVdcId(const std::string &value);

    // 是否查询租户
    std::string GetIsDomain() const;
    bool IsDomainIsSet() const;
    void SetIsDomain(const std::string &value);

    // 管理员等级
    int32_t GetLevel() const;
    bool LevelIsSet() const;
    void SetLeveL(const int32_t &value);

    // 过滤所属的域
    std::string GetDomainId() const;
    bool DomainIdIsSet() const;
    void SetDomainId(const std::string &value);

protected:
    int32_t m_start;
    bool m_startIsSet = false;
    int32_t m_limit;
    bool m_limitIsSet = false;
    std::string m_upperVdcId;
    bool m_upperVdcIdIsSet = false;
    std::string m_IsDomain;
    bool m_isDomainIsSet = false;
    int32_t m_level = 0;
    bool m_levelIsSet = false;
    std::string m_domainId; // 绑定对应租户id
    bool m_domainIdIsSet = false;
};
}

#endif