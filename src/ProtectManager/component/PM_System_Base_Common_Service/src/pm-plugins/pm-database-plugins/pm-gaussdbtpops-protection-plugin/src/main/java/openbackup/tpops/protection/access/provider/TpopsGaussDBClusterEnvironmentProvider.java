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
package openbackup.tpops.protection.access.provider;

import static java.util.stream.Collectors.joining;
import static java.util.stream.Collectors.toList;
import static java.util.stream.Collectors.toMap;
import static java.util.stream.Collectors.toSet;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.DB_VERSION;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.EXTEND_INFO_KEY_INSTANCE_STATUS;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.EXTEND_INFO_KEY_PROJECT_ADDRESS;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.EXTEND_INFO_KEY_STATE;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.EXTEND_INFO_VALUE_INSTANCE_ABNORMAL;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.EXTEND_INFO_VALUE_INSTANCE_ONLINE;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.GAUSSDB_AGENTS;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.NORMAL_VALUE_STATE;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.REGION;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.TPOPS_VERSION;
import static openbackup.tpops.protection.access.constant.TpopsGaussDBConstant.VERSION;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tpops.protection.access.service.TpopsGaussDBService;
import openbackup.tpops.protection.access.util.TpopsGaussDBValidator;

import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.lang3.ObjectUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.function.Function;

/**
 * The GaussDBClusterEnvironmentProvider
 *
 */
@Slf4j
@Component
public class TpopsGaussDBClusterEnvironmentProvider extends DatabaseEnvironmentProvider {
    private final AgentUnifiedService agentUnifiedService;

    @Autowired
    private TpopsGaussDBService tpopsGaussDbService;

    @Autowired
    private JsonSchemaValidator jsonSchemaValidator;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private ResourceExtendInfoService resourceExtendInfoService;

    @Autowired
    private ClusterBasicService clusterBasicService;

    /**
     * DatabaseResourceProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param agentUnifiedService agentUnifiedService
     */
    public TpopsGaussDBClusterEnvironmentProvider(ProviderManager providerManager,
        PluginConfigManager pluginConfigManager, AgentUnifiedService agentUnifiedService) {
        super(providerManager, pluginConfigManager);
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.getType().equals(object);
    }

    /**
     * 资源校验
     *
     * @param environment 受保护环境
     */
    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start to check environment");
        jsonSchemaValidator.doValidate(environment, ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.getType());

