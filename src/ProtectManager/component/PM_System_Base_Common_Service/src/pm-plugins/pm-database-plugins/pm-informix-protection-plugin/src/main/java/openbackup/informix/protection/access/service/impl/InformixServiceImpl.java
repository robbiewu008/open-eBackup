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
package openbackup.informix.protection.access.service.impl;

import openbackup.access.framework.resource.service.provider.UnifiedEnvironmentCheckProvider;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionRejectException;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.informix.protection.access.constant.InformixConstant;
import openbackup.informix.protection.access.service.InformixService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicReference;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * InformixServiceImpl
 *
 */
@Service
@Slf4j
public class InformixServiceImpl implements InformixService {
    private static final String LOG_PATH_PATTERN = "^(/[^/]+/?)$";

    private static final String CHECK_SPECIAL_PATTERN = "^(/mnt/databackup|/opt/DataBackup)$";

    private static final String FILE_SEPARATOR = "/";

    private final InstanceProtectionService instanceProtectionService;

    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final ProviderManager providerManager;

    private final UnifiedEnvironmentCheckProvider unifiedCheckProvider;

    private final ClusterEnvironmentService clusterEnvironmentService;

    @Autowired
    private DmeUnifiedRestApi dmeUnifiedRestApi;

    @Autowired
    private JobService jobService;

