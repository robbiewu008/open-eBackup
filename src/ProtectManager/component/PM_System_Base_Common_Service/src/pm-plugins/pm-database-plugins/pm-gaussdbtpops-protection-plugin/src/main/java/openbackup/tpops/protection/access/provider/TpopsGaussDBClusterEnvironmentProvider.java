/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tpops.protection.access.provider;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
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
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;
import openbackup.tpops.protection.access.service.TpopsGaussDBService;
import openbackup.tpops.protection.access.util.TpopsGaussDBValidator;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

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
 * @author x30021699
 * @since 2023-05-09
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
    private TpopsGaussDBAgentProvider tpopsGaussDBAgentProvider;

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
            .collect(Collectors.toList());

        // 去框架查询agent信息，不报错表明环境信息正常
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            log.info("start to register");
            TpopsGaussDBValidator.checkGaussDbCount(existingEnvironments);

            // 校验注册集群是否重复并设置uuid
            generateUniqueUuid(environment, existingEnvironments);
        }

        // endpoint 适配sla
        Set<String> agentEndpoints = new HashSet<>();
        List<ProtectedResource> agentResources = environment.getDependencies().get(TpopsGaussDBConstant.GAUSSDB_AGENTS);
        agentResources.forEach(node -> {
            String endpoint = tpopsGaussDbService.getEnvironmentById(node.getUuid()).getEndpoint();
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
        String uuid = UUID.nameUUIDFromBytes(
            (environment.getName() + environment.getSubType() + clusterBasicService.getCurrentClusterEsn()).getBytes(
                Charset.defaultCharset())).toString();
        environment.setUuid(uuid);
        environment.setRootUuid(uuid);
        environment.setParentUuid(uuid);
        boolean isNameOrUuidDuplicate = existingEnvironments.stream()
            .anyMatch(existEnv -> uuid.equals(existEnv.getUuid()) || environment.getName().equals(existEnv.getName())
                || environment.getExtendInfoByKey(TpopsGaussDBConstant.EXTEND_INFO_KEY_PROJECT_ADDRESS)
                .equals(existEnv.getExtendInfoByKey(TpopsGaussDBConstant.EXTEND_INFO_KEY_PROJECT_ADDRESS)));
        if (isNameOrUuidDuplicate) {
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "register is duplicate.");
        }
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        List<ProtectedResource> agentResources = environment.getDependencies().get(TpopsGaussDBConstant.GAUSSDB_AGENTS);
        if (ObjectUtils.isEmpty(agentResources)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Environment dependencies agent is empty.");
        }

        List<ProtectedResource> instances = new ArrayList<>();
        LegoCheckedException legoCheckedException = new LegoCheckedException("Scan tpops resource failed");
        for (ProtectedResource agentResource : agentResources) {
            try {
                scanResource(environment, instances, agentResource);
                log.info("The scan instances size {}", instances.size());
                return instances;
            } catch (LegoCheckedException exception) {
                log.error("Get tpops resource failed, try next");
                legoCheckedException = exception;
            }
        }
        throw legoCheckedException;
    }

    private void scanResource(ProtectedEnvironment environment, List<ProtectedResource> instances,
        ProtectedResource agentResource) {
        String endpoint = agentResource.getEndpoint();
        Integer port = agentResource.getPort();
        log.info("Scan tpops resource in endpoint: {}, port: {}, resource id: {}", endpoint, port,
            environment.getUuid());
        Map<String, Object> filter = new HashMap<>();
        PageListResponse<ProtectedResource> result;
        ArrayList<ProtectedResource> pastResource = new ArrayList<>();
        int pageNo = 0;
        do {
            filter.put("parentUuid", environment.getUuid());
            filter.put("type", ResourceTypeEnum.DATABASE.getType());
            filter.put("subType", ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
            result = resourceService.query(pageNo, TpopsGaussDBConstant.QUERY_SIZE, filter);

            // 读不到资源直接退出
            if (result.getRecords().size() == 0) {
                break;
            }
            pastResource.addAll(result.getRecords());
            pageNo++;
        } while (result.getRecords().size() >= TpopsGaussDBConstant.QUERY_SIZE);

        log.info("Get past resource size: {}", pastResource.size());
        PageListResponse<ProtectedResource> response;
        int count = TpopsGaussDBConstant.INT_ZERO;
        List<String> distanceUuid = instances.stream()
            .map(ProtectedResource::getUuid)
            .distinct()
            .collect(Collectors.toList());
        do {
            response = agentUnifiedService.getDetailPageListNoRetry(
                ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType(), endpoint, port,
                generateListResourceV2Req(count++, TpopsGaussDBConstant.QUERY_SIZE, environment, agentResource), false);
            if (!response.getRecords().isEmpty()) {
                buildInstances(response, distanceUuid, environment, instances);
            }
        } while (response.getRecords().size() == TpopsGaussDBConstant.QUERY_SIZE);
        // 追加离线代理
        if (pastResource.size() == 0) {
            log.info("Past instance is empty");
            return;
        }
        addDeletedInstances(pastResource, distanceUuid, environment, instances);
    }

    private void buildInstances(PageListResponse<ProtectedResource> response, List<String> distanceUuid,
        ProtectedEnvironment environment, List<ProtectedResource> instances) {
        response.getRecords().forEach(protectedResource -> {
            if (distanceUuid.isEmpty() || !distanceUuid.contains(protectedResource.getUuid())) {
                log.info("new scan instances are inserted, uuid: {}", protectedResource.getUuid());
                instances.add(buildProtectedResource(protectedResource, environment, false));
                distanceUuid.add(protectedResource.getUuid());
            }
        });
    }

    private void addDeletedInstances(List<ProtectedResource> existInstance, List<String> distanceUuid,
        ProtectedEnvironment environment, List<ProtectedResource> instances) {
        existInstance.forEach(protectedResource -> {
            if (distanceUuid.isEmpty() || !distanceUuid.contains(protectedResource.getUuid())) {
                log.info("past instances are inserted, uuid: {}", protectedResource.getUuid());
                instances.add(buildProtectedResource(protectedResource, environment, true));
                distanceUuid.add(protectedResource.getUuid());
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
        List<ProtectedResource> existingGaussDbResources = tpopsGaussDbService.getExistingGaussDbResources(
            ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType(), filterConditions);
        try {
            // 扫描生产端的数据信息
            log.info("start to scan resources");
            scan(environment);
        } catch (LegoCheckedException exception) {
            log.info("the status is offline");
            existingGaussDbResources.forEach(
                instance -> tpopsGaussDbService.updateResourceLinkStatus(instance.getUuid(),
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
        copy.setSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
        listResourceV2Req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        listResourceV2Req.setPageSize(size);
        listResourceV2Req.setPageNo(page);
        agentResource.setSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
        listResourceV2Req.setApplications(Lists.newArrayList(BeanTools.copy(agentResource, Application::new)));
        return listResourceV2Req;
    }

    /**
     * 生成ProtectedResource
     *
     * @param protectedResource agentResource
     * @param environment environment
     * @param isDelete 实例是否被删除
     * @return ProtectedResource 资源信息
     */
    private ProtectedResource buildProtectedResource(ProtectedResource protectedResource,
        ProtectedEnvironment environment, boolean isDelete) {
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
        resource.setVersion(protectedResource.getExtendInfo().get(TpopsGaussDBConstant.VERSION));

        String extendStatus = LinkStatusEnum.ABNORMAL.getStatus().toString();
        if (TpopsGaussDBConstant.NORMAL_VALUE_STATE.equals(
            protectedResource.getExtendInfoByKey(TpopsGaussDBConstant.EXTEND_INFO_KEY_STATE))) {
            extendStatus = LinkStatusEnum.ONLINE.getStatus().toString();
        }
        Map<String, String> extendInfo = new HashMap<>(environment.getExtendInfo());
        extendInfo.put(TpopsGaussDBConstant.EXTEND_INFO_KEY_STATE, extendStatus);
        extendInfo.put(TpopsGaussDBConstant.EXTEND_INFO_KEY_INSTANCE_STATUS, isDelete
            ? TpopsGaussDBConstant.EXTEND_INFO_VALUE_INSTANCE_OFFLINE
            : TpopsGaussDBConstant.EXTEND_INFO_VALUE_INSTANCE_ONLINE);
        extendInfo.put(TpopsGaussDBConstant.REGION, protectedResource.getExtendInfo().get(TpopsGaussDBConstant.REGION));
        extendInfo.put(TpopsGaussDBConstant.DB_VERSION, protectedResource.getExtendInfo()
            .get(TpopsGaussDBConstant.DB_VERSION));
        extendInfo.remove(ResourceConstants.IS_ALLOW_RESTORE_KEY);
        resource.setExtendInfo(extendInfo);
        log.info("Get environment name {}", environment.getName());
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

        return tpopsGaussDBAgentProvider.getSelectedAgents(agentSelectParam)
            .stream()
            .findFirst()
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "not find any agent can connect"));
    }
}
