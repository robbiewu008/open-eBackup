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
package openbackup.opengauss.resources.access.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * OpenGauss mok data
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-20
 */
public class OpenGaussMockData {
    public static final String agentId = "e40c888d-fa29-3da3-b3f0-847545d00411";
    public static final String agentId2 = "e40c888d-fa29-3da3-b3f0-847545d00412";
    public static final String agentId3 = "e40c888d-fa29-3da3-b3f0-847545d00413";
    public static final String environmentId = "e9720043-6c3c-33e5-b1f0-e9f2268488e6";

    public static AppEnvResponse buildAppEnvResponse() {
        AppEnvResponse appEnvResponse = commonAppEnvResponse();
        appEnvResponse.setExtendInfo(getClusterStateNormal());
        return appEnvResponse;
    }

    private static Map<String, String> getClusterStateNormal() {
        Map<String, String> extra = new HashMap<>();
        extra.put("clusterState", "Normal");
        extra.put("clusterVersion", "MogDB 2.1.1");
        extra.put("deployType", "3");
        extra.put(OpenGaussConstants.SYSTEM_ID, "13456789");
        return extra;
    }

    private static AppEnvResponse commonAppEnvResponse() {
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setName("node_1");
        nodeInfo1.setType(ResourceTypeEnum.DATABASE.getType());
        nodeInfo1.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        nodeInfo1.setEndpoint("8.40.97.211");
        nodeInfo1.setRole(1);

        Map<String, String> extendInfo = new HashMap<>();
        nodeInfo1.setExtendInfo(extendInfo);

        List<NodeInfo> nodeInfoList = new ArrayList<>();
        nodeInfoList.add(nodeInfo1);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setNodes(nodeInfoList);
        return appEnvResponse;
    }

    public static AppEnvResponse buildAppEnvResponseClusterStateUnavailable() {
        AppEnvResponse appEnvResponse = commonAppEnvResponse();
        appEnvResponse.setExtendInfo(getClusterStateUnavailable());
        return appEnvResponse;
    }

    private static Map<String, String> getClusterStateUnavailable() {
        Map<String, String> clusterStateStateUnavailable = new HashMap<>();
        clusterStateStateUnavailable.put("clusterState", "Unavailable");
        clusterStateStateUnavailable.put("clusterVersion", "MogDB 2.1.1");
        clusterStateStateUnavailable.put("deployType", "3");
        return clusterStateStateUnavailable;
    }

    public static AppEnvResponse buildAppEnvResponseNodeEndpointEmpty() {
        NodeInfo nodeInfo1 = getNodeInfoEndpointEmpty();
        Map<String, String> extendInfo = new HashMap<>();
        nodeInfo1.setExtendInfo(extendInfo);
        List<NodeInfo> nodeInfoList = new ArrayList<>();
        nodeInfoList.add(nodeInfo1);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setNodes(nodeInfoList);
        Map<String, String> extra = new HashMap<>();
        extra.put("clusterState", "Normal");
        extra.put("clusterVersion", "MogDB 2.1.1");
        extra.put("deployType", "3");
        appEnvResponse.setExtendInfo(extra);
        return appEnvResponse;
    }

    private static NodeInfo getNodeInfoEndpointEmpty() {
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setName("node_1");
        nodeInfo1.setType(ResourceTypeEnum.DATABASE.getType());
        nodeInfo1.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        nodeInfo1.setRole(1);
        return nodeInfo1;
    }

    /**
     * @return 节点的Endpoint为空
     */
    public static AppEnvResponse buildAppEnvResponseEndpointIsEmpty() {
        NodeInfo nodeInfo1 = getNodeInfo();

        List<NodeInfo> nodeInfoList = new ArrayList<>();
        nodeInfoList.add(nodeInfo1);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setNodes(nodeInfoList);

        Map<String, String> extra = new HashMap<>();
        extra.put("clusterState", "Normal");
        extra.put("clusterVersion", "MogDB 2.1.1");
        extra.put("deployType", "3");
        appEnvResponse.setExtendInfo(extra);
        return appEnvResponse;
    }

