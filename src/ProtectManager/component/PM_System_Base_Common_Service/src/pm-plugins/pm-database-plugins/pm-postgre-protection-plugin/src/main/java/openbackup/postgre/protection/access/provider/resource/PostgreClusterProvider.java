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
package openbackup.postgre.protection.access.provider.resource;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.ClusterInstanceOnlinePolicy;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.postgre.protection.access.common.PostgreConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * postgre集群provider
 *
 */
@Component
@Slf4j
public class PostgreClusterProvider implements EnvironmentProvider {
    private final ClusterEnvironmentService clusterEnvironmentService;

    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final InstanceResourceService instanceResourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final ResourceService resourceService;

    public PostgreClusterProvider(ClusterEnvironmentService clusterEnvironmentService,
        ProtectedEnvironmentService protectedEnvironmentService, InstanceResourceService instanceResourceService,
        AgentUnifiedService agentUnifiedService, ResourceService resourceService) {
        this.clusterEnvironmentService = clusterEnvironmentService;
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.instanceResourceService = instanceResourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.resourceService = resourceService;
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("Start check postgre cluster. name: {}", environment.getName());
        List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
        clusterEnvironmentService.checkClusterNodeNum(agents);
        List<ProtectedEnvironment> agentList = queryEnvironments(agents);
        clusterEnvironmentService.checkClusterNodeStatus(agentList);
        clusterEnvironmentService.checkClusterNodeOsType(agentList);
        if (StringUtils.equals(environment.getExtendInfo().get(PostgreConstants.INSTALL_DEPLOY_TYPE),
            PostgreConstants.CLUP)) {
            List<ProtectedResource> clupServers = environment.getDependencies().get(PostgreConstants.CLUP_SERVERS);
            checkClupServerNum(clupServers);
            List<ProtectedEnvironment> clupServerList = queryEnvironments(clupServers);
            clusterEnvironmentService.checkClusterNodeStatus(clupServerList);
            checkClupServerCluster(clupServerList, environment);
        }
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            environment.setUuid(UUIDGenerator.getUUID());
        } else {
            List<ProtectedResource> children = resourceService.getResourceByParentId(environment.getUuid());
            for (ProtectedResource child : children) {
                child.setPath(environment.getEndpoint());
                child.getExtendInfo().put(DatabaseConstants.VIRTUAL_IP, environment.getEndpoint());
            }
            resourceService.updateSourceDirectly(children);
        }
        log.info("End check postgre cluster. name: {}", environment.getName());
    }

    private List<ProtectedEnvironment> queryEnvironments(List<ProtectedResource> agents) {
        return agents.stream()
            .map(host -> protectedEnvironmentService.getEnvironmentById(host.getUuid()))
            .collect(Collectors.toList());
    }

    private void checkClupServerNum(List<ProtectedResource> clupServers) {
        Set<String> clupServerSet = clupServers.stream().map(ProtectedResource::getUuid).collect(Collectors.toSet());
        // check重复节点
        if (clupServers.size() != clupServerSet.size()) {
            log.error("There are duplicate clup servers.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "There are duplicate clup servers.");
        }
        // check管理节点个数
        if (clupServers.size() < IsmNumberConstant.ONE) {
            log.error("Select CLup server num is error");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Select CLup server num is error");
        }
    }

    private void checkClupServerCluster(List<ProtectedEnvironment> clupServerList, ProtectedEnvironment environment) {
        if (clupServerList.size() == IsmNumberConstant.ONE) {
            log.info("There is only one CLup server node, so there is no need to check the CLup server cluster.");
        } else {
            log.info("Start to check CLup server cluster.");
            ProtectedEnvironment clupServerEnvironment = clupServerList.get(0);
            AppEnv appEnv = BeanTools.copy(environment, AppEnv::new);
            appEnv.setSubType(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType());
            CheckAppReq checkAppReq = new CheckAppReq();
            checkAppReq.setAppEnv(appEnv);
            Application application = BeanTools.copy(environment, Application::new);
            application.setSubType(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType());
            if (VerifyUtil.isEmpty(application.getAuth())) {
                application.setAuth(environment.getAuth());
            }
            application.getExtendInfo().put(PostgreConstants.ACTION_TYPE, PostgreConstants.QUERY_CLUP_SERVER);
            checkAppReq.setApplication(application);
            AppEnvResponse clupServerClusterInfo = agentUnifiedService.getClusterInfo(
                ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType(), clupServerEnvironment, checkAppReq, false);
            if (CollectionUtils.isEmpty(clupServerClusterInfo.getNodes())) {
                log.error("Select Clup servers dont belong to the same cluster");
                throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_BELONG_TO_SAME_CLUSTER,
                    "Select Clup servers dont belong to the same cluster");
            }
            Set<String> clupServerQueryEndpointSet = clupServerClusterInfo.getNodes()
                .stream()
                .map(NodeInfo::getEndpoint)
                .collect(Collectors.toSet());
            Set<String> clupServerEndpointSet = clupServerList.stream()
                .map(ProtectedEnvironment::getExtendInfo)
                .filter(server -> server.containsKey("agentIpList"))
                .map(server -> server.get("agentIpList"))
                .flatMap((str -> Arrays.stream(str.split(","))))
                .map(String::trim)
                .collect(Collectors.toSet());
            Set<String> identicalClupServerEndpoint = new HashSet<String>(
                CollectionUtils.intersection(clupServerQueryEndpointSet, clupServerEndpointSet));
            if (identicalClupServerEndpoint.size() != clupServerList.size()) {
                log.error("Select Clup servers dont belong to the same cluster");
                throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_BELONG_TO_SAME_CLUSTER,
                    "Select Clup servers dont belong to the same cluster");
            }
            log.info("Succeed to check CLup server cluster.");
        }
    }

    private AgentBaseDto healthCheckSingleInstanceByAgent(ProtectedResource resource,
        ProtectedEnvironment environment) {
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        checkAppReq.setApplication(BeanTools.copy(resource, Application::new));
        return agentUnifiedService.check(resource.getSubType(), environment, checkAppReq);
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        if (StringUtils.equals(environment.getExtendInfo().get(PostgreConstants.INSTALL_DEPLOY_TYPE),
            PostgreConstants.CLUP)) {
            Map<String, Object> conditions = new HashMap<>();
            conditions.put(DatabaseConstants.PARENT_UUID, environment.getUuid());
            PageListResponse<ProtectedResource> data;
            List<ProtectedResource> resources = new ArrayList<>();
            int pageNo = 0;
            do {
                data = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, conditions);
                resources.addAll(data.getRecords());
                pageNo++;
            } while (data.getRecords().size() >= IsmNumberConstant.HUNDRED);
            updateResourceStatus(resources);
            List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
            List<ProtectedResource> clupServers = environment.getDependencies().get(PostgreConstants.CLUP_SERVERS);
            List<ProtectedResource> clupNodes = new ArrayList<>();
            clupNodes.addAll(agents);
            clupNodes.addAll(clupServers);
            for (ProtectedResource clupNode : clupNodes) {
                if (!(clupNode instanceof ProtectedEnvironment)) {
                    continue;
                }
                ProtectedEnvironment agentEnvironment = (ProtectedEnvironment) clupNode;
                if (StringUtils.equals(LinkStatusEnum.OFFLINE.getStatus().toString(),
                    agentEnvironment.getLinkStatus())) {
                    log.error("Postgre cluster health check fail. uuid: {}", environment.getUuid());
                    environment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
                    throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "Select host is offLine.");
                }
            }
        } else {
            instanceResourceService.healthCheckClusterInstanceOfEnvironment(environment,
                ClusterInstanceOnlinePolicy.ALL_NODES_ONLINE);
            List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
            boolean isOffline = agents.stream()
                .flatMap(StreamUtil.match(ProtectedEnvironment.class))
                .anyMatch(childNode -> LinkStatusEnum.OFFLINE.getStatus()
                    .toString()
                    .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(childNode)));
            if (isOffline) {
                log.error("Postgre cluster health check fail. uuid: {}", environment.getUuid());
                environment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
                throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "Select host is offLine.");
            }
        }
    }

    private void updateResourceStatus(List<ProtectedResource> resources) {
        resources.forEach(resource -> {
            ProtectedResource protectedResource = resourceService.getResourceById(resource.getUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not exist"));
            protectedResource.getExtendInfo().put(PostgreConstants.ACTION_TYPE, PostgreConstants.QUERY_CLUP_SERVER);
            protectedResource.getExtendInfo()
                .put(PostgreConstants.DB_INSTALL_PATH_KEY, PostgreConstants.CLUP_INSTALL_PATH);
            List<ProtectedResource> clupServers = protectedResource.getDependencies()
                .get(PostgreConstants.CLUP_SERVERS);
            // clup是高可用的有1个调通就行了
            String status = getStatus(clupServers, protectedResource);
            ProtectedResource updateResource = new ProtectedResource();
            updateResource.setUuid(protectedResource.getUuid());
            updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
            resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
        });
    }

    private String getStatus(List<ProtectedResource> clupServers, ProtectedResource protectedResource) {
        int errorNodeNumber = 0;
        String status = "";
        for (ProtectedResource clupServer : clupServers) {
            if (!(clupServer instanceof ProtectedEnvironment)) {
                continue;
            }
            clupServer.getExtendInfo().putAll(protectedResource.getExtendInfo());
            AgentBaseDto checkResult;
            try {
                checkResult = healthCheckSingleInstanceByAgent(protectedResource, (ProtectedEnvironment) clupServer);
            } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
                log.error("clup check application error: {}.", ExceptionUtil.getErrorMessage(e));
                errorNodeNumber++;
                continue;
            }
            if (checkResult.isAgentBaseDtoReturnSuccess()) {
                status = LinkStatusEnum.ONLINE.getStatus().toString();
                break;
            } else {
                JSONObject jsonObject = JSONObject.fromObject(checkResult.getErrorMessage());
                if (StringUtils.equals(jsonObject.getString("bodyErr"),
                    LinkStatusEnum.OFFLINE.getStatus().toString())) {
                    status = LinkStatusEnum.OFFLINE.getStatus().toString();
                    break;
                } else {
                    errorNodeNumber++;
                }
            }
        }
        if (errorNodeNumber == clupServers.size()) {
            status = LinkStatusEnum.OFFLINE.getStatus().toString();
        }
        return status;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.POSTGRE_CLUSTER.equalsSubType(resourceSubType);
    }
}