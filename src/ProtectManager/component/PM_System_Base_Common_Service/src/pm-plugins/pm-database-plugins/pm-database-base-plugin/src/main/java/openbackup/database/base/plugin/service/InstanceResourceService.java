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
package openbackup.database.base.plugin.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.enums.ClusterInstanceOnlinePolicy;

/**
 * 数据库的实例资源服务
 *
 */
public interface InstanceResourceService {
    /**
     * 校验资源是否属于集群实例
     *
     * @param resource 集群实例资源信息
     * @return 校验结果
     */
    AgentBaseDto checkIsClusterInstance(ProtectedResource resource);

    /**
     * 校验资源是否属于集群实例
     *
     * @param resource 集群实例资源信息
     * @return 校验结果
     */
    AgentBaseDto checkClusterInstance(ProtectedResource resource);

    /**
     * 设置集群实例每个节点角色
     *
     * @param resource 集群实例资源信息
     */
    void setClusterInstanceNodeRole(ProtectedResource resource);

    /**
     * 检查集群实例是否已经被注册
     *
     * @param resource 集群实例资源信息
     */
    void checkClusterInstanceIsRegistered(ProtectedResource resource);

    /**
     * 检查集群实例的端口是否被修改
     *
     * @param resource 集群实例资源信息
     */
    void checkClusterInstancePortIsChanged(ProtectedResource resource);

    /**
     * 检查单实例是否已经被注册
     *
     * @param resource 单实例资源信息
     */
    void checkSignalInstanceIsRegistered(ProtectedResource resource);

    /**
     * 检查单实例的端口是否被修改
     *
     * @param resource 单实例资源信息
     */
    void checkSignalInstancePortIsChanged(ProtectedResource resource);

    /**
     * 单实例资源健康检查
     *
     * @param resource 单实例资源信息
     */
    void healthCheckSingleInstance(ProtectedResource resource);

    /**
     * 环境下的集群实例资源健康检查
     *
     * @param environment 环境信息
     * @param policy 集群实例在线的策略
     */
    void healthCheckClusterInstanceOfEnvironment(ProtectedEnvironment environment, ClusterInstanceOnlinePolicy policy);

    /**
     * 根据资源UUID获取资源信息
     *
     * @param resourceId 资源的UUID
     * @return ProtectedResource 资源信息
     */
    ProtectedResource getResourceById(String resourceId);
}
