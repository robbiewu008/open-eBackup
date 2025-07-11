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
package openbackup.system.base.service;

import openbackup.system.base.common.model.host.AgentManagementDomain;

import java.net.Proxy;
import java.net.URI;
import java.util.List;
import java.util.Optional;

/**
 * 获取agent的nginx/pod代理信息
 *
 */
public interface AvailableAgentManagementDomainService {
    /**
     * 获取可用的pod代理信息
     *
     * @param uuid agent所在主机的id
     * @return ManagementInfo 可用的代理信息
     */
    AgentManagementDomain getAvailableManagementInfo(String uuid);

    /**
     * 获取可用的pod代理信息
     *
     * @param uuid agent所在主机的id
     * @return List<AgentManagementDomain> 所有的可用的代理信息
     */
    List<AgentManagementDomain> getAllAvailableManagementInfo(String uuid);

    /**
     * 根据endpoint获取Proxy
     *
     * @param endpoint endpoint agent所在机器的ip
     * @return dme代理对象
     */
    Optional<Proxy> getDmeProxy(String endpoint);

    /**
     * 通过agent id集合取一个能通的pod，并返回其url
     * 规则：选择出现频率最高的pod，如果有多个出现频率一样的则随机选一个
     *
     * @param uuids agent id集合
     * @return 一个能通的pod，返回其url
     */
    URI getUrlByAgents(List<String> uuids);

    /**
     * 通过agent id集合取一个能通的pod，并返回其url
     * 规则：选择出现频率最高的pod，如果有多个出现频率一样的则随机选一个
     *
     * @param agentIds agent id集合
     * @return 一个能通的服务信息
     */
    AgentManagementDomain getAvailableServiceInfoByAgents(List<String> agentIds);

    /**
     * 获取可用的pod代理信息
     *
     * @param endpoint agent所在主机的ip
     * @return ManagementInfo 可用的代理信息
     */
    AgentManagementDomain getAvailableManagementInfoByEndpoint(String endpoint);

    /**
     * 获取所有节点
     *
     * @return List<AgentManagementDomain> 所有代理信息
     */
    List<AgentManagementDomain> getAllManagementInfo();
}
