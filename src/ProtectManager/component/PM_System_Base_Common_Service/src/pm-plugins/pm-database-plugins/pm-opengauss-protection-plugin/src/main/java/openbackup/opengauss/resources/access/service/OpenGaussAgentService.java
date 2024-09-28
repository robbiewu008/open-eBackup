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
package openbackup.opengauss.resources.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;

/**
 * OpenGaussAgentService与agent插件交互的接口
 *
 */
public interface OpenGaussAgentService {
    /**
     * 查询集群节点信息
     *
     * @param protectedResource 检查的环境资源
     * @return AppEnvResponse 集群信息
     */
    AppEnvResponse getClusterNodeStatus(ProtectedResource protectedResource);

    /**
     * 获取agent端口信息
     *
     * @param envId 环境id
     * @return List<Endpoint> 端口信息
     */
    List<Endpoint> getAgentEndpoint(String envId);

    /**
     * 构建环境的nodes信息
     *
     * @param taskEnvironment taskEnvironment
     * @return 环境信息
     */
    TaskEnvironment buildEnvironmentNodes(TaskEnvironment taskEnvironment);
}
