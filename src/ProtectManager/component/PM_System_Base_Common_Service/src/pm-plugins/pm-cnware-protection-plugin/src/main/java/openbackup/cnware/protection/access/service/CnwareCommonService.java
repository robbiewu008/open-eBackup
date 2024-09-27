/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.cnware.protection.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;

import java.util.List;

/**
 * CNware类型服务
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-12-11
 */
public interface CnwareCommonService {
    /**
     * 检查环境名称
     *
     * @param name 环境名称
     */
    void checkEnvName(String name);

    /**
     * 查询集群信息
     *
     * @param environment environment
     * @param agent agent
     * @return AppEnvResponse
     */
    AppEnvResponse queryClusterInfo(ProtectedEnvironment environment, ProtectedEnvironment agent);

    /**
     * 获取Agent环境信息
     *
     * @param envId 环境uuid
     * @return Agent环境信息
     */
    ProtectedEnvironment getEnvironmentById(String envId);

    /**
     * 校验Agent连通性
     *
     * @param environment 环境信息
     * @param agentEnvList agent环境信息列表
     */
    void checkConnectivity(ProtectedEnvironment environment, List<ProtectedEnvironment> agentEnvList);
}
