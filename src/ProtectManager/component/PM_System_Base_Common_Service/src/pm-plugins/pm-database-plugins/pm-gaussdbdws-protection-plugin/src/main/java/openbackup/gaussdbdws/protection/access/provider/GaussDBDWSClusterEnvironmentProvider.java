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
package openbackup.gaussdbdws.protection.access.provider;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import io.jsonwebtoken.lang.Collections;
import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.LockedValueEnum;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.gaussdbdws.protection.access.util.DwsTaskEnvironmentUtil;
import openbackup.gaussdbdws.protection.access.util.DwsValidator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * GaussDbDws资源相关接口的具体实现类
 * 实现了：扫描，健康状态检查，资源浏览，环境信息检查相关等接口
 *
 */
@Slf4j
@Component
public class GaussDBDWSClusterEnvironmentProvider extends DatabaseEnvironmentProvider {
    /**
     * DWS_CLUSTER_STATUS 转换值
     */
    private static final Map<String, String> DWS_CLUSTER_STATUS = ImmutableMap.of(
        DwsConstant.EXTEND_INFO_NORMAL_VALUE_STATE, LinkStatusEnum.ONLINE.getStatus().toString(),
        DwsConstant.EXTEND_INFO_UNAVAILABLE_VALUE_STATE, LinkStatusEnum.UNAVAILABLE.getStatus().toString(),
        DwsConstant.EXTEND_INFO_DEGRADED_VALUE_STATE, LinkStatusEnum.DEGRADED.getStatus().toString(),
        DwsConstant.EXTEND_INFO_UNSTARTED_VALUE_STATE, LinkStatusEnum.UNSTARTED.getStatus().toString());

    private final ResourceService resourceService;

    private final UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    private final AgentUnifiedService agentUnifiedService;

    @Autowired
    private GaussDBBaseService gaussDBBaseService;

