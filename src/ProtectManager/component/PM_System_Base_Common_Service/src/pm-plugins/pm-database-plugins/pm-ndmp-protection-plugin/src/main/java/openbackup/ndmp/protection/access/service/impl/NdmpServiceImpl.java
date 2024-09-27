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
package openbackup.ndmp.protection.access.service.impl;

import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import com.huawei.oceanprotect.nas.protection.access.service.StorageAgentService;
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.ndmp.protection.access.service.NdmpService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.SymbolConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.compress.utils.Lists;
import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Random;
import java.util.stream.Collectors;

/**
 * ndmp 服务实现类
 *
 * @author t30021437
 * @since 2023-05-06
 */
@Slf4j
@Service
public class NdmpServiceImpl implements NdmpService {
    private final ProviderManager providerManager;

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    private final ResourceService resourceService;

    private final UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    private final StorageAgentService storageAgentService;

    @Autowired
    private AgentUnifiedService agentService;

    @Autowired
    private ProtectedEnvironmentService protectedEnvironmentService;

    /**
     * NDMP 应用基本的Service有参构造方法
     *
     * @param providerManager DataMover provider registry
     * @param resourceConnectionCheckProvider resourceConnectionCheckProvider
     * @param resourceService resourceService
     * @param clusterIntegrityChecker clusterIntegrityChecker
     * @param storageAgentService storageAgentService
     */
    public NdmpServiceImpl(ProviderManager providerManager,
        ResourceConnectionCheckProvider resourceConnectionCheckProvider, ResourceService resourceService,
        UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker, StorageAgentService storageAgentService) {
        this.providerManager = providerManager;
        this.resourceConnectionCheckProvider = resourceConnectionCheckProvider;
        this.resourceService = resourceService;
        this.clusterIntegrityChecker = clusterIntegrityChecker;
        this.storageAgentService = storageAgentService;
    }

    /**
     * 检查连通性
     *
     * @param protectedResource protectedResource
     */
    @Override
    public void checkConnention(ProtectedResource protectedResource) {
        log.info("check connection");
        ResourceConnectionCheckProvider provider = providerManager.findProviderOrDefault(
            ResourceConnectionCheckProvider.class, protectedResource, resourceConnectionCheckProvider);
        provider.checkConnection(protectedResource);
    }

    /**
     * 获取已存在的NDMP资源信息
     *
     * @param filter 查询条件
     * @return 已存在的GaussDb资源信息
     */
    @Override
    public List<ProtectedResource> getexistingNdmpresources(Map<String, Object> filter) {
        log.info("query the existing Ndmp resources");
        List<ProtectedResource> existingResources = new ArrayList<>();
        PageListResponse<ProtectedResource> response;
        int count = NdmpConstant.INT_ZERO;
        do {
            response = resourceService.query(count++, NdmpConstant.QUERY_SIZE, filter);
            if (!response.getRecords().isEmpty()) {
                existingResources.addAll(response.getRecords());
            }
        } while (response.getRecords().size() == NdmpConstant.QUERY_SIZE);
        return Optional.ofNullable(existingResources).orElse(Collections.emptyList());
    }

    /**
     * 获取已存在的NDMP资源信息
     *
     * @param subType 资源类型
     * @param filter 查询条件
     * @return 已存在的GaussDb资源信息
     */
    @Override
    public List<ProtectedResource> getexistingNdmpresources(String subType, Map<String, Object> filter) {
        log.info("query the existing Ndmp resources");
        filter.put("type", ResourceTypeEnum.STORAGE_EQUIPMENT.getType());
        filter.put("subType", subType);
        List<ProtectedResource> existingResources = new ArrayList<>();
        PageListResponse<ProtectedResource> response;
        int count = NdmpConstant.INT_ZERO;
        do {
            response = resourceService.query(count++, NdmpConstant.QUERY_SIZE, filter);
            if (!response.getRecords().isEmpty()) {
                existingResources.addAll(response.getRecords());
            }
        } while (response.getRecords().size() == NdmpConstant.QUERY_SIZE);
        return Optional.ofNullable(existingResources).orElse(Collections.emptyList());
    }

