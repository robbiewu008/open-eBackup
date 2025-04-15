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
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.postgre.protection.access.common.PostgreConstants;
import openbackup.postgre.protection.access.common.PostgreErrorCode;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * postgre集群实例provider
 *
 */
@Component
@Slf4j
public class PostgreClusterInstanceProvider implements ResourceProvider {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final InstanceResourceService instanceResourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final ResourceService resourceService;

    public PostgreClusterInstanceProvider(ProtectedEnvironmentService protectedEnvironmentService,
        InstanceResourceService instanceResourceService, AgentUnifiedService agentUnifiedService,
        ResourceService resourceService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.instanceResourceService = instanceResourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.resourceService = resourceService;
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> children = resourceService.queryDependencyResources(true, DatabaseConstants.CHILDREN,
            Collections.singletonList(resource.getUuid()));
        children.stream().forEach(child -> {
            Map<String, List<ProtectedResource>> childDependencies = new HashMap<>();
            List<ProtectedResource> agents = resourceService.queryDependencyResources(true, DatabaseConstants.AGENTS,
                Collections.singletonList(child.getUuid()));
            childDependencies.put(DatabaseConstants.AGENTS, agents);
            child.setDependencies(childDependencies);
        });
        dependencies.put(DatabaseConstants.CHILDREN, children);
        List<ProtectedResource> clupServers = resourceService.queryDependencyResources(true,
            PostgreConstants.CLUP_SERVERS, Collections.singletonList(resource.getUuid()));
        if (CollectionUtils.isNotEmpty(clupServers)) {
            dependencies.put(PostgreConstants.CLUP_SERVERS, clupServers);
        }
        resource.setDependencies(dependencies);
        return true;
    }

    @Override
    public void check(ProtectedResource resource) {
        log.info("Start create postgre cluster instance check. resource name: {}", resource.getName());
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(resource.getParentUuid());
        resource.setEnvironment(environment);
        resource.getExtendInfo().put(DatabaseConstants.VIRTUAL_IP, resource.getEnvironment().getEndpoint());

        // 校验实例是否已经存在
        instanceResourceService.checkClusterInstanceIsRegistered(resource);
        String installDeployType = resource.getExtendInfo().get(PostgreConstants.INSTALL_DEPLOY_TYPE);
        if (StringUtils.equals(installDeployType, PostgreConstants.CLUP)) {
            // 调用CLup server查找集群信息，填充进对应的节点中，不用用户输入
            Map<String, List<NodeInfo>> appEnvMap = getClupClusterInfo(resource, environment);
            if (CollectionUtils.size(appEnvMap.keySet()) != CollectionUtils.size(
                resource.getDependencies().get(DatabaseConstants.CHILDREN))) {
                log.error("Select server is error");
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Select server is error");
            }
            resource.getDependencies()
                .get(DatabaseConstants.CHILDREN)
                .forEach(childNode -> buildClusterNode(resource, childNode, appEnvMap));
            resource.setPath(resource.getEnvironment().getEndpoint());
        } else {
            checkCluster(resource);
        }
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End create postgre cluster instance check. resource name: {}", resource.getName());
    }

    private Map<String, List<NodeInfo>> getClupClusterInfo(ProtectedResource resource,
        ProtectedEnvironment environment) {
        AppEnv appEnv = BeanTools.copy(resource, AppEnv::new);
        appEnv.setSubType(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType());
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(appEnv);
        Application application = BeanTools.copy(resource, Application::new);
        application.setSubType(resource.getSubType());
        if (VerifyUtil.isEmpty(application.getAuth())) {
            application.setAuth(resource.getAuth());
        }
        checkAppReq.setApplication(application);
        List<ProtectedResource> clupServers = environment.getDependencies().get(PostgreConstants.CLUP_SERVERS);
        if (CollectionUtils.isEmpty(clupServers)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The clupServers is empty.");
        }
        ProtectedEnvironment clupServerEnvironment = protectedEnvironmentService.getEnvironmentById(
            clupServers.get(0).getUuid());
        AppEnvResponse clusterInstanceInfo = agentUnifiedService.getClusterInfo(
            ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType(), clupServerEnvironment, checkAppReq, false);
        Map<String, String> clusterExtendInfo = clusterInstanceInfo.getExtendInfo();
        if (MapUtils.isNotEmpty(clusterExtendInfo) && clusterExtendInfo.containsKey("bodyErr")) {
            throw new LegoCheckedException(MapUtils.getLong(clusterExtendInfo, "bodyErr"),
                MapUtils.getString(clusterExtendInfo, "message"));
        }
        if (CollectionUtils.isEmpty(clusterInstanceInfo.getNodes())) {
            log.error("Virtual IP is error.");
            throw new LegoCheckedException(PostgreErrorCode.SERVICE_IP_IS_INVALID, "Virtual IP is error.");
        }
        return clusterInstanceInfo.getNodes().stream().collect(Collectors.groupingBy(NodeInfo::getEndpoint));
    }

