/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.opengauss.resources.access.provider;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.enums.NodeType;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.opengauss.resources.access.enums.OpenGaussClusterStateEnum;
import openbackup.opengauss.resources.access.util.OpenGaussClusterUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.RequestUriUtil;
import openbackup.system.base.util.StreamUtil;

import com.fasterxml.jackson.core.type.TypeReference;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.net.URI;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * openGauss环境注册提供者
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-08
 */
@Component
@Slf4j
public class OpenGaussDatabaseEnvironmentProvider extends DatabaseEnvironmentProvider {
    private final ResourceService resourceService;

    private final JsonSchemaValidator jsonSchemaValidator;

    private ProtectedEnvironmentService environmentService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * 构造器注入
     *
     * @param providerManager providerManager
     * @param pluginConfigManager pluginConfigManager
     * @param resourceService resourceService
     * @param jsonSchemaValidator jsonSchemaValidator
     * @param agentUnifiedService agentUnifiedService
     */
    public OpenGaussDatabaseEnvironmentProvider(ProviderManager providerManager,
        PluginConfigManager pluginConfigManager, ResourceService resourceService,
        JsonSchemaValidator jsonSchemaValidator, AgentUnifiedService agentUnifiedService) {
        super(providerManager, pluginConfigManager);
        this.resourceService = resourceService;
        this.jsonSchemaValidator = jsonSchemaValidator;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Autowired
    public void setEnvironmentService(ProtectedEnvironmentService environmentService) {
        this.environmentService = environmentService;
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        log.info("scan open gauss environment uuid:{}, name: {}", environment.getUuid(), environment.getName());
        List<ProtectedResource> agentsResourceList = environment.getDependencies().get(DatabaseConstants.AGENTS);
        return queryProtectedResourcesByPrimaryNode(environment, agentsResourceList);
    }

    private List<ProtectedResource> queryProtectedResourcesByPrimaryNode(ProtectedEnvironment environment,
        List<ProtectedResource> agentsResourceList) {
        if (ObjectUtils.isEmpty(agentsResourceList)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Environment dependencies agent is empty.");
        }
        log.info("environment agent size: {}", agentsResourceList.size());
        // 单机的场景
        if (agentsResourceList.size() == OpenGaussConstants.SINGLE_NODE) {
            return getProtectedResources(agentsResourceList.get(0), environment);
        }
        // 集群场景，寻找主节点下发扫描请求
        List<NodeInfo> nodeInfos = JsonUtil.read(environment.getExtendInfo().get(OpenGaussConstants.NODES),
            new TypeReference<List<NodeInfo>>() {
            });
        Optional<String> optionalPrimaryNodeName = nodeInfos.stream()
            .filter(nodeInfo -> NodeType.MASTER.getNodeType()
                .equals(nodeInfo.getExtendInfo().get(OpenGaussConstants.CLUSTER_NODE_ROLE)))
            .map(NodeInfo::getName)
            .findAny();
        Optional<ProtectedResource> primaryAgentsResource = Optional.empty();
        if (optionalPrimaryNodeName.isPresent()) {
            primaryAgentsResource = agentsResourceList.stream()
                .filter(agentProtectResource -> optionalPrimaryNodeName.get().equals(agentProtectResource.getName()))
                .findFirst();
        }
        if (!primaryAgentsResource.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "No available agent information was found.");
        }
        return getProtectedResources(primaryAgentsResource.get(), environment);
    }

