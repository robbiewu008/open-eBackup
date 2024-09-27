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

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;

/**
 * 数据库集群环境服务
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-26
 */
public interface ClusterEnvironmentService {
    /**
     * 校验集群下的节点个数
     *
     * @param agents 节点信息
     */
    void checkClusterNodeNum(List<ProtectedResource> agents);

    /**
     * 校验集群下的节点状态
     *
     * @param environments 节点信息
     */
    void checkClusterNodeStatus(List<ProtectedEnvironment> environments);

    /**
     * 校验集群下的节点操作系统类型
     *
     * @param environments 节点信息
     */
    void checkClusterNodeOsType(List<ProtectedEnvironment> environments);

    /**
     * 校验注册集群的节点是否被注册
     *
     * @param resource 资源信息
     */
    void checkRegisterNodeIsRegistered(ProtectedResource resource);

    /**
     * 校验更新集群的节点是否被注册
     *
     * @param resource 资源信息
     */
    void checkUpdateNodeIsRegistered(ProtectedResource resource);

    /**
     * 校验集群是否注册实例资源
     *
     * @param environment 环境信息
     */
    void checkClusterIsRegisteredInstance(ProtectedEnvironment environment);

    /**
     * 校验集群下的节点数规格
     *
     * @param agentsNum 节点个数
     * @param clusterMaxNodeCount 规格数
     */
    void checkClusterNodeCountLimit(int agentsNum, int clusterMaxNodeCount);
}