    private void buildClusterNode(ProtectedResource resource, ProtectedResource childNode,
        Map<String, List<NodeInfo>> appEnvMap) {
        String agentUuid = childNode.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
        log.info("Build ClusterNodeRole, agentUuid: {}", agentUuid);
        ProtectedEnvironment child = protectedEnvironmentService.getEnvironmentById(agentUuid);
        String agentIpList = child.getExtendInfoByKey(ResourceConstants.AGENT_IP_LIST);
        boolean canFind = false;
        for (String endPoint : appEnvMap.keySet()) {
            if (agentIpList.contains(endPoint)) {
                NodeInfo nodeInfo = appEnvMap.get(endPoint).get(IsmNumberConstant.ZERO);
                Map<String, String> extendInfo = nodeInfo.getExtendInfo();
                resource.setVersion(StringUtils.trim(extendInfo.get(DatabaseConstants.VERSION)));
                String role = StringUtils.equals(MapUtils.getString(extendInfo, PostgreConstants.IS_PRIMARY),
                    PostgreConstants.PRIMARY) ? PostgreConstants.PRIMARY : PostgreConstants.SLAVE;
                childNode.getExtendInfo().put(DatabaseConstants.ROLE, role);
                String clientPath = MapUtils.getString(extendInfo, PostgreConstants.PG_BIN_PATH);
                clientPath = clientPath.replace("/bin", "");
                if (StringUtils.endsWith(clientPath, "/")) {
                    clientPath = clientPath.substring(0, clientPath.length() - 1);
                }
                childNode.getExtendInfo().put(PostgreConstants.DB_INSTALL_PATH_KEY, clientPath);
                childNode.getExtendInfo()
                    .put(DatabaseConstants.DATA_DIRECTORY, MapUtils.getString(extendInfo, PostgreConstants.PG_DATA));
                childNode.getExtendInfo().put(DatabaseConstants.SERVICE_IP, nodeInfo.getEndpoint());
                childNode.getExtendInfo()
                    .put(DatabaseConstants.INSTANCE_PORT, MapUtils.getString(extendInfo, DatabaseConstants.PORT));
                childNode.getExtendInfo()
                    .put(PostgreConstants.ARCHIVE_DIR,
                        MapUtils.getString(resource.getExtendInfo(), PostgreConstants.ARCHIVE_DIR));
                canFind = true;
                break;
            }
        }
        if (!canFind) {
            log.error("Select server is error");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Select server is error");
        }
    }

