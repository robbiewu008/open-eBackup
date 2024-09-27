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
#ifndef HCS_GET_SERVER_LIST_REQUEST_H
#define HCS_GET_SERVER_LIST_REQUEST_H

#include <string>
#include "common/model/ModelBase.h"
#include "protect_engines/hcs/common/HcsCommonInfo.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "common/token_mgr/TokenDetail.h"

using namespace VirtPlugin;

namespace HcsPlugin {
class GetServerListRequest : public ModelBase {
public:
    GetServerListRequest();
    ~GetServerListRequest();

    virtual Scope GetScopeType() const override;
    virtual ApiType GetApiType() override;

    /// <summary>
    /// 云服务器状态。
    /// </summary>

    std::string GetServerStatus() const;
    bool ServerStatusIsSet() const;
    void UnsetServerStatus();
    void SetServerStatus(const std::string& value);

    /// <summary>
    /// 云服务器IPv4地址。
    /// </summary>

    std::string GetServerIp() const;
    bool ServerIpIsSet() const;
    void UnsetServerIp();
    void SetServerIp(const std::string& value);

    /// <summary>
    /// 云服务器名称。
    /// </summary>

    std::string GetServerName() const;
    bool ServerNameIsSet() const;
    void UnsetServerName();
    void SetServerName(const std::string& value);

    /// <summary>
    /// 页码，默认为1。
    /// </summary>

    int32_t GetServerOffset() const;
    bool ServerOffsetIsSet() const;
    void UnsetServerOffset();
    void SetServerOffset(const int32_t& value);

    /// <summary>
    /// Marker，从marker指定的云服务器ID的下一条数据开始查询。
    /// </summary>

    std::string GetServerMarker() const;

    bool ServerMarkerIsSet() const;

    void UnsetServerMarker();

    void SetServerMarker(const std::string &value);

    /// <summary>
    /// 查询返回云服务器当前页面的大小。
    /// </summary>

    int32_t GetServerLimit() const;
    bool ServerLimitIsSet() const;
    void UnsetServerLimit();
    void SetServerLimit(const int32_t& value);

protected:
    std::string m_status;
    bool m_statusIsSet;
    std::string m_ip;
    bool m_ipIsSet;
    std::string m_name;
    bool m_nameIsSet;
    int32_t m_offset;
    bool m_offstIsSet;
    int32_t m_limit;
    bool m_limitIsSet;
    std::string m_marker = "";
    bool m_markerIsSet = false;
};
}

#endif