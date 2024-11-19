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
package openbackup.gaussdb.protection.access.provider;

import com.huawei.oceanprotect.repository.service.LocalStorageService;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.gaussdb.protection.access.constant.GaussDBConstant;
import openbackup.gaussdb.protection.access.service.GaussDBService;
import openbackup.gaussdb.protection.access.util.GaussDBValidator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.lang3.ObjectUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * The GaussDBClusterEnvironmentProvider
 *
 */
@Slf4j
@Component
public class GaussDBClusterEnvironmentProvider extends DatabaseEnvironmentProvider {
    private final AgentUnifiedService agentUnifiedService;

    @Autowired
    private GaussDBService gaussDbService;

    @Autowired
    private LocalStorageService localStorageService;

    @Autowired
    private JsonSchemaValidator jsonSchemaValidator;

    @Autowired
    private GaussDBAgentProvider gaussDBAgentProvider;

    /**
     * DatabaseResourceProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param agentUnifiedService agentUnifiedService
     */
    public GaussDBClusterEnvironmentProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        AgentUnifiedService agentUnifiedService) {
        super(providerManager, pluginConfigManager);
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT.getType().equals(object);
    }

    /**
     * 资源校验
     *
     * @param environment 受保护环境
     */
    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start to check environment");
        jsonSchemaValidator.doValidate(environment, ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT.getType());

        // 检查连通性
        ProtectedResource protectedResource = getProtectedResource(environment);
        try {
            gaussDbService.checkConnention(protectedResource);
        } catch (LegoCheckedException exception) {
            log.error("check agent connection is wrong");
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "check application is wrong");
        }

        // 获取已注册成功的GaussDb资源信息
        Map<String, Object> filterConditions = new HashMap<>();
        List<ProtectedEnvironment> existingEnvironments = gaussDbService.getExistingGaussDbResources(
                ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT.getType(), filterConditions)
            .stream()
            .filter(existingResource -> existingResource instanceof ProtectedEnvironment)
            .map(existingResource -> (ProtectedEnvironment) existingResource)
            .collect(Collectors.toList());

        // 去框架查询agent信息，不报错表明环境信息正常
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            log.info("start to register");
            GaussDBValidator.checkGaussDbCount(existingEnvironments);

            // 校验注册集群是否重复并设置uuid
            generateUniqueUuid(environment, existingEnvironments);
        }

        // endpoint 适配sla
        Set<String> agentEndpoints = new HashSet<>();
        List<ProtectedResource> agentResources = environment.getDependencies().get(GaussDBConstant.GAUSSDB_AGENTS);
        agentResources.forEach(node -> {
            String endpoint = gaussDbService.getEnvironmentById(node.getUuid()).getEndpoint();
            agentEndpoints.add(endpoint);
        });
        if (agentEndpoints.isEmpty()) {
            throw new LegoCheckedException("endpoint can not been null");
        }

        String clusterEndpoint = agentEndpoints.stream().sorted().collect(Collectors.joining(","));
        environment.setEndpoint(clusterEndpoint);
        environment.setPath(environment.getEndpoint());

        // 检查通过后，添加数据到environment中，由框架负责持久化
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("finish the check");
    }

    private ProtectedResource getProtectedResource(ProtectedEnvironment environment) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(environment.getSubType());
        protectedResource.setType(environment.getType());
        protectedResource.setExtendInfo(environment.getExtendInfo());
        protectedResource.setAuth(environment.getAuth());
        protectedResource.setName(environment.getName());
        protectedResource.setDependencies(environment.getDependencies());
        return protectedResource;
    }

    private void generateUniqueUuid(ProtectedEnvironment environment, List<ProtectedEnvironment> existingEnvironments) {
        String hcsGaussDbClusterUuid = UUID.nameUUIDFromBytes(
            (environment.getName() + environment.getSubType() + localStorageService.getStorageInfo().getEsn()).getBytes(
                Charset.defaultCharset())).toString();
        environment.setUuid(hcsGaussDbClusterUuid);
        environment.setRootUuid(hcsGaussDbClusterUuid);
        environment.setParentUuid(hcsGaussDbClusterUuid);
        boolean isNameOrUuidDuplicate = existingEnvironments.stream()
            .anyMatch(existEnv -> hcsGaussDbClusterUuid.equals(existEnv.getUuid()) || environment.getName()
                .equals(existEnv.getName()) || environment.getExtendInfoByKey(
                    GaussDBConstant.EXTEND_INFO_KEY_PROJECT_ID)
                .equals(existEnv.getExtendInfoByKey(GaussDBConstant.EXTEND_INFO_KEY_PROJECT_ID)));
        if (isNameOrUuidDuplicate) {
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "register is duplicate.");
        }
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        List<ProtectedResource> agentResources = environment.getDependencies().get(GaussDBConstant.GAUSSDB_AGENTS);
        if (ObjectUtils.isEmpty(agentResources)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Environment dependencies agent is empty.");
        }

        List<ProtectedResource> instances = new ArrayList<>();
        for (ProtectedResource agentResource : agentResources) {
            List<String> distanceUuid = instances.stream()
                .map(ProtectedResource::getUuid)
                .distinct()
                .collect(Collectors.toList());

            Endpoint endpoint = selectAgent(environment);
            PageListResponse<ProtectedResource> response;
            int count = GaussDBConstant.INT_ZERO;
            do {
                response = agentUnifiedService.getDetailPageListNoRetry(
                    ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType(), endpoint.getIp(), endpoint.getPort(),
                    generateListResourceV2Req(count++, GaussDBConstant.QUERY_SIZE, environment, agentResource), false);
                if (!response.getRecords().isEmpty()) {
                    buildInstances(response, distanceUuid, agentResource, environment, instances);
                }
            } while (response.getRecords().size() == GaussDBConstant.QUERY_SIZE);
        }
        log.info("the scan instances size {}", instances.size());
        return instances;
    }

    private void buildInstances(PageListResponse<ProtectedResource> response, List<String> distanceUuid,
        ProtectedResource agentResource, ProtectedEnvironment environment, List<ProtectedResource> instances) {
        response.getRecords().forEach(protectedResource -> {
            if (distanceUuid.isEmpty() || !distanceUuid.contains(protectedResource.getUuid())) {
                log.info("new scan instances are inserted");
                instances.add(buildProtectedResource(protectedResource, environment, agentResource));
            }
        });
    }

    /**
     * 受保护环境健康状态检查, 返回连接状态
     *
     * @param environment 受保护环境
     * @return LinkStatusEnum
     */
    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        log.info("start to health check");
        Map<String, Object> filterConditions = new HashMap<>();

        // 数据库中已有的实例信息
        List<ProtectedResource> existingGaussDbResources = gaussDbService.getExistingGaussDbResources(
            ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType(), filterConditions);
        try {
            // 扫描生产端的数据信息
            log.info("start to scan resources");
            scan(environment);
        } catch (LegoCheckedException exception) {
            log.info("the status is offline");
            existingGaussDbResources.forEach(instance -> gaussDbService.updateResourceLinkStatus(instance.getUuid(),
                LinkStatusEnum.OFFLINE.getStatus().toString()));
            return Optional.of(LinkStatusEnum.OFFLINE.getStatus().toString());
        }
        log.info("health check end and the cluster status is online");
        return Optional.of(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 生成ListResourceV2Req
     *
     * @param page page
     * @param size size
     * @param environment environment
     * @param agentResource agentResource
     * @return ListResourceV2Req ListResourceV2Req
     */
    private ListResourceV2Req generateListResourceV2Req(int page, int size, ProtectedEnvironment environment,
        ProtectedResource agentResource) {
        ListResourceV2Req listResourceV2Req = new ListResourceV2Req();
        AppEnv copy = BeanTools.copy(environment, AppEnv::new);
        copy.setSubType(ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType());
        listResourceV2Req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        listResourceV2Req.setPageSize(size);
        listResourceV2Req.setPageNo(page);
        agentResource.setSubType(ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType());
        listResourceV2Req.setApplications(Lists.newArrayList(BeanTools.copy(agentResource, Application::new)));
        return listResourceV2Req;
    }

    /**
     * 生成ProtectedResource
     *
     * @param protectedResource agentResource
     * @param environment environment
     * @param agentResource agentResource
     * @return ProtectedResource 资源信息
     */
    private ProtectedResource buildProtectedResource(ProtectedResource protectedResource,
        ProtectedEnvironment environment, ProtectedResource agentResource) {
        ProtectedResource resource = new ProtectedResource();
        String resourceUuid = protectedResource.getUuid();
        resource.setUuid(resourceUuid);
        resource.setName(protectedResource.getName());
        resource.setType(protectedResource.getType());
        resource.setSubType(protectedResource.getSubType());
        log.info("set gaussDb getEndpoint:{}, path:{}", environment.getEndpoint(), environment.getPath());
        resource.setPath(environment.getEndpoint());
        log.info("set gaussDb path:{}", resource.getPath());

        resource.setAuth(environment.getAuth());
        resource.setVersion(protectedResource.getExtendInfo().get(GaussDBConstant.VERSION));

        String extendStatus = LinkStatusEnum.ABNORMAL.getStatus().toString();
        if (GaussDBConstant.NORMAL_VALUE_STATE.equals(
            protectedResource.getExtendInfoByKey(GaussDBConstant.EXTEND_INFO_KEY_STATE))) {
            extendStatus = LinkStatusEnum.ONLINE.getStatus().toString();
        }
        Map<String, String> extendInfo = new HashMap<>(environment.getExtendInfo());
        extendInfo.put(GaussDBConstant.EXTEND_INFO_KEY_STATE, extendStatus);
        extendInfo.put(GaussDBConstant.REGION, protectedResource.getExtendInfo().get(GaussDBConstant.REGION));
        resource.setExtendInfo(extendInfo);
        resource.setParentName(environment.getName());
        resource.setParentUuid(environment.getUuid());
        resource.setRootUuid(environment.getUuid());
        return resource;
    }

    private Endpoint selectAgent(ProtectedResource resource) {
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.RESOURCE_SCAN.getValue())
            .build();

        return gaussDBAgentProvider.getSelectedAgents(agentSelectParam)
            .stream()
            .findFirst()
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "not find any agent can connect"));
    }
}
