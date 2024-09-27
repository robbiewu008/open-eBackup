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
package openbackup.oceanbase.service;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-04
 */
public interface OceanBaseService {
    /**
     * 获取受保护环境
     *
     * @param agentUuid 环境id
     * @return 受保护环境
     */
    ProtectedEnvironment getEnvironmentById(String agentUuid);

    /**
     * 获取受保护资源
     *
     * @param uuid uuid
     * @return 受保护资源
     */
    Optional<ProtectedResource> getResourceById(String uuid);

    /**
     * 查询集群信息,获取版本号、租户集列表、资源池
     *
     * @param environment environment
     * @param conditions conditions
     * @return ProtectedResource
     */
    OBClusterInfo queryClusterInfo(ProtectedEnvironment environment, String conditions);

    /**
     * 查询集群信息,获取版本号、租户集列表
     *
     * @param environment environment
     * @return ProtectedResource
     */
    default OBClusterInfo queryClusterInfo(ProtectedEnvironment environment) {
        return queryClusterInfo(environment, null);
    }

    /**
     * 获取已存在的OceanBase资源信息
     *
     * @param excludeUuid 排除的uuid
     * @return 已存在的OceanBase资源信息
     */
    List<String> getExistingOceanBaseCluster(String excludeUuid);

    /**
     * 查询集群下哪些租户已经被注册过了。
     *
     * @param type 资源的subType
     * @param parentUuid 资源父资源
     * @return 集群下已注册的租户名称
     */
    List<ProtectedResource> getProtectedEnvironments(ResourceSubTypeEnum type, String parentUuid);

    /**
     * 查询集群下哪些租户已经被注册过了。
     *
     * @param clusterUuid 集群的uuid
     * @return 集群下已注册的租户名称
     */
    default List<String> getExistingOceanBaseTenant(String clusterUuid) {
        return getExistingOceanBaseTenant(clusterUuid, null);
    }

    /**
     * 查询集群下哪些租户已经被注册过了。
     *
     * @param clusterUuid 集群的uuid
     * @param excludeUuid 排除的租户集的uuid
     * @return 集群下已注册的租户名称
     */
    List<String> getExistingOceanBaseTenant(String clusterUuid, String excludeUuid);

    /**
     * 更新资源状态
     *
     * @param resourceList 资源信息
     */
    void updateExtendInfo(List<ProtectedResource> resourceList);

    /**
     * 租户集连通性和健康检查。实际就是检查租户集中的租户在集群中是否存在。
     *
     * @param resource 连通性检查环境
     * @return 不存在的租户名称
     */
    List<String> checkTenantSetConnect(ProtectedResource resource);

    /**
     * 更新资源---慎用，仅传入需要更新的值。
     *
     * @param environment 资源信息
     */
    void updateSourceDirectly(ProtectedEnvironment environment);

    /**
     * 设置受保护集群关联的 租户集及其中租户的状态为OFFLINE
     *
     * @param env 受保护环境
     */
    void setTenantSetStatue(ProtectedEnvironment env);

    /**
     * 移除资源的数据存储仓白名单
     *
     * @param resourceId resource id
     */
    void removeDataRepoWhiteListOfResource(String resourceId);

    /**
     * 持续挂载仓解挂载
     *
     * @param obClusterInfo obClusterInfo
     * @param resource resource
     */
    void umountDataRepo(OBClusterInfo obClusterInfo, ProtectedResource resource);

    /**
     * 校验是否支持NFS4.1
     */
    void checkSupportNFSV41();

    /**
     * 软硬解耦下校验是否支持NFS4.1
     *
     * @param repositories 仓库
     */
    void checkSupportNFSV41Dependent(List<StorageRepository> repositories);
}