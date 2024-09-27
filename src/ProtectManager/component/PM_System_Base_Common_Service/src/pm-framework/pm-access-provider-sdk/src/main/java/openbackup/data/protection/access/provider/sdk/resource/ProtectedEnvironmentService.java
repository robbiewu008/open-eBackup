/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;

import java.util.Optional;

/**
 * 受保护环境服务接口定义
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-15
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