    /**
     * GaussDBDWSClusterEnvironmentProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param clusterIntegrityChecker clusterIntegrityChecker
     */
    public GaussDBDWSClusterEnvironmentProvider(ProviderManager providerManager,
        PluginConfigManager pluginConfigManager, ResourceService resourceService,
        @Qualifier("unifiedClusterResourceIntegrityChecker")
            UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker, AgentUnifiedService agentUnifiedService) {
        super(providerManager, pluginConfigManager);
        this.resourceService = resourceService;
        this.clusterIntegrityChecker = clusterIntegrityChecker;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GAUSSDB_DWS.getType().equals(object);
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        List<ProtectedResource> clusterResources = getAgentResourcesByKey(environment, DwsConstant.DWS_CLUSTER_AGENT);
        ProtectedEnvironment protectedEnvironment = getClusterUuid(clusterResources);
        ProtectedResource protectedResource = getProtectedResource(environment);
        protectedResource.setSubType(ResourceSubTypeEnum.GAUSSDB_DWS.getType());
        protectedResource.setEnvironment(protectedEnvironment);
        AppEnvResponse appEnvResponse = clusterIntegrityChecker.generateCheckResult(protectedResource).getData();
        if (!LinkStatusEnum.ONLINE.getStatus()
            .toString()
            .equals(DWS_CLUSTER_STATUS.get(
                appEnvResponse.getNodes().get(0).getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_STATE)))) {
            log.error("dws cluster {} not online", protectedResource.getName());
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                "dws cluster " + protectedResource.getName() + " not online");
        }
        String dataBases = appEnvResponse.getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_DATABASES);
        return Arrays.stream(dataBases.split(","))
            .map(database -> getDatabaseResource(environment, database))
            .collect(Collectors.toList());
    }

    private ProtectedEnvironment getDependencyEnvironment(List<ProtectedResource> clusterResources) {
        return clusterResources.stream()
            .map(ProtectedResource::getUuid)
            .map(this::getEnvironmentById)
            .findFirst()
            .orElse(new ProtectedEnvironment());
    }

    private ProtectedResource getDatabaseResource(ProtectedEnvironment environment, String database) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setEnvironment(environment);
        protectedResource.setName(database);
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType());
        protectedResource.setUuid(UUID.nameUUIDFromBytes(
            (environment.getUuid() + database).getBytes(StandardCharsets.UTF_8)).toString());
        protectedResource.setVersion(environment.getVersion());
        protectedResource.setParentName(environment.getName());
        protectedResource.setParentUuid(environment.getUuid());
        protectedResource.setRootUuid(environment.getUuid());
        protectedResource.setPath(environment.getEndpoint());
        return protectedResource;
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("dws start cluster update name: {}, uuid: {}", environment.getName(), environment.getUuid());
        // 判断name是否为空
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(environment.getName());
        //  envFile 为空 拦截报错;
        DwsValidator.checkDwsValue(Optional.ofNullable(environment.getExtendInfo())
            .orElse(new HashMap<>())
            .get(DwsConstant.EXTEND_INFO_KEY_ENV_FILE));
        //  用户名  为空 拦截报错;
        DwsValidator.checkDwsValue(
            Optional.ofNullable(environment.getAuth()).orElse(new Authentication()).getAuthKey());

        // 获取已注册成功的DWS资源信息
        List<ProtectedEnvironment> existingEnvironments = getExistingDwsResources();
        // 判断 是否存在重复的集群节点或者代理主机
        checkDuplicateClusterAndHost(existingEnvironments, environment);

        ProtectedResource protectedResource = getProtectedResource(environment);

        // 通过不同集群节点调用完成资源查询, CentralCoordinator IP 是否完全一致;
        List<ProtectedResource> clusterResources = getAgentResourcesByKey(environment, DwsConstant.DWS_CLUSTER_AGENT);
        //  集群节点 为空 拦截报错;
        DwsValidator.checkDwsValue((clusterResources.size() == 0) ? "" : DwsConstant.DWS_CLUSTER_AGENT);
        gaussDBBaseService.checkConnention(protectedResource);

        AppEnvResponse appEnvResponse = getCentralCoordinator(clusterResources, protectedResource, environment);
        // 检查是否为我们不需要版本
        List<ProtectedResource> hostResources = getAgentResourcesByKey(environment, DwsConstant.HOST_AGENT);
        String version = appEnvResponse.getNodes().get(0).getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_VERSION);
        checkVersion(hostResources, version);
        // 设置版本信息;
        environment.setVersion(version);
        // 规格检查 是否大于8个
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            DwsValidator.checkDwsCount(existingEnvironments);
            DwsValidator.checkDwsResourceExistById(existingEnvironments, environment.getUuid());
            DwsValidator.checkDwsResourceExistByEndpoint(existingEnvironments, environment.getEndpoint());
            environment.setUuid(Optional.ofNullable(environment.getUuid()).orElse(UUIDGenerator.getUUID()));
        }

        // 添加对象信息位置
        environment.setPath(environment.getEndpoint());
        checkCnNodes(clusterResources, appEnvResponse);
        // 检查通过后，添加数据到environment中，由框架负责持久化
        String linkStatus = DWS_CLUSTER_STATUS.get(
            appEnvResponse.getNodes().get(0).getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_STATE));
        environment.setLinkStatus(linkStatus);

        // 设置为集群;
        environment.setCluster(true);

        log.info("dws cluster update name: {},linkStatus: {},version: {},", environment.getName(), linkStatus,
            environment.getVersion());
    }

    private void checkCnNodes(List<ProtectedResource> clusterResources, AppEnvResponse appEnvResponse) {
        if (ObjectUtils.isEmpty(appEnvResponse.getExtendInfo())) {
            return;
        }
        if (StringUtils.isEmpty(appEnvResponse.getExtendInfo().get(DwsConstant.CN_NODES))) {
            log.warn("cn nodes not find");
            return;
        }
        List<String> cnNodes = Arrays.stream(appEnvResponse.getExtendInfo().get(DwsConstant.CN_NODES).split(","))
            .collect(Collectors.toList());
        List<String> clusterAgentUuidList = clusterResources.stream()
            .map(ProtectedResource::getUuid)
            .map(gaussDBBaseService::getEnvironmentById)
            .filter(env -> LinkStatusEnum.ONLINE.getStatus()
                .toString()
                .equals(env.getLinkStatus()))
            .filter(env -> {
                String ips = DwsTaskEnvironmentUtil.getIps(env);
                return cnNodes.stream().anyMatch(ips::contains);
            })
            .map(ProtectedEnvironment::getUuid)
            .collect(Collectors.toList());
        if (clusterAgentUuidList.size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent network error");
        }
    }

    private void checkDuplicateClusterAndHost(List<ProtectedEnvironment> existingEnvironments,
        ProtectedEnvironment protectedResource) {
        List<String> clusterUuidList = Optional.ofNullable(
            protectedResource.getDependencies().get(DwsConstant.DWS_CLUSTER_AGENT))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        List<String> hostUuidList = Optional.ofNullable(protectedResource.getDependencies().get(DwsConstant.HOST_AGENT))
            .orElse(new ArrayList<>())
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        List<ProtectedEnvironment> existingEnvironmentList = existingEnvironments.stream()
            .map(ProtectedEnvironment::getUuid)
            .filter(clusterUuid -> !clusterUuid.equals(protectedResource.getUuid()))
            .map(gaussDBBaseService::getEnvironmentById)
            .collect(Collectors.toList());
        clusterUuidList.forEach(uuid -> DwsValidator.checkExistUuid(hostUuidList, uuid));
        DwsValidator.checkDwsExistSameClusterOrHost(existingEnvironmentList, clusterUuidList);
        DwsValidator.checkDwsExistSameHostAsCluster(existingEnvironmentList, hostUuidList);
    }

    private List<ProtectedResource> getAgentResourcesByKey(ProtectedEnvironment environment, String agentKey) {
        Map<String, List<ProtectedResource>> dependencies = environment.getDependencies();
        // 检查集群节点是否为内置代理
        return dependencies.get(agentKey);
    }

    private AppEnvResponse getCentralCoordinator(List<ProtectedResource> clusterResources,
        ProtectedResource protectedResource, ProtectedEnvironment environment) {
        ProtectedEnvironment protectedEnvironment = getClusterUuid(clusterResources);
        protectedResource.setEnvironment(protectedEnvironment);
        CheckResult<AppEnvResponse> appEnvResponseCheckResult = clusterIntegrityChecker.generateCheckResult(
            protectedResource);
        AppEnvResponse appEnv = appEnvResponseCheckResult.getData();
        if (ObjectUtils.isEmpty(appEnvResponseCheckResult.getData())
            || appEnvResponseCheckResult.getData().getNodes() == null) {
            log.error("The GaussDB-DWS cluster nodes query failed.");
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "The GaussDB-DWS cluster nodes query failed.");
        }
        String coordinatorIp = appEnv.getNodes().get(0).getExtendInfo().get(DwsConstant.COORDINATOR_IP);
        environment.setEndpoint(coordinatorIp);
        log.info("coordinatorIp: {}", coordinatorIp);
        return appEnv;
    }

    private ProtectedEnvironment getClusterUuid(List<ProtectedResource> clusterResources) {
        List<String> clusterUuids = Optional.of(clusterResources)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        for (String clusterUuid : clusterUuids) {
            ProtectedEnvironment environmentById = getEnvironmentById(clusterUuid);
            if (LinkStatusEnum.OFFLINE.getStatus().toString()
                    .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(environmentById))) {
                continue;
            }
            return environmentById;
        }
        throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "name is not exist");
    }

    private ProtectedEnvironment getEnvironmentById(String envId) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceByIdIgnoreOwner(envId);
        if (resOptional.isPresent() && resOptional.get() instanceof ProtectedEnvironment) {
            return (ProtectedEnvironment) resOptional.get();
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!");
    }

    private void checkVersion(List<ProtectedResource> clusterResources, String version) {
        if (!Collections.isEmpty(clusterResources) && DwsConstant.HOST_NO_ADD_VERSION.equals(version)) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_BUSY, "cluster Resources is not inside agent.");
        }
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

    /**
     * 获取已存在的DWS资源信息
     *
     * @return 已存在的HBase资源信息
     */
    private List<ProtectedEnvironment> getExistingDwsResources() {
        Map<String, Object> filter = new HashMap<>();
        filter.put("type", ResourceTypeEnum.DATABASE.getType());
        filter.put("subType", ResourceSubTypeEnum.GAUSSDB_DWS.getType());
        List<ProtectedResource> existingResources = resourceService.query(0, LegoNumberConstant.TWENTY, filter)
            .getRecords();
        // 转成ProtectedEnvironment环境对象
        return existingResources.stream()
            .filter(existingResource -> existingResource instanceof ProtectedEnvironment)
            .map(existingResource -> (ProtectedEnvironment) existingResource)
            .collect(Collectors.toList());
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        gaussDBBaseService.checkLinkStatus(environment.getUuid());
        List<ProtectedResource> clusterResources = getAgentResourcesByKey(environment, DwsConstant.DWS_CLUSTER_AGENT);
        ProtectedEnvironment protectedEnvironment = getClusterUuid(clusterResources);
        PageListResponse<ProtectedResource> detailPageList = agentUnifiedService.getDetailPageListNoRetry(
            environmentConditions.getResourceType(), protectedEnvironment.getEndpoint(), protectedEnvironment.getPort(),
            getListResourceReq(protectedEnvironment, environment, environmentConditions), false);
        convertTableResourceList(environment.getUuid(), environmentConditions.getResourceType(), detailPageList);

        return detailPageList;
    }

    private void convertTableResourceList(String uuid, String resourceType,
        PageListResponse<ProtectedResource> detailPageList) {
        Map<String, ProtectedResource> detailPageMap = new HashMap<>();
        detailPageList.getRecords()
            .forEach(protectedResources -> detailPageMap.put(protectedResources.getName(), protectedResources));

        PageListResponse<ProtectedResource> records = resourceService.query(LegoNumberConstant.ZERO,
            LegoNumberConstant.ONE, ImmutableMap.of(DatabaseConstants.ROOT_UUID, uuid, "subType", resourceType));
        int count = records.getTotalCount() / DwsConstant.QUERY_QUANTITY_SPECIFICATIONS;
        for (int size = 0; size <= count; size++) {
            PageListResponse<ProtectedResource> environments = resourceService.query(size,
                DwsConstant.QUERY_QUANTITY_SPECIFICATIONS,
                ImmutableMap.of(DatabaseConstants.ROOT_UUID, uuid, "subType", resourceType));
            addLockedStatus(detailPageMap, environments, resourceType);
        }
    }

    private void addLockedStatus(Map<String, ProtectedResource> detailPageMap,
        PageListResponse<ProtectedResource> environments, String subType) {
        List<List<String>> existResourceTableInfo = environments.getRecords()
            .stream()
            .map(ProtectedResource::getExtendInfo)
            .map(extendInfo -> extendInfo.get(DwsConstant.EXTEND_INFO_KEY_TABLE))
            .map(tableInfo -> tableInfo.split(","))
            .map(Arrays::asList)
            .collect(Collectors.toList());

        for (String resourceName : detailPageMap.keySet()) {
            ProtectedResource protectedResource = detailPageMap.get(resourceName);
            protectedResource.setExtendInfoByKey(DatabaseConstants.EXTEND_INFO_KEY_IS_LOCKED,
                addResourceLock(existResourceTableInfo, resourceName, subType));
        }
    }

    private String addResourceLock(List<List<String>> existResourceTableInfo, String resourceName, String subType) {
        if (ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType().equals(subType)) {
            return LockedValueEnum.NO_OPTIONAL.getLocked();
        }
        for (List<String> tableInfo : existResourceTableInfo) {
            if (tableInfo.contains(resourceName)) {
                return LockedValueEnum.OPTIONAL.getLocked();
            }
        }
        return LockedValueEnum.NO_OPTIONAL.getLocked();
    }

    private ListResourceV2Req getListResourceReq(ProtectedEnvironment env, ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        AppEnv appEnv = BeanTools.copy(env, AppEnv::new);
        Application application = new Application();
        application.setAuth(environment.getAuth());
        application.setName(environmentConditions.getParentId());
        application.setType(environment.getType());
        application.setSubType(environmentConditions.getResourceType());
        application.setExtendInfo(environment.getExtendInfo());

        ListResourceV2Req req = new ListResourceV2Req();
        req.setPageNo(environmentConditions.getPageNo());
        req.setPageSize(environmentConditions.getPageSize());
        req.setAppEnv(appEnv);
        req.setConditions(environmentConditions.getConditions());
        req.setApplications(Lists.newArrayList(application));
        return req;
    }

    /**
     * 受保护环境健康状态检查, 返回连接状态
     *
     * @param environment 受保护环境
     * @return LinkStatusEnum
     */
    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        ProtectedResource protectedResource = getProtectedResource(environment);

        // 通过不同集群节点调用完成资源查询, CentralCoordinator IP 是否完全一致;
        List<ProtectedResource> clusterResources = getAgentResourcesByKey(environment, DwsConstant.DWS_CLUSTER_AGENT);
        AppEnvResponse appEnvResponse = getCentralCoordinator(clusterResources, protectedResource, environment);
        checkCnNodes(clusterResources, appEnvResponse);
        // 检查通过后，添加数据到environment中，由框架负责持久化
        String linkStatus = DWS_CLUSTER_STATUS.get(
            appEnvResponse.getNodes().get(0).getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_STATE));
        return Optional.ofNullable(linkStatus);
    }
}