    /**
     * 获取environment 对象
     *
     * @param envId environment的uuid
     * @return 查询资源对象
     */
    @Override
    public ProtectedEnvironment getEnvironmentById(String envId) {
        log.info("start to query Environment by id");
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(envId);
        if (resOptional.isPresent() && resOptional.get() instanceof ProtectedEnvironment) {
            log.info("start to query getEnvironmentById :{}", resOptional.get());
            return (ProtectedEnvironment) resOptional.get();
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!");
    }

    /**
     * 检查连通性
     *
     * @param agentResources agentResources agent 列表
     * @param protectedResource protectedResource
     * @return AppEnvResponse AppEnvResponse agent 信息
     */
    @Override
    public AppEnvResponse getAppEnvResponse(List<ProtectedResource> agentResources,
        ProtectedResource protectedResource) {
        log.info("query agent environment");
        ProtectedEnvironment protectedEnvironment = getEnvironmentById(Optional.of(agentResources)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList())
            .get(NdmpConstant.INT_ZERO));
        protectedResource.setEnvironment(protectedEnvironment);
        CheckResult<AppEnvResponse> appEnvResponseCheckResult = clusterIntegrityChecker.generateCheckResult(
            protectedResource);
        AppEnvResponse appEnv = appEnvResponseCheckResult.getData();
        if (ObjectUtils.isEmpty(appEnvResponseCheckResult.getData())) {
            log.error("The ndmp nodes query failed.");
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, "The ndmp query failed.");
        }
        return appEnv;
    }

    /**
     * 返回节点信息列表
     *
     * @return 节点信息列表
     */
    @Override
    public List<TaskEnvironment> supplyNodes() {
        List<Endpoint> endpoints = storageAgentService.queryInternalAgents();
        List<ProtectedEnvironment> ndmpAgent = endpoints.stream()
            .map(Endpoint::getId)
            .map(this::getEnvironmentById)
            .collect(Collectors.toList());

        List<TaskEnvironment> nodes = new ArrayList<>();
        nodes.addAll(ndmpAgent.stream()
            .filter(protectedEnvironment -> LinkStatusEnum.ONLINE.getStatus()
                .toString()
                .equals(protectedEnvironment.getLinkStatus()))
            .map(environment -> BeanTools.copy(environment, TaskEnvironment::new))
            .collect(Collectors.toList()));
        return nodes;
    }

    /**
     * 修改task信息
     *
     * @param backupTask backupTask
     */
    @Override
    public void modifyBackupTaskParam(BackupTask backupTask) {
        String storageType = backupTask.getProtectEnv().getExtendInfo().get(NdmpConstant.STORAGE_TYPE);
        if (NdmpConstant.DORADO_FORMAT.equals(storageType)) {
            // dorado设备原生格式
            backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
            backupTask.getRepositories()
                .get(0)
                .getExtendInfo()
                .put(NdmpConstant.COPY_FORMAT, CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        } else {
            // NetApp用目录格式
            backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
            backupTask.getRepositories()
                .get(0)
                .getExtendInfo()
                .put(NdmpConstant.COPY_FORMAT, CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        }

        // 添加存储仓库类型：2-CACHE_REPOSITORY
        List<StorageRepository> repositories = backupTask.getRepositories();
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);

        // 设置速率上报方式
        Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams()).orElse(new HashMap<>());
        advanceParams.put(NdmpConstant.SPEED_STATISTICS, SpeedStatisticsEnum.APPLICATION.getType());

        // 部署类型
        TaskEnvironment protectEnv = backupTask.getProtectEnv();
        Map<String, String> extendInfo = protectEnv.getExtendInfo();
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SHARDING.getType());
    }

    /**
     * 删除资源信息
     *
     * @param resources resources
     */
    @Override
    public void deleteResourses(String[] resources) {
        resourceService.delete(resources);
    }

    /**
     * 获取内置agent
     *
     * @return List<ProtectedResource> 内置agents资源信息
     */
    @Override
    public List<ProtectedResource> getInterAgents() {
        log.info("start to query internal agents");
        List<Endpoint> endpoints = storageAgentService.queryInternalAgents();
        List<ProtectedResource> agentResources = new ArrayList<>();
        endpoints.forEach(endpoint -> {
            Optional<ProtectedResource> internalAgent = resourceService.getResourceById(endpoint.getId());
            internalAgent.ifPresent(resource -> agentResources.add(resource));
        });
        log.info("the size of internal agents is {}", agentResources.size());
        return agentResources;
    }

    /**
     * 获取可用的内置agent
     *
     * @param environment environment
     * @return List<ProtectedResource> 获取可用的内置agent资源信息
     */
    @Override
    public List<ProtectedResource> getOneAgentHealthCheck(ProtectedEnvironment environment) {
        List<ProtectedResource> agents = environment.getDependencies().get("agents");
        ProtectedEnvironment protectedEnvironment = protectedEnvironmentService.getEnvironmentById(
            agents.get(0).getUuid());
        List<ProtectedEnvironment> agentEnvList = Lists.newArrayList();
        agentEnvList.add(protectedEnvironment);
        List<ProtectedResource> availableAgents = new ArrayList<>();
        log.info("Prepare to health check, agent num:{}.", agentEnvList.size());
        for (ProtectedEnvironment agent : agentEnvList) {
            if (!agent.getLinkStatus().equals(LinkStatusEnum.ONLINE.getStatus().toString())) {
                log.warn("Agent is offline, uuid: {}, status: {}.", agent.getUuid(), agent.getLinkStatus());
                continue;
            }
            AgentBaseDto response = agentService.checkApplication(environment, agent);
            if (!VerifyUtil.isEmpty(response) && NdmpConstant.SUCCESS.equals(response.getErrorCode())) {
                availableAgents.add(agent);
                break;
            }
        }
        log.info("select one availableAgents to health check, agent num:{}.", availableAgents.size());
        return availableAgents;
    }

    @Override
    public List<ProtectedResource> getAvailableAgents(ProtectedEnvironment environment) {
        log.info("NdmpServiceImpl getAvailableAgents environment uuid: {}.", environment.getUuid());
        List<ProtectedResource> agents = environment.getDependencies().get("agents");
        List<ProtectedResource> availableAgents = new ArrayList<>();
        for (ProtectedResource agent : agents) {
            ProtectedEnvironment protectedEnvironment = protectedEnvironmentService.getEnvironmentById(agent.getUuid());
            if (!protectedEnvironment.getLinkStatus().equals(LinkStatusEnum.ONLINE.getStatus().toString())) {
                log.warn("Agent is offline, uuid: {}, status: {}.", protectedEnvironment.getUuid(),
                    protectedEnvironment.getLinkStatus());
                continue;
            }
            try {
                AgentBaseDto response = agentService.checkApplication(environment, protectedEnvironment);
                if (!VerifyUtil.isEmpty(response) && NdmpConstant.SUCCESS.equals(response.getErrorCode())) {
                    availableAgents.add(protectedEnvironment);
                }
            } catch (FeignException | LegoUncheckedException | LegoCheckedException e) {
                log.warn("get StandByAlarm failed!message", ExceptionUtil.getErrorMessage(e));
            }
        }
        log.info("NdmpServiceImpl get available agents num:{}.", availableAgents.size());
        return availableAgents;
    }

    @Override
    public List<Endpoint> getAgents(String parentUuid, String agents) {
        String[] agentIds = agents.split(SymbolConstant.SEMICOLON);
        List<ProtectedResource> agentList = new ArrayList<>();
        for (String agentUuid : agentIds) {
            Optional<ProtectedResource> agentResourceOpt = resourceService.getResourceByIdIgnoreOwner(agentUuid);
            if (agentResourceOpt.isPresent()) {
                log.error("agent not exist. agent uuid: {}.", agentUuid);
                agentList.add(agentResourceOpt.get());
            }
        }
        ProtectedEnvironment protectedEnvironment = protectedEnvironmentService.getEnvironmentById(parentUuid);
        protectedEnvironment.getDependencies().put("agents", agentList);
        List<ProtectedResource> protectedEnvironments = getAvailableAgents(protectedEnvironment);
        List<String> agentIdStrs = protectedEnvironments.stream()
            .map(item -> item.getUuid())
            .collect(Collectors.toList());
        List<Endpoint> endpoints = getEndpointList(StringUtils.join(agentIdStrs, SymbolConstant.SEMICOLON));
        if (CollectionUtils.isNotEmpty(endpoints)) {
            Endpoint endpoint = endpoints.stream()
                .skip(new Random().nextInt(endpoints.size()))
                .findFirst()
                .orElse(null);
            return Arrays.asList(endpoint);
        }
        return Lists.newArrayList();
    }

    @Override
    public void checkApplication(ProtectedEnvironment environment, ProtectedEnvironment agent) {
        AgentBaseDto response = agentService.checkApplication(environment, agent);
        if (VerifyUtil.isEmpty(response) || !NdmpConstant.SUCCESS.equals(response.getErrorCode())) {
            throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, "Network connection failed");
        }
    }

    @Override
    public List<Endpoint> getEndpointList(String agents) {
        return Arrays.stream(agents.split(SymbolConstant.SEMICOLON))
            .map(this::getEndpoint)
            .filter(Optional::isPresent)
            .filter(
                endpoint -> !(VerifyUtil.isEmpty(endpoint.get().getId()) || VerifyUtil.isEmpty(endpoint.get().getPort())
                    || VerifyUtil.isEmpty(endpoint.get().getIp())))
            .map(Optional::get)
            .collect(Collectors.toList());
    }

    private Optional<Endpoint> getEndpoint(String agentUuid) {
        Optional<ProtectedResource> agentResourceOpt = resourceService.getResourceByIdIgnoreOwner(agentUuid);
        if (!agentResourceOpt.isPresent()) {
            log.error("agent not exist. agent uuid: {}.", agentUuid);
            return Optional.empty();
        }
        return agentResourceOpt.filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()));
    }
}