    /**
     * InformixServiceImpl
     *
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param providerManager providerManager
     * @param instanceProtectionService instanceProtectionService
     * @param environmentServices environmentServices
     */
    public InformixServiceImpl(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
        ProviderManager providerManager, InstanceProtectionService instanceProtectionService,
        EnvironmentServices environmentServices) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.providerManager = providerManager;
        this.unifiedCheckProvider = environmentServices.getUnifiedCheckProvider();
        this.clusterEnvironmentService = environmentServices.getClusterEnvironmentService();
        this.instanceProtectionService = instanceProtectionService;
    }

    @Override
    public ProtectedEnvironment queryAgentEnvironment(ProtectedResource instance) {
        List<ProtectedResource> agentResources = instance.getDependencies().get(DatabaseConstants.AGENTS);
        if (VerifyUtil.isEmpty(agentResources)) {
            log.error("Single instance dependency agent is empty.");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Single instance dependency agent is empty.");
        }
        String agentUuid = agentResources.get(0).getUuid();
        return getEnvironmentById(agentUuid);
    }

    @Override
    public void doClusterInstanceAction(ProtectedResource resource, boolean isRegister) {
        // 获取调用插件的返回值
        List<PageListResponse<ProtectedResource>> responsesList = getResponsesList(resource, isRegister);
        // 一致性检查并返回extendInfo信息
        Map<String, Map<String, String>> clusterExtendInfo = getClusterExtendInfo(responsesList);
        // 更新插件返回值
        updateResource(resource, clusterExtendInfo);
    }

    @Override
    public void doSingleInstanceAction(ProtectedResource resource, boolean isRegister) {
        if (isRegister) {
            // 校验实例已经注册
            checkInstanceExist(resource);
        }
        ProtectedResource copyResource = BeanTools.copy(resource, new ProtectedResource());
        // 更新resource
        updateResource(copyResource);
        // 连通性检查
        checkInstanceConnection(resource);
        // 注册实例
        registerSingleInstance(resource, copyResource);
    }

    private void updateResource(ProtectedResource copyResource) {
        Map<String, List<ProtectedResource>> dependencies = copyResource.getDependencies();
        ProtectedResource clusterResource =
                getResourceById(dependencies.get(DatabaseConstants.AGENTS).get(0).getUuid());
        String clusterName = clusterResource.getName();
        copyResource.getExtendInfo().put(InformixConstant.CLUSTER_NAME, clusterName);
        String hostId = clusterResource.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(hostId);
        dependencies.put(DatabaseConstants.AGENTS, Collections.singletonList(protectedResource));
    }

    @Override
    public void checkLogBackupItem(ProtectedEnvironment environment) {
        // 校验日志备份，未开启则无需校验；开启需要校验日志备份路径是不是合法
        Map<String, String> extendInfo = environment.getExtendInfo();
        if (InformixConstant.LOG_BACKUP_OFF.equals(extendInfo.getOrDefault(InformixConstant.LOG_BACKUP, ""))) {
            log.info("The logBackup is off.");
            return;
        }
        String logBackupPath = extendInfo.getOrDefault(InformixConstant.LOG_BACKUP_PATH, "");
        if (isPathInvalid(logBackupPath)) {
            throw new DataProtectionRejectException(CommonErrorCode.FILE_PATH_VALIDATE_FAILED, new String[]{},
                    "The logBackupPath of protected environment is wrong!");
        }
    }

    private boolean isPathInvalid(String logBackupPath) {
        if (StringUtils.isEmpty(logBackupPath) || FILE_SEPARATOR.equals(logBackupPath)) {
            return true;
        }

        boolean isNormalPath = Pattern.compile(InformixConstant.LOG_BACKUP_PATH_PATTERN).matcher(logBackupPath).find();
        boolean isRootPath = Pattern.compile(LOG_PATH_PATTERN).matcher(logBackupPath).find();
        boolean isSpecialPath = Pattern.compile(CHECK_SPECIAL_PATTERN).matcher(logBackupPath).find();
        return !isNormalPath || isRootPath || isSpecialPath;
    }

    @Override
    public void checkHostInfo(ProtectedEnvironment environment) {
        List<ProtectedResource> agentsList = environment.getDependencies().get(DatabaseConstants.AGENTS);
        if (agentsList.size() != 1) {
            throw new DataProtectionRejectException(CommonErrorCode.AGENT_MISMATCH_NODE, new String[]{},
                    "Protected environment does not exist!");
        }
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            log.info("start to register Informix single instance host check.");
            // 获取当前已注册的服务主机信息,校验主机是否已经注册过，不允许注册已经注册过的主机信息
            clusterEnvironmentService.checkRegisterNodeIsRegistered(environment);
        } else {
            log.info("start to update informix cluster preCheck. uuid:{}", environment.getUuid());
        }
        // 校验主机状态是否在线，要求状态在线，不在线则抛出异常
        agentsList.forEach(agent -> checkNodeOnline(environment, agent));
    }

    private void checkNodeOnline(ProtectedEnvironment environment, ProtectedResource protectedResource) {
        ProtectedEnvironment env = getEnvironmentById(protectedResource.getUuid());
        if (LinkStatusEnum.OFFLINE.getStatus().toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env))) {
            throw new DataProtectionAccessException(CommonErrorCode.HOST_OFFLINE, new String[]{},
                    "Protected environment is offLine!");
        }
        environment.setEndpoint(env.getEndpoint());
        environment.setPort(env.getPort());
        environment.setAuth(env.getAuth());
        environment.setPath(Optional.ofNullable(env.getPath()).orElse(env.getEndpoint()));
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Override
    public PageListResponse<ProtectedResource> listResource(ProtectedResource resource, ProtectedEnvironment host,
        String type) {
        PageListResponse<ProtectedResource> response = agentUnifiedService.getDetailPageList(
                type, host.getEndpoint(), host.getPort(), generateListResourceV2Req(host, resource));
        if (Objects.isNull(response) || response.getRecords() == null || response.getRecords().size() < 1) {
            log.warn("Network connection timeout. ip: {}, port: {}", host.getEndpoint(), host.getPort());
            throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT,
                    "Network connection timeout.");
        }
        return response;
    }

    @Override
    public void checkInstanceExist(ProtectedResource resource) {
        Map<String, Object> cons = new HashMap<String, Object>() {{
            put(DatabaseConstants.RESOURCE_TYPE, resource.getType());
            put(DatabaseConstants.SUB_TYPE, resource.getSubType());
            put(InformixConstant.HOST_ID, resource.getParentUuid());
            put(DatabaseConstants.NAME, resource.getName());
        }};
        if (resourceService.query(0, 1, cons).getTotalCount() != 0) {
            log.error("Informix instance already exists.");
            throw new DataProtectionAccessException(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED, new String[] {},
                    "Informix instance already exists.");
        }
    }

    @Override
    public void updateResource(ProtectedResource resource, Map<String, Map<String, String>> clusterExtendInfo) {
        Map<String, String> map = Optional.ofNullable(resource.getExtendInfo()).orElse(new HashMap<>());
        map.put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        map.put(DatabaseConstants.IS_TOP_INSTANCE, DatabaseConstants.TOP_INSTANCE);
        resource.setExtendInfo(map);

        AtomicReference<String> clusterNameReference = new AtomicReference<>("");
        List<String> endpoints = Lists.newArrayList();
        resource.getDependencies().get(DatabaseConstants.CHILDREN).forEach(child -> {
            String uuid = child.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
            Map<String, String> extendInfoMap = clusterExtendInfo.get(uuid);
            child.setExtendInfo(getNewExtendInfo(extendInfoMap, child.getExtendInfo()));
            resource.setVersion(extendInfoMap.get(InformixConstant.APPLICATION_VERSION));
            child.getExtendInfo().put(InformixConstant.HOST_ID, child.getParentUuid());

            ProtectedResource protectedResource = getResourceById(uuid);
            clusterNameReference.set(clusterNameReference + protectedResource.getName() + InformixConstant.COMMA);
            endpoints.add(protectedResource.getEndpoint());
            child.setPath(protectedResource.getEndpoint());
        });

        String clusterName = clusterNameReference.get();
        clusterName = clusterName.contains(InformixConstant.COMMA)
                ? clusterName.substring(0, clusterName.lastIndexOf(InformixConstant.COMMA)) : "";
        resource.getExtendInfo().put(InformixConstant.CLUSTER_NAME, clusterName);
        resource.setPath(String.join(DatabaseConstants.SPLIT_CHAR, endpoints));
    }

    private Map<String, String> getNewExtendInfo(Map<String, String> extendInfoMap, Map<String, String> extendInfo) {
        extendInfo.put(InformixConstant.APPLICATION_VERSION, extendInfoMap.get(InformixConstant.APPLICATION_VERSION));
        extendInfo.put(InformixConstant.PAIRED_SERVER_IP, extendInfoMap.get(InformixConstant.PAIRED_SERVER_IP));
        extendInfo.put(InformixConstant.SERVER_NUM, extendInfoMap.get(InformixConstant.SERVER_NUM));
        extendInfo.put(InformixConstant.PAIRED_SERVER, extendInfoMap.get(InformixConstant.PAIRED_SERVER));
        extendInfo.put(InformixConstant.LOCAL_SERVER, extendInfoMap.get(InformixConstant.LOCAL_SERVER));
        extendInfo.put(InformixConstant.INSTANCESTATUS, extendInfoMap.get(InformixConstant.INSTANCESTATUS));
        extendInfo.put(InformixConstant.ROOT_DBS_PATH, extendInfoMap.get(InformixConstant.ROOT_DBS_PATH));
        return extendInfo;
    }

    @Override
    public List<PageListResponse<ProtectedResource>> getResponsesList(ProtectedResource resource, boolean isRegister) {
        ProtectedEnvironment parentEnvironment = resource.getEnvironment();
        List<PageListResponse<ProtectedResource>> responsesList = new ArrayList<>();
        resource.getDependencies().get(DatabaseConstants.CHILDREN).forEach(childResource -> {
            childResource.setSubType(ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType());
            childResource.setEnvironment(parentEnvironment);
            List<ProtectedResource> agentsList = childResource.getDependencies().get(DatabaseConstants.AGENTS);
            ProtectedResource agentResource = getAgentResource(childResource);
            agentsList.clear();
            agentsList.add(agentResource);
            if (isRegister) {
                // 校验实例已经注册，任一实例被注册过，则不允许注册该集群实例
                checkInstanceExist(childResource);
            }
            // 连通性检查
            checkInstanceConnection(childResource);
            // 注册实例
            ProtectedResource protectedResource = childResource.getDependencies().get(DatabaseConstants.AGENTS).get(0);
            ProtectedEnvironment host = getEnvironmentById(protectedResource.getUuid());
            PageListResponse<ProtectedResource> response = listResource(childResource, host, resource.getSubType());
            response.getRecords().get(0).getExtendInfo().put(InformixConstant.AGENT_IP_LIST,
                    agentResource.getExtendInfo().get(InformixConstant.AGENT_IP_LIST));
            responsesList.add(response);
        });
        return responsesList;
    }

    private ProtectedResource getAgentResource(ProtectedResource childResource) {
        List<ProtectedResource> agentsList = childResource.getDependencies().get(DatabaseConstants.AGENTS);
        if (agentsList.size() != 1) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_NUM_EXCEED_LIMIT,
                    "Informix agents info are wrong!");
        }
        ProtectedResource agentResourceById = getResourceById(agentsList.get(0).getUuid());
        ProtectedResource agent = agentResourceById.getDependencies().get(DatabaseConstants.AGENTS).get(0);
        ProtectedResource resourceById = getResourceById(agent.getUuid());
        String ipList = resourceById.getExtendInfo().get(InformixConstant.AGENT_IP_LIST);
        agentResourceById.getExtendInfo().put(InformixConstant.AGENT_IP_LIST, ipList);
        return agentResourceById;
    }

    private static ListResourceV2Req generateListResourceV2Req(ProtectedEnvironment environment,
        ProtectedResource resource) {
        ListResourceV2Req listResourceV2Req = new ListResourceV2Req();
        listResourceV2Req.setPageSize(1);
        listResourceV2Req.setPageNo(0);
        listResourceV2Req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        listResourceV2Req.setApplications(Lists.newArrayList(BeanTools.copy(resource, Application::new)));
        return listResourceV2Req;
    }

    @Override
    public ProtectedEnvironment getEnvironmentById(String envId) {
        return resourceService.getResourceById(envId)
                .filter(env -> env instanceof ProtectedEnvironment)
                .map(env -> (ProtectedEnvironment) env)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                        "Protected environment is not exists!"));
    }

    @Override
    public void checkInstanceConnection(ProtectedResource resource) {
        // 校验连通性
        ResourceConnectionCheckProvider provider = providerManager.findProvider(
                ResourceConnectionCheckProvider.class, resource);
        ResourceCheckContext context = provider.tryCheckConnection(resource);
        if (VerifyUtil.isEmpty(context.getActionResults())) {
            log.error("INFORMIX instance check connection result is empty. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check connection result is empty.");
        }
        ActionResult actionResult = context.getActionResults().get(IsmNumberConstant.ZERO);
        if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
            log.error("INFORMIX instance check connection failed. name: {}", resource.getName());
            throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), "check connection failed.");
        }
    }

    @Override
    public void checkServiceConnection(ProtectedEnvironment environment) {
        log.info("Start to register service. Resource name : {}, SubType : {}",
                environment.getName(), environment.getSubType());
        EnvironmentCheckProvider provider = providerManager.findProviderOrDefault(EnvironmentCheckProvider.class,
                environment, unifiedCheckProvider);
        provider.check(environment);
    }

    @Override
    public Map<String, Map<String, String>> getClusterExtendInfo(
            List<PageListResponse<ProtectedResource>> responsesList) {
        if (responsesList.size() != 2) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Failed to obtain the agents response.");
        }
        checkIfInstanceMatch(responsesList);
        Map<String, Map<String, String>> clusterExtendInfo = new HashMap<>();
        responsesList.forEach(responses -> {
            Optional<ProtectedResource> protectedResource = Optional.ofNullable(responses.getRecords().get(0));
            if (!protectedResource.isPresent()) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Failed to obtain the agents response.");
            }
            clusterExtendInfo.put(protectedResource.get().getUuid(), protectedResource.get().getExtendInfo());
        });
        return clusterExtendInfo;
    }

    @Override
    public void registerSingleInstance(ProtectedResource resource, ProtectedResource copyResource) {
        List<ProtectedResource> protectedResources = copyResource.getDependencies().get(DatabaseConstants.AGENTS);
        if (protectedResources.size() != 1) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "agent params are wrong!");
        }
        ProtectedEnvironment host = getEnvironmentById(protectedResources.get(0).getUuid());
        PageListResponse<ProtectedResource> response = listResource(copyResource, host, copyResource.getSubType());
        Map<String, String> agentExtendInfo = response.getRecords().get(0).getExtendInfo();
        resource.setVersion(agentExtendInfo.get(InformixConstant.APPLICATION_VERSION));
        resource.getExtendInfo().put(InformixConstant.SERVER_NUM, agentExtendInfo.get(InformixConstant.SERVER_NUM));
        resource.getExtendInfo().put(InformixConstant.LOCAL_SERVER, agentExtendInfo.get(InformixConstant.LOCAL_SERVER));
        resource.getExtendInfo().put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        resource.getExtendInfo().put(DatabaseConstants.IS_TOP_INSTANCE, DatabaseConstants.TOP_INSTANCE);
        resource.getExtendInfo().put(InformixConstant.HOST_ID, resource.getParentUuid());
        resource.getExtendInfo().put(InformixConstant.CLUSTER_NAME,
                copyResource.getExtendInfo().get(InformixConstant.CLUSTER_NAME));
        resource.getExtendInfo().put(InformixConstant.APPLICATION_VERSION,
                agentExtendInfo.get(InformixConstant.APPLICATION_VERSION));
        resource.getExtendInfo().put(InformixConstant.ROOT_DBS_PATH,
                agentExtendInfo.get(InformixConstant.ROOT_DBS_PATH));
    }

    /**
     * 检查主备集群实例agent上报数据，判断是否组成主备集群关系
     * isStatusMatch 检查状态是否匹配
     * isVersionMatch 检查informix数据库版本是否匹配
     * isServerMatcher 检查两端服务是否匹配
     * isIpMatcher 检查两端IP是否匹配
     *
     * @param responsesList responsesList
     */
    private void checkIfInstanceMatch(List<PageListResponse<ProtectedResource>> responsesList) {
        Map<String, String> extendInfo1 = responsesList.get(0).getRecords().get(0).getExtendInfo();
        Map<String, String> extendInfo2 = responsesList.get(1).getRecords().get(0).getExtendInfo();
        // 检查是否组成主备集群
        String instanceStatus1 = getTargetValue(extendInfo1, InformixConstant.INSTANCESTATUS);
        String instanceStatus2 = getTargetValue(extendInfo2, InformixConstant.INSTANCESTATUS);

        boolean isStatusMatch = (isMasterNodeMatch(instanceStatus1) && isSecondNodeMatch(instanceStatus2))
                || (isMasterNodeMatch(instanceStatus2) && isSecondNodeMatch(instanceStatus1));

        boolean isVersionMatch = getTargetValue(extendInfo1, InformixConstant.APPLICATION_VERSION)
                .equals(getTargetValue(extendInfo2, InformixConstant.APPLICATION_VERSION));
        boolean isServerMatcher = getTargetValue(extendInfo1, InformixConstant.PAIRED_SERVER)
                .equals(getTargetValue(extendInfo2, InformixConstant.LOCAL_SERVER))
                && getTargetValue(extendInfo1, InformixConstant.LOCAL_SERVER)
                .equals(getTargetValue(extendInfo2, InformixConstant.PAIRED_SERVER));

        if (!isStatusMatch || !isVersionMatch || !isServerMatcher) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_INCONSISTENT,
                    "informix cluster node info check failed.");
        }
    }

    private boolean isMasterNodeMatch(String instanceStatus) {
        return InformixConstant.MASTER_NODE_STATUS.equals(instanceStatus);
    }

    private boolean isSecondNodeMatch(String instanceStatus) {
        return InformixConstant.SECOND_NODE_STATUS.equals(instanceStatus)
                || InformixConstant.UPDATABLE_SEC_STATUS.equals(instanceStatus);
    }

    private String getTargetValue(Map<String, String> extendInfo, String key) {
        return extendInfo.getOrDefault(key, "");
    }

    @Override
    public List<TaskEnvironment> getEnvNodesByInstanceResource(ProtectedResource resource) {
        if (ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.equalsSubType(resource.getSubType())) {
            return getSingleNodes(resource);
        }
        return getClusterNodes(resource);
    }

    private List<TaskEnvironment> getSingleNodes(ProtectedResource resource) {
        return convertAgentByCluster(instanceProtectionService.extractEnvNodesBySingleInstance(resource));
    }

    private List<TaskEnvironment> convertAgentByCluster(List<TaskEnvironment> clusterNodes) {
        return clusterNodes.stream().map(this::buildTaskEnvironment).collect(Collectors.toList());
    }

    private TaskEnvironment buildTaskEnvironment(TaskEnvironment taskEnvironment) {
        ProtectedResource agent = getResourceById(taskEnvironment.getUuid()).getDependencies()
                .get(DatabaseConstants.AGENTS)
                .get(0);
        if (!(agent instanceof ProtectedEnvironment)) {
            log.error("This resource is not environment. uuid: {}.", agent.getUuid());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This resource is not environment.");
        }
        Map<String, String> extendInfo = Optional.of(agent.getExtendInfo()).orElse(new HashMap<>());
        if (!VerifyUtil.isEmpty(taskEnvironment.getExtendInfo())) {
            extendInfo.putAll(taskEnvironment.getExtendInfo());
        }
        agent.setExtendInfo(extendInfo);
        return BeanTools.copy((ProtectedEnvironment) agent, TaskEnvironment::new);
    }

    private List<TaskEnvironment> getClusterNodes(ProtectedResource resource) {
        return convertAgentByCluster(instanceProtectionService.extractEnvNodesByClusterInstance(resource));
    }

    @Override
    public List<Endpoint> getAgentsByInstanceResource(ProtectedResource resource) {
        List<TaskEnvironment> nodeList = getEnvNodesByInstanceResource(resource);
        return nodeList.stream()
                .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
                .collect(Collectors.toList());
    }

    @Override
    public ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                        "Protected resource not exist. uuid: " + resourceId));
    }

    @Override
    public void healthCheckOfClusterInstance(ProtectedResource resource) {
        log.debug("Start informix cluster instance health check. uuid: {}.", resource.getUuid());
        String clusterInstanceStatus = LinkStatusEnum.OFFLINE.getStatus().toString();
        List<ProtectedResource> childrenResources = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        for (ProtectedResource instance : childrenResources) {
            ProtectedEnvironment agent = getAgentByInstance(instance);
            try {
                PageListResponse<ProtectedResource> response = listResource(instance, agent, instance.getSubType());
                String instanceStatus = response.getRecords()
                        .get(IsmNumberConstant.ZERO)
                        .getExtendInfo()
                        .get(InformixConstant.INSTANCESTATUS);
                if (InformixConstant.MASTER_NODE_STATUS.equals(instanceStatus)
                        || InformixConstant.MASTER_STATUS_QUIESCENT.equals(instanceStatus)) {
                    clusterInstanceStatus = LinkStatusEnum.ONLINE.getStatus().toString();
                    break;
                }
            } catch (LegoCheckedException e) {
                log.error("Informix query instance status is failed.", e);
            }
        }
        updateResource(resource.getUuid(), clusterInstanceStatus);
    }

    private ProtectedEnvironment getAgentByInstance(ProtectedResource instance) {
        return instance.getDependencies()
                .get(DatabaseConstants.AGENTS)
                .stream()
                .flatMap(StreamUtil.match(ProtectedEnvironment.class))
                .findFirst()
                .orElseThrow(() ->
                        new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This environment is not exist."));
    }

    private void updateResource(String resourceId, String clusterInstanceStatus) {
        ProtectedResource newResource = new ProtectedResource();
        newResource.setUuid(resourceId);
        newResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, clusterInstanceStatus);
        resourceService.updateSourceDirectly(Collections.singletonList(newResource));
    }

    @Override
    public void checkApplication(ProtectedEnvironment environment) {
        String agentUuid = environment.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
        ProtectedEnvironment agentEnv = getEnvironmentById(agentUuid);
        AgentBaseDto agentBaseDto = agentUnifiedService.checkApplicationNoRetry(environment, agentEnv);
        if (agentBaseDto.isAgentBaseDtoReturnSuccess()) {
            return;
        }

        String errorMessage = agentBaseDto.getErrorMessage();
        ActionResult result = JsonUtil.read(Optional.ofNullable(errorMessage).orElse("{}"), ActionResult.class);
        String bodyErrCode = result.getBodyErr();
        if (VerifyUtil.isEmpty(bodyErrCode)) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error.");
        } else {
            throw new LegoCheckedException(Long.parseLong(bodyErrCode),
                    "Check whether the host is installed the informix correctly.");
        }
    }

    @Override
    public void removeRepositoryOfResource(String resourceId) {
        Integer count = jobService.getJobCount(Lists.newArrayList(JobTypeEnum.BACKUP.getValue()),
            Lists.newArrayList(JobStatusEnum.SUCCESS.name()), Lists.newArrayList(resourceId));
        if (count <= 0) {
            log.info("No need to remove data repository white list of resource(uuid={}).", resourceId);
            return;
        }
        Map<String, Object> paramsMap = new HashMap<>();
        ArrayList<String> actionList = new ArrayList<>();
        actionList.add("removeRepository");
        paramsMap.put("resourceId", resourceId);
        paramsMap.put("actions", actionList);
        log.info("Removing data repository white list of resource(uuid={}), body: {}.", resourceId, paramsMap);
        dmeUnifiedRestApi.removeRepoWhiteListOfResource(paramsMap);
    }
}
