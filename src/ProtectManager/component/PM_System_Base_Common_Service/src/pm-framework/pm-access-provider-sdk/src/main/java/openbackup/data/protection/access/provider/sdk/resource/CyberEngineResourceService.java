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
package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;

/**
 * 功能描述 安全一体机受保护资源服务类
 *
 */
public interface CyberEngineResourceService {
    /**
     * 批量查询所有租户信息
     *
     * @param pageNo 页面编号
     * @param pageSize 页面大小
     * @return 租户信息
     */
    PageListResponse<TenantInfo> listAllTenants(int pageNo, int pageSize);

    /**
     * 批量查询所有租户信息
     *
     * @param deviceId 设备Id
     * @return 租户信息
     */
    PageListResponse<TenantInfo> listAllTenantsByDeviceId(String deviceId);

    /**
     * 使用文件系统ID查询租户信息
     *
     * @param pageNo 页面编号
     * @param pageSize 页面大小
     * @param resourceId 文件系统ID
     * @return 租户信息
     */
    PageListResponse<TenantInfo> listTenantByResourceId(int pageNo, int pageSize, String resourceId);

    /**
     * 批量查设备信息
     *
     * @param deviceId 设备Id
     * @param tenantId 租户Id
     * @return 设备信息
     */
    StorageInfo listStorageInfo(String deviceId, String tenantId);

    /**
     * 批量查文件系统信息
     *
     * @param pageNo 页面编号
     * @param pageSize 页面大小
     * @param tenantId 租户Id
     * @param resourceId 文件系统Id
     * @return 文件系统信息
     */
    PageListResponse<FileSystemInfo> listFileSystems(int pageNo, int pageSize, String tenantId, String resourceId);

    /**
     * 安全一体机删除设备
     *
     * @param environmentId 环境uuid
     */
    void deleteEnvironment(String environmentId);
}