        // 检查连通性
        ProtectedResource protectedResource = getProtectedResource(environment);
        try {
            tpopsGaussDbService.checkConnention(protectedResource);
        } catch (LegoCheckedException exception) {
            log.error("check agent connection is wrong");
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "check application is wrong");
        }

        // 获取已注册成功的GaussDb资源信息
        Map<String, Object> filterConditions = new HashMap<>();
        List<ProtectedEnvironment> existingEnvironments = tpopsGaussDbService.getExistingGaussDbResources(
                ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.getType(), filterConditions)
            .stream()
            .filter(existingResource -> existingResource instanceof ProtectedEnvironment)
            .map(existingResource -> (ProtectedEnvironment) existingResource)
            .collect(toList());

        // 去框架查询agent信息，不报错表明环境信息正常
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            log.info("start to register");
            TpopsGaussDBValidator.checkGaussDbCount(existingEnvironments);

            // 校验注册集群是否重复并设置uuid
            generateUniqueUuid(environment, existingEnvironments);
        } else {
            // 修改项目时，更新实例的所属项目字段
            log.info("start to modify parentName");
            Map<String, Object> updateKv = new HashMap<>();
            updateKv.put("parent_name", environment.getName());
            resourceService.updateSubResource(Collections.singletonList(environment.getUuid()), updateKv);
        }

        // endpoint 适配sla
        Set<String> agentEndpoints = new HashSet<>();
        List<ProtectedResource> agentResources = environment.getDependencies().get(GAUSSDB_AGENTS);
        agentResources.forEach(node -> {
            String endpoint = tpopsGaussDbService.getEnvironmentById(node.getUuid()).getEndpoint();
            agentEndpoints.add(endpoint);
        });
        if (agentEndpoints.isEmpty()) {
            throw new LegoCheckedException("endpoint can not been null");
        }

        String clusterEndpoint = agentEndpoints.stream().sorted().collect(joining(","));
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
        String uuid = UUID.nameUUIDFromBytes(
            (environment.getName() + environment.getSubType() + clusterBasicService.getCurrentClusterEsn()).getBytes(
                Charset.defaultCharset())).toString();
        environment.setUuid(uuid);
        environment.setRootUuid(uuid);
        environment.setParentUuid(uuid);
        boolean isNameOrUuidDuplicate = existingEnvironments.stream()
            .anyMatch(existEnv -> uuid.equals(existEnv.getUuid()) || environment.getName().equals(existEnv.getName())
                || environment.getExtendInfoByKey(EXTEND_INFO_KEY_PROJECT_ADDRESS)
                .equals(existEnv.getExtendInfoByKey(EXTEND_INFO_KEY_PROJECT_ADDRESS)));
        if (isNameOrUuidDuplicate) {
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "register is duplicate.");
        }
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        List<ProtectedResource> agentResources = environment.getDependencies().get(GAUSSDB_AGENTS);
        if (ObjectUtils.isEmpty(agentResources)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Environment dependencies agent is empty.");
        }

        LegoCheckedException legoCheckedException = new LegoCheckedException("Scan tpops resource failed");
        for (ProtectedResource agent : agentResources) {
            try {
                List<ProtectedResource> gaussInstances = scanResource(environment, agent);
                log.info("The scan instances size {}", gaussInstances.size());
                setTpopsVersion(gaussInstances, environment);
                return gaussInstances;
            } catch (LegoCheckedException exception) {
                log.error("Get tpops resource failed, try next");
                legoCheckedException = exception;
            }
        }
        throw legoCheckedException;
    }

    private void setTpopsVersion(List<ProtectedResource> instances, ProtectedEnvironment environment) {
        log.info("start set tpops version");
        if (!instances.isEmpty()) {
            Map<String, String> extendInfo = environment.getExtendInfo();
            if (!extendInfo.isEmpty()) {
                String tpopsVersion = instances.get(0).getExtendInfoByKey(TPOPS_VERSION);
                extendInfo.put(TPOPS_VERSION, tpopsVersion);
                resourceExtendInfoService.saveOrUpdateExtendInfo(environment.getUuid(), extendInfo);
            }
        }
    }

    private List<ProtectedResource> queryProjectInstances(ProtectedEnvironment environment) {
        Map<String, Object> filter = new HashMap<>();
        filter.put("parentUuid", environment.getUuid());
        filter.put("type", ResourceTypeEnum.DATABASE.getType());
        filter.put("subType", ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
        return resourceService.queryAllResources(filter);
    }

    private List<ProtectedResource> scanResource(ProtectedEnvironment environment, ProtectedResource agent) {
        String endpoint = agent.getEndpoint();
        Integer port = agent.getPort();
        log.info("Scan tpops resource in endpoint: {}, port: {}, resource id: {}", endpoint, port,
            environment.getUuid());
        List<ProtectedResource> gaussInstances = queryProjectInstances(environment);
        log.info("Get past resource size: {}", gaussInstances);
        PageListResponse<ProtectedResource> pageList = agentUnifiedService.getDetailPageListNoRetry(
            ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.getType(), endpoint, port,
            generateListResourceV2Req(environment, agent), false);
        // 查询为空，所有实例不可用
        if (CollectionUtils.isEmpty(pageList.getRecords())) {
            gaussInstances.forEach(instance -> {
                instance.setSourceType(ResourceConstants.SOURCE_TYPE_REGISTER);
                instance.setExtendInfoByKey(EXTEND_INFO_KEY_INSTANCE_STATUS,
                    LinkStatusEnum.UNAVAILABLE.getStatus().toString());
            });
            return gaussInstances;
        }
        List<ProtectedResource> returnList = new ArrayList<>(gaussInstances);
        Set<String> onPMIstIds = gaussInstances.stream().map(ProtectedResource::getUuid).collect(toSet());
        Set<String> onProtectIds = pageList.getRecords().stream().map(ProtectedResource::getUuid).collect(toSet());
        Map<String, ProtectedResource> queryResMap = pageList.getRecords()
            .stream()
            .map(queryRes -> buildProtectedResource(queryRes, environment))
            .collect(toMap(ProtectedResource::getUuid, Function.identity(), (v1, v2) -> v1));
        // 查询结果包含,ACTIVE处理成在线，INACTIVE处理成故障
        returnList.stream().filter(instance -> onProtectIds.contains(instance.getUuid())).forEach(instance -> {
            instance.setSourceType(ResourceConstants.SOURCE_TYPE_REGISTER);
            ProtectedResource queryRes = queryResMap.get(instance.getUuid());
            instance.setPath(queryRes.getPath());
            instance.setAuth(queryRes.getAuth());
            instance.setVersion(queryRes.getVersion());
            instance.setExtendInfo(queryRes.getExtendInfo());
        });
        // 如果存在于PM，但不在当前查询结果中，处理为不可用
        returnList.stream().filter(instance -> !onProtectIds.contains(instance.getUuid())).forEach(instance -> {
            instance.setSourceType(ResourceConstants.SOURCE_TYPE_REGISTER);
            instance.setExtendInfoByKey(EXTEND_INFO_KEY_INSTANCE_STATUS,
                LinkStatusEnum.UNAVAILABLE.getStatus().toString());
        });
        // PM中不存在，直接新增
        List<ProtectedResource> addInstances = pageList.getRecords()
            .stream()
            .filter(protectedResource -> !onPMIstIds.contains(protectedResource.getUuid()))
            .map(queryRes -> buildProtectedResource(queryRes, environment))
            .collect(toList());
        returnList.addAll(addInstances);
        return returnList;
    }

    /**
     * 受保护环境健康状态检查, 返回连接状态
     *
     * @param environment 受保护环境
     * @return LinkStatusEnum
     */
    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        log.info("start to health check start, env id:{}", environment.getUuid());
        // 数据库中已有的实例信息
        List<ProtectedResource> gaussInstances = this.queryProjectInstances(environment);
        try {
            // 扫描生产端的数据信息
            log.info("start to check tpops project connection");
            tpopsGaussDbService.checkConnention(environment);
            log.info("start to scan tpops instance info");
            List<ProtectedResource> scanResults = scan(environment);
            Set<String> onPMIstIds = gaussInstances.stream().map(ProtectedResource::getUuid).collect(toSet());
            for (ProtectedResource instance : scanResults) {
                // 如果存在于PM，更新实例信息
                if (onPMIstIds.contains(instance.getUuid())) {
                    log.info("update gauss instance status: instance id:{}, status:{}", instance.getUuid(),
                        instance.getExtendInfo());
                    resourceService.updateSourceDirectly(Collections.singletonList(instance));
                }
            }
        } catch (LegoCheckedException exception) {
            log.info("health check error", ExceptionUtil.getErrorMessage(exception));
            // 健康检查失败，所有实例都处理为离线
            gaussInstances.forEach(instance -> tpopsGaussDbService.updateResourceLinkStatus(instance.getUuid(),
                LinkStatusEnum.OFFLINE.getStatus().toString()));
            return Optional.of(LinkStatusEnum.OFFLINE.getStatus().toString());
        }
        log.info("health check end and the cluster status is online");
        return Optional.of(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 生成ListResourceV2Req
     *
     * @param environment environment
     * @param agent agentResource
     * @return ListResourceV2Req ListResourceV2Req
     */
    private ListResourceV2Req generateListResourceV2Req(ProtectedEnvironment environment, ProtectedResource agent) {
        ListResourceV2Req listResourceV2Req = new ListResourceV2Req();
        AppEnv copy = BeanTools.copy(environment, AppEnv::new);
        copy.setSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
        listResourceV2Req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        agent.setSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
        listResourceV2Req.setApplications(Lists.newArrayList(BeanTools.copy(agent, Application::new)));
        return listResourceV2Req;
    }

    /**
     * 生成ProtectedResource
     *
     * @param protectedResource agentResource
     * @param environment environment
     * @return ProtectedResource 资源信息
     */
    private ProtectedResource buildProtectedResource(ProtectedResource protectedResource,
        ProtectedEnvironment environment) {
        ProtectedResource resource = new ProtectedResource();
        String resourceUuid = protectedResource.getUuid();
        resource.setUuid(resourceUuid);
        resource.setName(protectedResource.getName());
        resource.setType(protectedResource.getType());
        resource.setSubType(protectedResource.getSubType());
        resource.setPath(environment.getPath());
        log.info("set gaussDb getEndpoint: {}, path: {}", environment.getEndpoint(), environment.getPath());
        resource.setPath(environment.getEndpoint());
        log.info("set gaussDb path: {}", resource.getPath());

        resource.setAuth(environment.getAuth());
        resource.setVersion(protectedResource.getExtendInfo().get(VERSION));

        String extendStatus = LinkStatusEnum.ABNORMAL.getStatus().toString();
        if (NORMAL_VALUE_STATE.equals(protectedResource.getExtendInfoByKey(EXTEND_INFO_KEY_STATE))) {
            extendStatus = LinkStatusEnum.ONLINE.getStatus().toString();
        }
        Map<String, String> extendInfo = new HashMap<>(environment.getExtendInfo());
        extendInfo.put(EXTEND_INFO_KEY_STATE, extendStatus);
        String instanceStatus = EXTEND_INFO_VALUE_INSTANCE_ONLINE;

        // 实例状态正常
        if (!NORMAL_VALUE_STATE.equals(protectedResource.getExtendInfoByKey(EXTEND_INFO_KEY_STATE))) {
            instanceStatus = EXTEND_INFO_VALUE_INSTANCE_ABNORMAL;
        }
        log.info("Get instanceStatus: {} status: {}", protectedResource.getName(), instanceStatus);
        extendInfo.put(EXTEND_INFO_KEY_INSTANCE_STATUS, instanceStatus);
        extendInfo.put(REGION, protectedResource.getExtendInfo().get(REGION));
        extendInfo.put(DB_VERSION, protectedResource.getExtendInfo().get(DB_VERSION));
        extendInfo.put(TPOPS_VERSION, protectedResource.getExtendInfo().get(TPOPS_VERSION));
        extendInfo.remove(ResourceConstants.IS_ALLOW_RESTORE_KEY);
        resource.setExtendInfo(extendInfo);
        log.info("Get environment name {}", environment.getName());
        resource.setParentName(environment.getName());
        resource.setParentUuid(environment.getUuid());
        resource.setRootUuid(environment.getUuid());
        return resource;
    }
}
