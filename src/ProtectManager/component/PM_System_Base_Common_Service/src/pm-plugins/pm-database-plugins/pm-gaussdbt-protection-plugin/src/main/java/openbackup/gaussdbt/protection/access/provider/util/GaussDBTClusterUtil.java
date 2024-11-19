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
package openbackup.gaussdbt.protection.access.provider.util;

import com.fasterxml.jackson.core.type.TypeReference;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTClusterStateEnum;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * GaussDBTClusterUtil
 *
 */
public class GaussDBTClusterUtil {
    /**
     * 设置GaussDBT集群信息
     *
     * @param environment 集群环境信息
     * @param appEnvResponse agent请求返回的参数
     * @param resourceCheckContext 资源上下文
     */
    public static void setClusterInfo(ProtectedEnvironment environment, AppEnvResponse appEnvResponse,
        ResourceCheckContext resourceCheckContext) {
        Map<String, String> appExtendInfo = Optional.ofNullable(appEnvResponse.getExtendInfo()).orElse(new HashMap<>());
        environment.setVersion(appExtendInfo.get(GaussDBTConstant.CLUSTER_VERSION_KEY));
        String state = appExtendInfo.get(GaussDBTConstant.CLUSTER_STATE_KEY);
        // 环境状态只有在线和离线：降级和在线 -> 环境在线
        if (GaussDBTClusterStateEnum.getOnlineClusterState().contains(state)) {
            environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        } else {
            // 不可用和异常 -> 环境离线
            environment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        }
        List<NodeInfo> nodes = getNodeInfoList(resourceCheckContext, appEnvResponse.getNodes());
        String endPoint = nodes.stream().map(NodeInfo::getEndpoint).collect(Collectors.joining(";"));
        environment.setEndpoint(endPoint);
        environment.setPath(endPoint);
        Map<String, String> envExtendInfo = Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>());
        envExtendInfo.put(GaussDBTConstant.NODES_KEY, JSONObject.writeValueAsString(nodes));
        envExtendInfo.put(GaussDBTConstant.CLUSTER_STATE_KEY, appExtendInfo.get(GaussDBTConstant.CLUSTER_STATE_KEY));
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, appExtendInfo.get(DatabaseConstants.DEPLOY_TYPE));
        environment.setExtendInfo(envExtendInfo);
    }

    /**
     * 获取GaussDBT集群节点集合
     *
     * @param resourceCheckContext 资源上下文
     * @param nodes 节点
     * @return 节点集合
     */
    public static List<NodeInfo> getNodeInfoList(ResourceCheckContext resourceCheckContext, List<NodeInfo> nodes) {
        Optional<List<ProtectedEnvironment>> agentList = resourceCheckContext.getResourceConnectableMap()
            .values()
            .stream()
            .findFirst();
        return nodes.stream()
            .map(nodeInfo -> agentList.get()
                .stream()
                .filter(agent -> nodeInfo.getName().equals(agent.getName()))
                .findFirst()
                .map(agent -> {
                    nodeInfo.setUuid(agent.getUuid());
                    return nodeInfo;
                })
                .orElseGet(() -> nodeInfo))
            .collect(Collectors.toList());
    }

    /**
     * 将节点信息转换为TaskEnvironment
     *
     * @param nodeInfo 环境扩展信息中的node信息
     * @return TaskEnvironment
     */
    public static TaskEnvironment convertNodeInfoToTaskEnvironment(NodeInfo nodeInfo) {
        TaskEnvironment taskEnv = new TaskEnvironment();
        taskEnv.setName(nodeInfo.getName());
        taskEnv.setUuid(nodeInfo.getUuid());
        taskEnv.setType(nodeInfo.getType());
        taskEnv.setSubType(nodeInfo.getSubType());
        taskEnv.setEndpoint(nodeInfo.getEndpoint());
        Map<String, String> extendInfo = Optional.ofNullable(taskEnv.getExtendInfo()).orElse(new HashMap<>());
        Map<String, String> nodeExtendInfo = Optional.ofNullable(nodeInfo.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(DatabaseConstants.ROLE, nodeExtendInfo.get(DatabaseConstants.ROLE));
        extendInfo.put(GaussDBTConstant.NODE_STATUS_KEY, nodeExtendInfo.get(GaussDBTConstant.NODE_STATUS_KEY));
        taskEnv.setExtendInfo(extendInfo);
        return taskEnv;
    }

    /**
     * 从保护集群环境中获取任务环境nodes
     *
     * @param environment 保护集群环境
     * @return List<TaskEnvironment>
     */
    public static List<TaskEnvironment> getNodesFromEnv(ProtectedEnvironment environment) {
        String nodeString = environment.getExtendInfo().get(GaussDBTConstant.NODES_KEY);
        List<NodeInfo> nodeInfoList = JsonUtil.read(nodeString, new TypeReference<List<NodeInfo>>() {
        });
        return nodeInfoList.stream()
            .map(GaussDBTClusterUtil::convertNodeInfoToTaskEnvironment)
            .collect(Collectors.toList());
    }
}
