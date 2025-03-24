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
package openbackup.system.base.service.hostagent;

import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.service.hostagent.model.AgentInfo;

import java.util.List;
import java.util.Map;

/**
 * 客户端内部查询服务
 *
 */
public interface AgentQueryService {
    /**
     * 查询客户端接口
     *
     * @param pageSize pageSize
     * @param pageNo pageNo
     * @param conditions conditions
     * @return 分页的客户端信息 PageListResponse<AgentInfo>
     */
    PageListResponse<AgentInfo> queryAgents(int pageSize, int pageNo, Map<String, Object> conditions);

    /**
     * 查询共享客户端id
     *
     * @return 共享客户端id List<AgentInfo>
     */
    List<String> querySharedAgentIds();

    /**
     * 查询指定插件类型的共享agent
     *
     * @param pluginType 插件类型
     * @return 分页的客户端信息 PageListResponse<AgentInfo>
     */
    PageListResponse<AgentInfo> querySharedAgents(String pluginType);

    /**
     * 查询内置agent的uuid列表
     *
     * @return 内置agent的uuid列表
     */
    List<String> queryInternalAgentIds();

    /**
     * 查询agent插件列表
     *
     * @param agentId 要查询的agentid
     * @return 内置agent插件的uuid列表
     */
    List<String> queryAgentPlugInIds(String agentId);
}
