/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;

/**
 * 主机环境上资源浏览 Provider
 *
 * @author lwx776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-04
 */
public interface ResourceBrowseProvider extends DataProtectionProvider<String> {
    /**
     * 浏览环境资源
     *
     * @param environment 受保护环境
     * @param environmentConditions 查询资源的条件
     * @return 返回资源列表
     */
    PageListResponse<ProtectedResource> browse(
        ProtectedEnvironment environment, BrowseEnvironmentResourceConditions environmentConditions);
}