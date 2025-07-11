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

import java.util.Optional;

/**
 * 受保护环境服务接口定义
 *
 */
public interface ProtectedEnvironmentService {
    /**
     * 扫描受保护环境
     *
     * @param protectedEnvironment 受保护环境
     * @return environment uuid
     */
    String register(ProtectedEnvironment protectedEnvironment);

    /**
     * 检查受保护环境
     *
     * @param protectedEnvironment 受保护环境
     * @return 检查结果
     */
    ActionResult[] checkProtectedEnvironment(ProtectedEnvironment protectedEnvironment);

    /**
     * 浏览受保护环境资源
     *
     * @param environmentConditions 查询资源的条件
     * @return 环境资源列表
     */
    PageListResponse<ProtectedResource> browse(
        BrowseEnvironmentResourceConditions environmentConditions);

    /**
     *
     * 查询受保护环境ID
     *
     * @param envId 环境ID
     * @return 受保护环境ID
     */
    ProtectedEnvironment getEnvironmentById(String envId);

    /**
     * 查询受保护环境
     *
     * @param envId 环境Id
     * @return 受保护环境
     */
    Optional<ProtectedEnvironment> getBasicEnvironmentById(String envId);

    /**
     * 根据受保护环境的ID删除受保护环境
     *
     * @param envId 受保护环境的ID
     */
    void deleteEnvironmentById(String envId);

    /**
     * 刷新受保护环境
     *
     * @param envId 受保护环境的ID
     */
    void refreshEnvironment(String envId);

    /**
     * 更新受保护环境
     *
     * @param environment 受保护环境
     */
    void updateEnvironment(ProtectedEnvironment environment);

    /**
     * 是否已有该环境信息在数据库中
     *
     * @param environment 需要注册的环境
     * @return 是否存在
     */
    boolean hasSameEnvironmentInDb(ProtectedEnvironment environment);

    /**
     * 是否存在相同endpoint的环境信息
     *
     * @param environment 待注册环境
     */
    void checkHasSameEndpointEnvironment(ProtectedEnvironment environment);

    /**
     * 是否为系统重装后并且执行管理数据恢复时,内置agent注册场景
     *
     * @param environment 待注册环境
     */
    void updateInternalAgentAfterSystemDataRecovery(ProtectedEnvironment environment);
}
