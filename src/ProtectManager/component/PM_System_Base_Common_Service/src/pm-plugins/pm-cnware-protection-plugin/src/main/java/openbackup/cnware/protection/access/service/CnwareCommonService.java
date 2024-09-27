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
