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
package openbackup.saphana.protection.access.service;

import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;
import java.util.Map;

/**
 * SAP HANA资源Service接口
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-10
 */
public interface SapHanaResourceService {
    /**
     * 查询SAP HANA的agent详细信息列表
     *
     * @param agents SAP HANA的agent id列表
     * @return agent主机信息列表
     */
    List<ProtectedEnvironment> queryEnvironments(List<ProtectedResource> agents);

    /**
     * 根据查询条件查询资源
     *
     * @param conditions 资源查询条件
     * @return 资源列表
     */
    List<ProtectedResource> listResourcesByConditions(Map<String, Object> conditions);

    /**
     * 根据资源UUID获取资源信息
     *
     * @param resourceId 资源的UUID
     * @return ProtectedResource 资源信息
     */
    ProtectedResource getResourceById(String resourceId);

    /**
     * 根据检查结果获取实例状态
     *
     * @param environment 环境信息
     * @param actionResults 连通检查结果
     * @return 实例状态
     */
    String getInstStatusByActionResults(ProtectedEnvironment environment, List<ActionResult> actionResults);

    /**
     * 检查SAP HANA实例规格
     */
    void checkInstanceNumber();

    /**
     * 检查SAP HANA实例是否已注册
     *
     * @param environment SAP HANA实例资源信息
     */
    void checkInstanceIsRegistered(ProtectedEnvironment environment);

    /**
     * 检查SAP HANA数据库是否已注册
     *
     * @param resource SAP HANA数据库资源信息
     */
    void checkDbIsRegistered(ProtectedResource resource);

    /**
     * 检查SAP HANA数据库是否已在通用数据库中注册
     *
     * @param resource SAP HANA数据库资源信息
     */
    void checkDbIsRegisteredInGeneralDb(ProtectedResource resource);

    /**
     * 检查SAP HANA数据库资源的状态
     *
     * @param resource SAP HANA数据库资源信息
     */
    void checkDatabaseConnection(ProtectedResource resource);

    /**
     * 是否执行修改操作
     *
     * @param resource SAP HANA资源信息
     * @return 是否修改
     */
    boolean isModifyResource(ProtectedResource resource);

    /**
     * 使用SAP HANA实例资源更新实例和包含数据库资源的状态
     *
     * @param environment SAP HANA实例资源信息
     * @param shouldUpdateSystemDb 是否更新系统数据库
     * @param shouldUpdateTenantDb 是否更新租户数据库
     * @param linkStatus 状态
     */
    void updateInstAndDbLinkStatusByInst(ProtectedEnvironment environment, String linkStatus,
        boolean shouldUpdateSystemDb, boolean shouldUpdateTenantDb);

    /**
     * 更新SAP HANA实例资源的状态
     *
     * @param environment SAP HANA实例资源信息
     * @param linkStatus 状态
     */
    void updateInstanceLinkStatus(ProtectedEnvironment environment, String linkStatus);

    /**
     * 更新SAP HANA实例包含的数据库资源的状态
     *
     * @param environment SAP HANA实例资源信息
     * @param shouldUpdateSystemDb 是否更新系统数据库
     * @param shouldUpdateTenantDb 是否更新租户数据库
     * @param linkStatus 状态
     */
    void updateDbLinkStatusOfInstance(ProtectedEnvironment environment, String linkStatus, boolean shouldUpdateSystemDb,
        boolean shouldUpdateTenantDb);

    /**
     * 更新SAP HANA数据库资源的状态
     *
     * @param resource SAP HANA数据库资源信息
     * @param linkStatus 状态
     */
    void updateDbLinkStatus(ProtectedResource resource, String linkStatus);

    /**
     * 检查并更新SAP HANA租户数据库资源的状态
     *
     * @param environment SAP HANA实例资源信息
     */
    void checkAndUpdateTenantDbLinkStatusOfInstance(ProtectedEnvironment environment);

    /**
     * 设置SAP HANA数据库资源的环境信息
     *
     * @param resource SAP HANA数据库资源信息
     */
    void setDatabaseResourceInfo(ProtectedResource resource);
}