    private List<ProtectedResource> getProtectedResources(ProtectedResource agentResource,
        ProtectedEnvironment environment) {
        Optional<Endpoint> optionalEndpoint = getEndpoint(agentResource.getUuid());
        if (!optionalEndpoint.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Agent is empty");
        }
        Endpoint endpoint = optionalEndpoint.get();
        ListResourceReq openGaussRequest = new ListResourceReq();
        openGaussRequest.setAppEnv(BeanTools.copy(agentResource, AppEnv::new));
        openGaussRequest.setApplication(BeanTools.copy(environment, Application::new));
        URI requestUri = RequestUriUtil.getRequestUri(endpoint.getIp(), endpoint.getPort());
        String appType = ResourceSubTypeEnum.OPENGAUSS.getType();
        AgentDetailDto response = agentUnifiedService.getDetail(appType, endpoint.getIp(), endpoint.getPort(),
            openGaussRequest);
        return convertProtectedResources(agentResource, requestUri, response, environment.getEndpoint());
    }

    private List<ProtectedResource> convertProtectedResources(ProtectedResource agent, URI requestUri,
        AgentDetailDto response, String endpoint) {
        if (VerifyUtil.isEmpty(response)) {
            log.error("Response is empty, agentId: {}, uri: {}, response: {}", agent.getUuid(), requestUri, response);
            return Collections.emptyList();
        }
        if (!VerifyUtil.isEmpty(response.getErrorCode()) && Integer.parseInt(response.getErrorCode()) != 0) {
            log.error("Agent return error, agentId: {}, uri: {}, code: {}, msg: {}", agent.getUuid(), requestUri,
                response.getErrorCode(), response.getErrorMessage());
            throw new LegoCheckedException(Long.parseLong(response.getErrorCode()), response.getErrorMessage());
        }
        List<AppResource> resources = response.getResourceList();

        if (VerifyUtil.isEmpty(resources)) {
            log.info("OpenGauss resource is empty");
            return Collections.emptyList();
        }
        List<ProtectedResource> protectedResourceList = resources.stream()
            .map(appResource -> getProtectedResource(endpoint, appResource))
            .collect(Collectors.toList());
        log.info("OpenGauss scan protectedResourceList: {} successfully.", protectedResourceList.size());
        return protectedResourceList;
    }

    private ProtectedResource getProtectedResource(String endpoint, AppResource appResource) {
        ProtectedResource protectedResource = appResource.castToProtectedResource();
        // 设置path路径，不设置复制的时候会报错
        protectedResource.setPath(endpoint);
        return protectedResource;
    }

