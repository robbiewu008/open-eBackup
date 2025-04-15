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
package openbackup.db2.protection.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;
import java.util.Optional;

/**
 * db2实例服务
 *
 */
public interface Db2InstanceService {
    /**
     * 检查单实例是否已经被注册
     *
     * @param resource 单实例资源信息
     */
    void checkSingleInstanceIsRegistered(ProtectedResource resource);

    /**
     * 检查单实例的名称是否被修改
     *
     * @param resource 单实例资源信息
     */
    void checkSingleInstanceNameIsChanged(ProtectedResource resource);

    /**
     * 校验资源是否属于集群实例
     *
     * @param resource 集群实例资源信息
     * @return 校验结果
     */
    AgentBaseDto checkIsClusterInstance(ProtectedResource resource);

    /**
     * 检查集群实例是否已经被注册
     *
     * @param resource 集群实例资源信息
     */
    void checkClusterInstanceIsRegistered(ProtectedResource resource);

    /**
     * 检查集群实例的名称是否被修改
     *
     * @param resource 集群实例资源信息
     */
    void checkClusterInstanceNameIsChanged(ProtectedResource resource);


    /**
     * 过滤集群实例信息
     *
     * @param clusterInstance 集群实例资源信息
     */
    void filterClusterInstance(ProtectedResource clusterInstance);

    /**
     * 校验hadr集群实例
     *
     * @param resource 集群实例资源信息
     */
    void checkHadrClusterInstance(ProtectedResource resource);

    /**
     * 扫描数据库
     *
     * @param clusterInstance 集群实例资源信息
     * @param environment 集群实例资源信息
     * @return 扫描结果
     */
    List<ProtectedResource> scanDatabase(ProtectedResource clusterInstance, ProtectedEnvironment environment);

    /**
     * 查询HA集群的主节点
     *
     * @param clusterInstance 集群实例
     * @return 集群实例的主节点
     */
    ProtectedResource queryHadrPrimaryNode(ProtectedResource clusterInstance);

    /**
     * 查询主节点
     *
     * @param subInstance 子实例
     * @param subEnv 环境
     * @return 主节点
     */
    Optional<NodeInfo> queryMasterNode(ProtectedResource subInstance, ProtectedEnvironment subEnv);
}
