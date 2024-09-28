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
package openbackup.tidb.resources.access.service;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;
import java.util.Map;

/**
 * tidb service
 *
 */
public interface TidbService {
    /**
     * 更新资源的状态
     *
     * @param resourceId resourceId
     * @param status status
     */
    void updateResourceLinkStatus(String resourceId, String status);

    /**
     * 批量更新资源状态
     *
     * @param resourceList resourceList
     * @param status status
     */
    void updateResourceLinkStatus(List<ProtectedResource> resourceList, String status);

    /**
     * 查询集群实例信息
     *
     * @param environmentConditions environmentConditions
     * @param endpointResource endpointResource
     * @param isCluster isCluster
     * @return PageListResponse
     */
    PageListResponse<ProtectedResource> getBrowseResult(BrowseEnvironmentResourceConditions environmentConditions,
        ProtectedResource endpointResource, boolean isCluster);

    /**
     * 设置表空间被锁定的状态
     *
     * @param databaseId 数据库id
     * @param tablespaceList 表信息
     */
    void setTableLockedStatus(String databaseId, PageListResponse<ProtectedResource> tablespaceList);

    /**
     * 检查集群信息
     *
     * @param environment environment
     * @param type type
     * @param endpoint endpoint
     */
    void checkClusterInfo(ProtectedEnvironment environment, String type, Endpoint endpoint);

    /**
     * 获取受保护资源
     *
     * @param uuid 资源唯一id
     * @return 受保护资源
     */
    ProtectedResource getResourceByCondition(String uuid);

    /**
     * 条件查询资源
     *
     * @param conditions conditions
     * @return PageListResponse
     */
    PageListResponse<ProtectedResource> getResourceByCondition(Map<String, Object> conditions);

    /**
     * 获取集群信息
     *
     * @param environmentConditions environmentConditions
     * @return 集群信息
     */
    ProtectedResource getEndpointResource(BrowseEnvironmentResourceConditions environmentConditions);

    /**
     * 健康检查
     *
     * @param clusterResource clusterResource
     * @param agentResource agentResource
     * @param resourceSubType resourceSubType
     * @param actionType actionType
     */
    void checkHealth(ProtectedResource clusterResource, ProtectedResource agentResource, String resourceSubType,
        String actionType);

    /**
     * 获取资源
     *
     * @param resourceSubType 资源子类型
     * @return 资源list
     */
    List<ProtectedResource> getClusterList(String resourceSubType);

    /**
     * 检查资源重复
     *
     * @param resource resource
     * @param resourceName resourceName
     */
    void checkDuplicateResource(ProtectedResource resource, String resourceName);

    /**
     * 表集注册 检查
     *
     * @param tablespaceList tablespaceList
     * @param databaseId databaseId
     */
    void checkDuplicateResource(List<String> tablespaceList, String databaseId);

    /**
     * 检查资源是否在线
     *
     * @param protectedResource protectedResource
     */
    void checkResourceStatus(ProtectedResource protectedResource);

    /**
     * 设置supply Agent
     *
     * @param dependencies dependencies
     * @return List Endpoint
     */
    List<Endpoint> getSupplyAgent(Map<String, List<ProtectedResource>> dependencies);

    /**
     * 集群资源
     *
     * @param clusterResource clusterResource
     * @return List Endpoint
     */
    List<Endpoint> getTaskEndpoint(ProtectedResource clusterResource);

    /**
     * 根据集群信息获取agent 信息
     *
     * @param clusterResource clusterResource
     * @return agent信息
     */
    ProtectedResource getAgentResource(ProtectedResource clusterResource);
}