    private Optional<Endpoint> getEndpoint(String agentId) {
        Optional<ProtectedResource> optResource = resourceService.getResourceById(agentId);
        return optResource.filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()));
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.OPENGAUSS.equalsSubType(resourceSubType);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("Preparing to check the environment, environment uuid: {}.", environment.getUuid());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(environment.getName());
        this.checkClusterCount();
        // 检验环境节点是否已经离线
        environment.getDependencies().get(DatabaseConstants.AGENTS).forEach(this::checkNodeOnline);

        jsonSchemaValidator.doValidate(environment, ResourceSubTypeEnum.OPENGAUSS.getType());
        Map<String, String> extendInfo = Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(OpenGaussConstants.GUI_NODES,
            JSONObject.writeValueAsString(getEndpointFromEnvironment(environment)));
        environment.setExtendInfo(extendInfo);

        ResourceCheckContext resourceCheckContext = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            environment).checkConnection(environment);

        // 检查集群状态
        AppEnvResponse clusterInfo = OpenGaussClusterUtil.getContextClusterInfo(resourceCheckContext);

        // uuid为空，注册集群
        if (StringUtils.isEmpty(environment.getUuid())) {
            String environmentId = generateUniqueUuid(clusterInfo, environment.getAuth().getAuthKey(),
                environment.getDependencies().get(DatabaseConstants.AGENTS));
            log.info("open gauss environment name: {}, environmentId :{}", environment.getName(), environmentId);
            Optional<ProtectedResource> resourceCluster = resourceService.getResourceById(environmentId);
            if (resourceCluster.isPresent()) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                    "The selected node has been registered.");
            }
            environment.setUuid(environmentId);
            List<String> endpoints = clusterInfo.getNodes()
                .stream()
                .filter(Objects::nonNull)
                .map(NodeInfo::getEndpoint)
                .filter(Objects::nonNull)
                .collect(Collectors.toList());
            if (CollectionUtils.isEmpty(endpoints)) {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    "The cluster endpoint address does not exist.");
            }
            // 设置环境的endpoint，创建保保护的时候，会校验环境的ip信息，不传会报错。
            environment.setEndpoint(endpoints.get(0));
        }
        environment.setLinkStatus(String.valueOf(LinkStatusEnum.ONLINE.getStatus()));
        // 更新环境的扩展信息
        OpenGaussClusterUtil.buildProtectedEnvironment(environment, clusterInfo);
    }

    private void checkClusterCount() {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.OPENGAUSS.getType());
        List<ProtectedResource> environments = new ArrayList<>();
        int num = OpenGaussConstants.OPENGAUSS_CLUSTER_MAX_COUNT / DatabaseConstants.PAGE_SIZE;
        for (int size = 0; size < num; size++) {
            List<ProtectedResource> resources = resourceService.query(size, DatabaseConstants.PAGE_SIZE, conditions)
                .getRecords();
            environments.addAll(resources);
        }
        if (environments.size() >= OpenGaussConstants.OPENGAUSS_CLUSTER_MAX_COUNT) {
            log.error("OpenGauss resource register exceed max count: {}.",
                OpenGaussConstants.OPENGAUSS_CLUSTER_MAX_COUNT);
            throw new LegoCheckedException(DatabaseErrorCode.RESOURCE_REACHED_THE_UPPER_LIMIT,
                new String[] {String.valueOf(OpenGaussConstants.OPENGAUSS_CLUSTER_MAX_COUNT)},
                "OpenGauss resource register exceed max count.");
        }
    }

    private void checkNodeOnline(ProtectedResource protectedResource) {
        ProtectedEnvironment env = environmentService.getEnvironmentById(protectedResource.getUuid());
        if (LinkStatusEnum.OFFLINE.getStatus()
            .toString()
            .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env))) {
            log.error("host is offline, host id: {} ", env.getUuid());
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, new String[] {},
                "Protected environment is offLine!");
        }
    }

    private List<Endpoint> getEndpointFromEnvironment(ProtectedEnvironment environment) {
        List<ProtectedResource> agentsResourceList = environment.getDependencies().get(DatabaseConstants.AGENTS);
        if (ObjectUtils.isEmpty(agentsResourceList)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Environment dependencies agent is empty.");
        }
        List<Endpoint> endpointList = new ArrayList<>();
        for (ProtectedResource agentResource : agentsResourceList) {
            Endpoint endpoint = getEndpoint(agentResource.getUuid()).orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Agent is empty"));
            endpointList.add(endpoint);
        }
        return endpointList;
    }

    /**
     * 集群id生成规则：OpenGauss + ip地址排序 + 用户名代表集群的唯一id
     *
     * @param clusterInfo 集群信息
     * @param authKey 认证名字 环境的认证信息Authentication.authKey
     * @param protectedResources protectedResources
     * @return 集群环境对应的唯一uuid
     */
    private String generateUniqueUuid(AppEnvResponse clusterInfo, String authKey,
        List<ProtectedResource> protectedResources) {
        String agentUuids = protectedResources.stream()
            .map(ResourceBase::getUuid)
            .sorted()
            .collect(Collectors.joining(";"));
        String clusterIps = clusterInfo.getNodes()
            .stream()
            .map(NodeInfo::getEndpoint)
            .sorted()
            .collect(Collectors.joining(";"));
        String systemId = clusterInfo.getExtendInfo().get(OpenGaussConstants.SYSTEM_ID);
        log.info("openGauss clusterIps: {}, systemId: {}, agentUuids:{}", clusterIps, systemId, agentUuids);
        String envIdentity = ResourceSubTypeEnum.OPENGAUSS.getType() + systemId + clusterIps + authKey + agentUuids;
        return UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString();
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        log.info("The healthCheck of the open gauss start, uuid: {},", environment.getUuid());
        // 校验主机状态，跟新集群和node节点信息
        checkHostStatus(environment);
        // 收集资源的上下文信息
        ResourceCheckContext resourceCheckContext = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            environment).tryCheckConnection(environment);

        // 先更新集群扩展信息，再检验上下文的错误信息和集群信息，页面才能实时拿到更新状态信息
        AppEnvResponse clusterInfo = OpenGaussClusterUtil.getContextClusterInfo(resourceCheckContext);

        ProtectedEnvironment newEnvironment = new ProtectedEnvironment();
        newEnvironment.setUuid(environment.getUuid());
        ProtectedEnvironment newEnv = OpenGaussClusterUtil.buildProtectedEnvironment(newEnvironment, clusterInfo);
        resourceService.updateSourceDirectly(Stream.of(newEnv).collect(Collectors.toList()));

        // 检查CheckContext错误信息
        ResourceCheckContextUtil.check(resourceCheckContext, "check connection failed.");

        // 检查集群状态，抛出异常，返回给框架更新linkStatus为离线，发送告警信息
        OpenGaussClusterUtil.checkClusterState(clusterInfo);
    }

    private void checkHostStatus(ProtectedEnvironment environment) {
        // 获取离线的主机信息
        List<ProtectedEnvironment> offlineAgentsEnvironments = environment.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .filter(env -> LinkStatusEnum.OFFLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env)))
            .collect(Collectors.toList());
        if (offlineAgentsEnvironments.size() > 0) {
            List<String> offlineAgentsEnvironmentsEndpoint = offlineAgentsEnvironments.stream()
                .map(ProtectedEnvironment::getEndpoint)
                .collect(Collectors.toList());
            log.info("Information about offline environment endpoint: {}", offlineAgentsEnvironmentsEndpoint);
            handleHostOfflineScenario(environment, offlineAgentsEnvironments);
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE,
                "host is offline,update the environment status to Offline");
        }
    }

    private void handleHostOfflineScenario(ProtectedEnvironment environment,
        List<ProtectedEnvironment> offlineAgentsEnvironments) {
        // 主机离线，更新集群状态为离线
        Map<String, String> extendInfo = environment.getExtendInfo();
        extendInfo.put(OpenGaussConstants.CLUSTER_STATE, OpenGaussClusterStateEnum.UNAVAILABLE.getState());
        List<NodeInfo> nodeInfos = JsonUtil.read(extendInfo.get(OpenGaussConstants.NODES),
            new TypeReference<List<NodeInfo>>() {
            });
        // 主机离线，更新对应的主机状态为离线
        for (ProtectedEnvironment offlineAgentEnvironment : offlineAgentsEnvironments) {
            Optional<NodeInfo> nodeInfo = nodeInfos.stream()
                .filter(agent -> agent.getName().equals(offlineAgentEnvironment.getName()))
                .findFirst();
            nodeInfo.ifPresent(node -> {
                Map<String, String> nodeExtendInfo = node.getExtendInfo();
                nodeExtendInfo.put(OpenGaussConstants.STATUS, OpenGaussClusterStateEnum.UNAVAILABLE.getState());
                node.setExtendInfo(nodeExtendInfo);
            });
        }
        extendInfo.put(OpenGaussConstants.NODES, JSONObject.writeValueAsString(nodeInfos));
        // 只更新需要使用的数据，不要全部更新环境信息
        ProtectedEnvironment newProtectedEnv = new ProtectedEnvironment();
        newProtectedEnv.setUuid(environment.getUuid());
        newProtectedEnv.setExtendInfo(extendInfo);
        resourceService.updateSourceDirectly(Stream.of(newProtectedEnv).collect(Collectors.toList()));
    }
}