    public static NodeInfo getNodeInfo() {
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setName("node_1");
        nodeInfo1.setType(ResourceTypeEnum.DATABASE.getType());
        nodeInfo1.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        nodeInfo1.setRole(1);

        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OpenGaussConstants.CLUSTER_NODE_ROLE, "1");
        nodeInfo1.setExtendInfo(extendInfo);
        return nodeInfo1;
    }

    /**
     * 备份任务的mock数据
     *
     * @param subType subType
     * @return BackupTask
     */
    public static BackupTask mockBackupTask(String subType) {
        BackupTask backupTask = new BackupTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setSubType(subType);
        backupTask.setProtectObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid(UUID.randomUUID().toString());
        taskEnvironment.setExtendInfo(new HashMap());
        backupTask.setProtectEnv(taskEnvironment);
        // 设置存储仓
        backupTask.setRepositories(getStorageRepositories());
        return backupTask;
    }

    public static List<StorageRepository> getStorageRepositories() {
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository dataStorageRepository = new StorageRepository();
        dataStorageRepository.setType(RepositoryTypeEnum.DATA.getType());
        repositories.add(dataStorageRepository);
        return repositories;
    }

    public static ProtectedEnvironment mockProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedEnvironment agentProEnv = new ProtectedEnvironment();
        agentProEnv.setUuid("xxxxxxxxxxxxxxxxxxxx11");
        agentProEnv.setEndpoint("2.3.3.3");
        agentProEnv.setPort(8963);
        agentProEnv.setName("node_1");
        dependency.put(DatabaseConstants.AGENTS, Collections.singletonList(agentProEnv));
        Authentication auth = new Authentication();
        auth.setAuthKey("omm");
        auth.setAuthType(Authentication.OS_PASSWORD);
        protectedEnvironment.setAuth(auth);
        protectedEnvironment.setDependencies(dependency);
        protectedEnvironment.setType(ResourceTypeEnum.DATABASE.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        return protectedEnvironment;
    }

    public static TaskEnvironment getTaskEnvironment() {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("xxxxxxxxxxxxxxxxxxxx11");
        Map<String, String> extendInfoNodes =  new HashMap<>();
        extendInfoNodes.put(OpenGaussConstants.NODES,JsonUtil.json(Collections.singletonList(getNodeInfo())));
        taskEnvironment.setExtendInfo(extendInfoNodes);
        return taskEnvironment;
    }

    public static TaskEnvironment mockTaskEnvironmentContainsNodes() {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("xxxxxxxxxxxxxxxxxxxx11");
        Map<String, String> extendInfoNodes = new HashMap<>();
        extendInfoNodes.put(OpenGaussConstants.NODES, JsonUtil.json(Collections.singletonList(getNodeInfo())));
        taskEnvironment.setExtendInfo(extendInfoNodes);
        taskEnvironment.setName("opengauss_cluster");

        taskEnvironment.setNodes(Collections.singletonList(getEnvironmentNode()));
        return taskEnvironment;
    }

    private static TaskEnvironment getEnvironmentNode() {
        TaskEnvironment nodeInfo1 = new TaskEnvironment();
        nodeInfo1.setName("node_1");
        nodeInfo1.setType(ResourceTypeEnum.DATABASE.getType());
        nodeInfo1.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        nodeInfo1.setUuid("uuid_node_1111");

        Map<String,String> roleExtendInfo = new HashMap<>();
        roleExtendInfo.put("role","1");
        nodeInfo1.setExtendInfo(roleExtendInfo);

        Map<String, String> extendInfo = new HashMap<>();
        nodeInfo1.setExtendInfo(extendInfo);
        return nodeInfo1;
    }

    public static ResourceCheckContext mockResourceCheckContext() {
        Map<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.VERSION, "9.6.0");
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(DatabaseConstants.SUCCESS_CODE);
        actionResult.setMessage(map.toString());
        List<ActionResult> list = new ArrayList<>();
        list.add(actionResult);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(list);
        return context;
    }

    public static ResourceCheckContext mockResourceCheckContextCluster() {
        ResourceCheckContext context = mockResourceCheckContext();
        context.getContext().put(OpenGaussConstants.CLUSTER_INFO,buildAppEnvResponse());
        return context;
    }

    public static ProtectedEnvironment getAgentEnvironment() {
        return buildCommonProtectedEnvironment();
    }

    public static ProtectedEnvironment buildCommonProtectedEnvironment() {
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid(agentId);
        agent.setEndpoint("8.3.6.63");
        agent.setPort(8088);
        return agent;
    }

    public static ProtectedEnvironment getAgentEnvironment3LinkStatusIsOffline() {
        ProtectedEnvironment agent = buildCommonProtectedEnvironment();
        agent.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        agent.setUuid(agentId3);
        return agent;
    }

    public static ProtectedEnvironment getAgentEnvironmentLinkStatusIsOnline() {
        ProtectedEnvironment agent = buildCommonProtectedEnvironment();
        agent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        return agent;
    }

    public static ProtectedEnvironment getAgentEnvironment2() {
        ProtectedEnvironment agent = buildCommonProtectedEnvironment();
        agent.setUuid(agentId2);
        return agent;
    }

    public static RestoreTask buildRestoreTaskEnvNodeInfo() {
        RestoreTask restoreTask = new RestoreTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        HashMap<String, String> extendInfo = new HashMap<>();
        String jsonNode = JsonUtil.json(Collections.singletonList(getNodeInfo()));
        extendInfo.put(OpenGaussConstants.NODES, jsonNode);
        taskEnvironment.setExtendInfo(extendInfo);
        taskEnvironment.setName("opengauss_cluster");

        restoreTask.setTargetEnv(taskEnvironment);

        TaskResource taskResource = new TaskResource();
        taskResource.setName("openGauss-instance-name");
        taskResource.setSubType(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType());
        restoreTask.setTargetObject(taskResource);
        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put(DatabaseConstants.DATABASE_NEW_NAME, "newName");
        restoreTask.setAdvanceParams(advanceParams);

        return restoreTask;
    }
}
