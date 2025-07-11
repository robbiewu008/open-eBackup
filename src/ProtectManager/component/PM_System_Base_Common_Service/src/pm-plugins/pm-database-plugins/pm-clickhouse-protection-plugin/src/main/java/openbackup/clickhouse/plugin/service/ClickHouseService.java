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
package openbackup.clickhouse.plugin.service;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;
import java.util.Optional;

/**
 * ClickHouse服务
 *
 */
public interface ClickHouseService {
    /**
     * 在创建或者更新集群之前做一些校验工作。
     *
     * @param protectedEnvironment 集群
     */
    void preCheck(ProtectedEnvironment protectedEnvironment);

    /**
     * 校验集群节点是否已经添加过。
     *
     * @param protectedResource 集群节点
     */
    void checkNodeExists(ProtectedResource protectedResource);

    /**
     * 扫描ClickHouse集群下面的数据库
     *
     * @param protectedEnvironment 集群
     * @return 数据库资源
     */
    List<ProtectedResource> scanDataBases(ProtectedEnvironment protectedEnvironment);

    /**
     * 选择一个在线的agent
     *
     * @param child child
     * @return agent
     */
    Endpoint selectAgent(ProtectedResource child);

    /**
     * 查询库下面的表
     *
     * @param environment 集群
     * @param environmentConditions 子资源查询过滤条件对象组合
     * @return 表的列表
     */
    PageListResponse<ProtectedResource> browseTables(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions);

    /**
     * 集群健康检查
     *
     * @param environment cluster
     * @param isUpdateDB 是否需要更新数据库，false一般用于只检测，不更新的场景，比如注册时
     * @return health status
     */
    Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment, boolean isUpdateDB);

    /**
     * 查询集群详情
     *
     * @param environment agent
     * @param resource node
     * @param queryType 查询类型database/version/table
     * @param queryDatabase 如果查询类型是table，需要传入数据库名
     * @param environmentConditions 分页参数
     * @return 集群详情
     */
    PageListResponse<ProtectedResource> queryClusterDetail(ProtectedEnvironment environment, ProtectedResource resource,
        String queryType, String queryDatabase, BrowseEnvironmentResourceConditions environmentConditions);
}
