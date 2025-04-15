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
package openbackup.opengauss.resources.access.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.opengauss.resources.access.enums.OpenGaussClusterStateEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.util.StreamUtil;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * OpenGaussClusterUtil 工具类
 *
 */
@Slf4j
public class OpenGaussClusterUtil {
    /**
     * 查询集群信息，实时更新环境信息
     *
     * @param environment 环境信息
     * @param clusterInfo 集群信息
     * @return ProtectedEnvironment
     */
    public static ProtectedEnvironment buildProtectedEnvironment(ProtectedEnvironment environment,
        AppEnvResponse clusterInfo) {
        Map<String, String> environmentExtendInfo = Optional.ofNullable(environment.getExtendInfo())
            .orElseGet(HashMap::new);

        // 放置集群的扩展信息
        environmentExtendInfo.putAll(clusterInfo.getExtendInfo());
        environmentExtendInfo.put(OpenGaussConstants.NODES, JSONObject.writeValueAsString(clusterInfo.getNodes()));
        environment.setExtendInfo(environmentExtendInfo);
        return environment;
    }

    /**
     * 检查联调性获取检查结果
     *
     * @param resourceCheckContext 资源检查上下文
     * @return AppEnvResponse
     */
    public static AppEnvResponse getContextClusterInfo(ResourceCheckContext resourceCheckContext) {
        Map<String, Object> context = Optional.ofNullable(resourceCheckContext.getContext()).orElse(new HashMap<>());
        Object clusterInfo = context.get(OpenGaussConstants.CLUSTER_INFO);
        if (Objects.isNull(clusterInfo) || !(clusterInfo instanceof AppEnvResponse)) {
            log.error("failed to query OpenGauss cluster nodes.");
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "failed to query OpenGauss cluster nodes.");
        }
        return (AppEnvResponse) clusterInfo;
    }

    /**
     * 检查集群是否在线
     *
     * @param clusterInfo 集群
     */
    public static void checkClusterState(AppEnvResponse clusterInfo) {
        Map<String, String> clusterState = Optional.ofNullable(clusterInfo.getExtendInfo()).orElse(new HashMap<>());
        boolean isOnline = OpenGaussClusterStateEnum.getOnlineClusterState()
            .contains(clusterState.get(OpenGaussConstants.CLUSTER_STATE));
        PowerAssert.state(isOnline, () -> new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
            "The open gauss cluster status is not normal"));
    }

    /**
     * 从环境中获取agent端口信息
     *
     * @param protectedEnvironment 环境信息
     * @return List<Endpoint>
     */
    public static List<Endpoint> buildAgentEndpointFromEnv(ProtectedEnvironment protectedEnvironment) {
        Map<String, List<ProtectedResource>> dependencies = protectedEnvironment.getDependencies();
        List<ProtectedResource> resourceList = dependencies.get(DatabaseConstants.AGENTS);
        return resourceList.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .filter(env -> LinkStatusEnum.ONLINE.getStatus().toString().equals(env.getLinkStatus()))
            .map(OpenGaussClusterUtil::applyAgentEndpoint)
            .collect(Collectors.toList());
    }

    private static Endpoint applyAgentEndpoint(ProtectedEnvironment agentProtectedEnv) {
        Endpoint endpoint = new Endpoint();
        endpoint.setIp(agentProtectedEnv.getEndpoint());
        endpoint.setPort(agentProtectedEnv.getPort());
        endpoint.setId(agentProtectedEnv.getUuid());
        return endpoint;
    }
}
