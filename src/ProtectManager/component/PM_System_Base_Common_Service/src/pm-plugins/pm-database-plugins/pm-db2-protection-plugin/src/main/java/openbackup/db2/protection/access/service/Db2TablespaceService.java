/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.db2.protection.access.service;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * db2表空间服务
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-07
 */
public interface Db2TablespaceService {
    /**
     * 设置表空间被锁定的状态
     *
     * @param databaseId 数据库id
     * @param tablespaceList 表空间信息
     */
    void setTablespaceLockedStatus(String databaseId, PageListResponse<ProtectedResource> tablespaceList);

    /**
     * 查询单机环境上的表空间
     *
     * @param environment 环境信息
     * @param environmentConditions 查询条件
     * @return PageListResponse<ProtectedResource> 表空间
     */
    PageListResponse<ProtectedResource> querySingleTablespace(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions);

    /**
     * 查询集群环境上的表空间
     *
     * @param environment 环境信息
     * @param environmentConditions 查询条件
     * @return PageListResponse<ProtectedResource> 表空间
     */
    PageListResponse<ProtectedResource> queryClusterTablespace(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions);
}