    private void checkCluster(ProtectedResource resource) {
        List<ProtectedResource> children = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        List<ProtectedResource> childrenCopy = new ArrayList<>(children);
        // 校验集群实例
        for (ProtectedResource child : children) {
            childrenCopy.remove(child);
            childrenCopy.add(0, child);
            resource.getDependencies().put(DatabaseConstants.CHILDREN, childrenCopy);
            AgentBaseDto checkResult = instanceResourceService.checkClusterInstance(resource);
            if (Long.parseLong(checkResult.getErrorCode()) != DatabaseConstants.SUCCESS_CODE) {
                JSONObject jsonObject = JSONObject.fromObject(checkResult.getErrorMessage());
                throw new LegoCheckedException(Long.parseLong(jsonObject.getString("bodyErr")),
                    jsonObject.getString("message"));
            }
            setPostgreClusterInstanceVersion(resource, checkResult);
        }
        resource.getDependencies().put(DatabaseConstants.CHILDREN, children);
        resource.setPath(resource.getEnvironment().getEndpoint());
        try {
            instanceResourceService.setClusterInstanceNodeRole(resource);
        } catch (LegoCheckedException | NullPointerException e) {
            log.error("Create postgre cluster instance check error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
        // 如果没有主节点直接报错
        Optional<ProtectedResource> childOptional = children.stream()
            .filter(
                item -> StringUtils.equals(PostgreConstants.PRIMARY, item.getExtendInfo().get(DatabaseConstants.ROLE)))
            .findFirst();
        if (!childOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
    }

    private void checkClupCluster(ProtectedResource resource) {
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(resource.getParentUuid());
        List<ProtectedResource> clupServers = environment.getDependencies().get(PostgreConstants.CLUP_SERVERS);
        ProtectedEnvironment clupServerEnvironment = protectedEnvironmentService.getEnvironmentById(
            clupServers.get(IsmNumberConstant.ZERO).getUuid());
        AgentBaseDto checkResult = checkClusterInstance(resource, clupServerEnvironment);
        if (Long.parseLong(checkResult.getErrorCode()) != DatabaseConstants.SUCCESS_CODE) {
            JSONObject jsonObject = JSONObject.fromObject(checkResult.getErrorMessage());
            throw new LegoCheckedException(Long.parseLong(jsonObject.getString("bodyErr")),
                jsonObject.getString("message"));
        }
        setPostgreClusterInstanceVersion(resource, checkResult);
        resource.setPath(resource.getEnvironment().getEndpoint());
        try {
            Map<String, List<NodeInfo>> appEnvMap = getClupClusterInfo(resource, environment);
            resource.getDependencies()
                .get(DatabaseConstants.CHILDREN)
                .forEach(childNode -> buildClusterNodeRole(childNode, appEnvMap));
        } catch (LegoCheckedException | NullPointerException e) {
            log.error("Create postgre cluster instance check error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
        List<ProtectedResource> children = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        // 如果没有主节点直接报错
        Optional<ProtectedResource> childOptional = children.stream()
            .filter(
                item -> StringUtils.equals(PostgreConstants.PRIMARY, item.getExtendInfo().get(DatabaseConstants.ROLE)))
            .findFirst();
        if (!childOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    private void setPostgreClusterInstanceVersion(ProtectedResource resource, AgentBaseDto checkResult) {
        Map<String, String> messageMap = JSONObject.fromObject(checkResult.getErrorMessage()).toMap(String.class);
        resource.setVersion(messageMap.get(DatabaseConstants.VERSION));
    }

    private AgentBaseDto checkClusterInstance(ProtectedResource resource, ProtectedEnvironment environment) {
        AppEnv appEnv = BeanTools.copy(environment, AppEnv::new);
        Map<String, String> extendInfo = Optional.ofNullable(appEnv.getExtendInfo()).orElseGet(HashMap::new);
        extendInfo.put(DatabaseConstants.ALL_NODES, getAllNodes(resource));
        extendInfo.putAll(resource.getExtendInfo());
        appEnv.setExtendInfo(extendInfo);
        appEnv.setSubType(resource.getEnvironment().getSubType());
        Application application = BeanTools.copy(environment, Application::new);
        application.setSubType(resource.getSubType());
        if (VerifyUtil.isEmpty(application.getAuth())) {
            application.setAuth(resource.getAuth());
        }
        return checkClusterInstance(appEnv, application);
    }

    private String getAllNodes(ProtectedResource resource) {
        return resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(this::getIpAndPort)
            .collect(Collectors.joining(DatabaseConstants.SPLIT_CHAR));
    }

    private String getIpAndPort(ProtectedResource subInstance) {
        return subInstance.getExtendInfoByKey(DatabaseConstants.SERVICE_IP) + DatabaseConstants.IP_PORT_SPLIT_CHAR
            + subInstance.getExtendInfoByKey(DatabaseConstants.INSTANCE_PORT);
    }

    private AgentBaseDto checkClusterInstance(AppEnv appEnv, Application application) {
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(appEnv);
        checkAppReq.setApplication(application);
        AgentBaseDto agentBaseDto;
        try {
            agentBaseDto = agentUnifiedService.check(application.getSubType(), appEnv.getEndpoint(), appEnv.getPort(),
                checkAppReq);
        } catch (LegoCheckedException e) {
            long errorCode = CommonErrorCode.AGENT_NETWORK_ERROR;
            if (!VerifyUtil.isEmpty(e.getMessage())) {
                ActionResult actionResult = JSONObject.toBean(e.getMessage(), ActionResult.class);
                errorCode = Long.parseLong(actionResult.getBodyErr());
            }
            log.error("Cluster instance check fail. name: {}", appEnv.getName());
            throw new LegoCheckedException(errorCode, "Cluster instance check fail.");
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Cluster instance check fail. name: {}", appEnv.getName());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Cluster instance check fail.");
        }
        return agentBaseDto;
    }

    private void buildClusterNodeRole(ProtectedResource childNode, Map<String, List<NodeInfo>> appEnvMap) {
        NodeInfo nodeInfo = appEnvMap.get(childNode.getExtendInfoByKey(DatabaseConstants.SERVICE_IP))
            .get(IsmNumberConstant.ZERO);
        Map<String, String> extendInfo = nodeInfo.getExtendInfo();
        String role = StringUtils.equals(MapUtils.getString(extendInfo, PostgreConstants.IS_PRIMARY),
            PostgreConstants.PRIMARY) ? PostgreConstants.PRIMARY : PostgreConstants.SLAVE;
        childNode.getExtendInfo().put(DatabaseConstants.ROLE, role);
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("Start update postgre cluster instance check. resource name: {}", resource.getName());
        resource.setEnvironment(protectedEnvironmentService.getEnvironmentById(resource.getParentUuid()));
        resource.getExtendInfo().put(DatabaseConstants.VIRTUAL_IP, resource.getEnvironment().getEndpoint());
        String installDeployType = resource.getExtendInfo().get(PostgreConstants.INSTALL_DEPLOY_TYPE);
        if (StringUtils.equals(installDeployType, PostgreConstants.CLUP)) {
            checkClupCluster(resource);
        } else {
            // check实例端口是否被修改
            instanceResourceService.checkClusterInstancePortIsChanged(resource);
            checkCluster(resource);
        }
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End update postgre cluster instance check. resource name: {}", resource.getName());
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.equalsSubType(protectedResource.getSubType());
    }
}
