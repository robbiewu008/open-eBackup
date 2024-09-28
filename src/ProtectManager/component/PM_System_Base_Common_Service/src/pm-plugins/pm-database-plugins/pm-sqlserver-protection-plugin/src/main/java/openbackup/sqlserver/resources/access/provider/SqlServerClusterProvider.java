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
package openbackup.sqlserver.resources.access.provider;

import static openbackup.database.base.plugin.common.DatabaseConstants.AGENTS;
import static openbackup.database.base.plugin.common.DatabaseConstants.DATABASE_ID;
import static openbackup.database.base.plugin.common.DatabaseConstants.END_POINT;
import static openbackup.database.base.plugin.common.DatabaseConstants.PAGE_SIZE;
import static openbackup.database.base.plugin.common.DatabaseConstants.PARENT_UUID;
import static openbackup.database.base.plugin.common.DatabaseConstants.ROOT_UUID;
import static openbackup.database.base.plugin.common.DatabaseConstants.SPLIT_CHAR;
import static openbackup.database.base.plugin.common.DatabaseConstants.SUB_TYPE;
import static openbackup.database.base.plugin.common.DatabaseConstants.VERSION;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 集群环境扫描类
 *
 */
@Component
@Slf4j
public class SqlServerClusterProvider extends DatabaseEnvironmentProvider {
    private final ResourceService resourceService;

    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final SqlServerBaseService sqlServerBaseService;

    /**
     * SqlServerEnvironmentProvider 构造器
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param resourceService 资源服务
     * @param protectedEnvironmentService 资源服务
     * @param sqlServerBaseService 数据库基础服务
     */
    public SqlServerClusterProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        ResourceService resourceService, ProtectedEnvironmentService protectedEnvironmentService,
        SqlServerBaseService sqlServerBaseService) {
        super(providerManager, pluginConfigManager);
        this.resourceService = resourceService;
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.sqlServerBaseService = sqlServerBaseService;
    }

    /**
     * 扫描SQL Server集群，包含集群实例下所有数据库扫描
     *
     * @param environment 受保护环境(集群环境)
     * @return 受保护环境中的资源列表
     */
    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        String clusterName = environment.getName();
        String clusterUuid = environment.getUuid();
        log.info("[SQL Server] cluster name: {}, uuid: {} scan start.", clusterName, clusterUuid);
        ProtectedEnvironment clusterToChange = new ProtectedEnvironment();
        clusterToChange.setUuid(clusterUuid);

        // 查询集群下节点列表并查询集群信息
        clusterToChange.setEndpoint(checkAndGetAgentsFromParam(environment).stream()
            .map(ProtectedEnvironment::getEndpoint)
            .collect(Collectors.joining(SPLIT_CHAR)));
        clusterToChange.setCluster(true);
        clusterToChange.setSourceType(environment.getSourceType());
        log.info("[SQL Server] cluster: {} instance scan start.", clusterName);

        // 查询集群下注册的集群实例列表
        List<ProtectedResource> clusterInstanceByClusterId = sqlServerBaseService.getResourceOfClusterByType(
            clusterUuid, ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType(), true);
        log.info("[SQL Server] cluster: {} instance size: {}.", clusterName, clusterInstanceByClusterId.size());

        // 查询集群下扫描的可用性组列表
        Map<String, ProtectedResource> availableGroupMap = new HashMap<>();
        log.info("[SQL Server] cluster: {} available group origin size: {}", clusterName, availableGroupMap.size());

        List<ProtectedResource> resources = clusterInstanceByClusterId.stream()
            .map(instance -> clusterInstanceScan(environment, instance, availableGroupMap))
            .flatMap(Collection::stream)
            .collect(Collectors.toList());
        log.info("[SQL Server] cluster: {} available group scan end size: {}, database scan end size: {}", clusterName,
            availableGroupMap.size(), resources.size());
        resources.addAll(availableGroupMap.values());
        resources.add(clusterToChange);
        log.info("[SQL Server] cluster name: {}, uuid: {} scan end resource size: {}.", clusterName, clusterUuid,
            resources.size());
        return resources;
    }

    private Map<String, String> getLocalDatabaseIdMap(String clusterId, String parentUuid) {
        Map<String, Object> agCondition = new HashMap<>();
        agCondition.put(ROOT_UUID, clusterId);
        agCondition.put(PARENT_UUID, parentUuid);
        agCondition.put(SUB_TYPE, ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        return resourceService.query(0, PAGE_SIZE, agCondition, "")
            .getRecords()
            .stream()
            .collect(Collectors.toMap(db -> db.getExtendInfo().get(DATABASE_ID), ProtectedResource::getUuid,
                (dbIdFirst, dbIdSecond) -> dbIdFirst));
    }

    private List<ProtectedResource> clusterInstanceScan(ProtectedEnvironment environment, ProtectedResource instance,
        Map<String, ProtectedResource> availableGroupMap) {
        log.info("[SQL Server] cluster: {} instance: {} uuid: {} scan start.", environment.getName(),
            instance.getName(), instance.getUuid());
        List<ProtectedEnvironment> nodes = sqlServerBaseService.getProtectedEnvironmentByResourceList(
            instance.getDependencies().get(AGENTS));
        log.info("[SQL Server] cluster: {} instance: {}, uuid: {} node size: {}", environment.getName(),
            instance.getName(), instance.getUuid(), nodes.size());
        instance.setPath(nodes.stream().map(ProtectedEnvironment::getEndpoint).collect(Collectors.joining(SPLIT_CHAR)));
        Map<String, ProtectedResource> databaseMap = new HashMap<>();
        for (ProtectedEnvironment node : nodes) {
            try {
                Map<Boolean, List<ProtectedResource>> collect = getInstanceResourceMapFromAgent(environment, instance,
                    node.getEndpoint(), node.getPort());
                if (collect.isEmpty()) {
                    continue;
                }

                // 已注册为集群实例的主机名列表
                List<String> registerInstanceHostNames = getHostNameListOfClusterInstance(environment.getUuid());
                log.info("[SQL Server] cluster: {} all register instance host name size: {}", environment.getName(),
                    registerInstanceHostNames.size());

                // 处理可用性组列表
                Optional.ofNullable(collect.get(true))
                    .orElse(Collections.emptyList())
                    .forEach(resource -> dealWithAvailableGroupResource(environment, instance, availableGroupMap,
                        registerInstanceHostNames, resource));

                // 处理数据库列表
                Optional.ofNullable(collect.get(false))
                    .orElse(Collections.emptyList())
                    .stream()
                    .filter(db -> !VerifyUtil.isEmpty(db.getExtendInfoByKey(DATABASE_ID)))
                    .forEach(
                        database -> dealWithDatabase(environment, instance, databaseMap, availableGroupMap, database));
                log.info("[SQL Server] cluster: {} instance: {} uuid: {} database size: {}", environment.getName(),
                    instance.getName(), instance.getUuid(), databaseMap.size());
                break;
            } catch (FeignException | LegoCheckedException exception) {
                log.error("[SQL Server] get resource from endpoint: {},port:{} exception:", environment.getEndpoint(),
                    environment.getPort(), exception);
                throw exception;
            }
        }
        log.info("[SQL Server] cluster({}) instance({}) id: {} scan end. available group size: {}, database size: {}",
            environment.getName(), instance.getName(), instance.getUuid(), availableGroupMap.size(),
            databaseMap.size());
        ArrayList<ProtectedResource> instanceResources = new ArrayList<>(databaseMap.values());
        databaseMap.values()
            .stream()
            .findFirst()
            .ifPresent(db -> instance.setVersion(
                Optional.ofNullable(db.getExtendInfoByKey(VERSION)).orElse(StringUtils.EMPTY)));
        instanceResources.add(instance);
        return instanceResources;
    }

    private void dealWithDatabase(ProtectedEnvironment environment, ProtectedResource instance,
        Map<String, ProtectedResource> databaseMap, Map<String, ProtectedResource> availableGroupMap,
        ProtectedResource database) {
        log.info("[SQL Server] cluster: {} instance: {} uuid: {} database deal, name: {}", environment.getName(),
            instance.getName(), instance.getUuid(), database.getName());
        if (!VerifyUtil.isEmpty(databaseMap.get(database.getName()))) {
            log.info("[SQL Server] cluster: {} instance: {} uuid: {} database deal, name: {} has in add list, abandon",
                environment.getName(), instance.getName(), instance.getUuid(), database.getName());
            return;
        }
        Map<String, String> extendInfo = Optional.ofNullable(database.getExtendInfo()).orElse(new HashMap<>(0));
        String availableGroupId = extendInfo.get(SqlServerConstants.AG_ID);
        ProtectedResource agGroup = availableGroupMap.get(availableGroupId);
        if (!VerifyUtil.isEmpty(agGroup)) {
            // 添加可用性组关联
            log.info("[SQL Server] cluster: {} instance: {} database: {} deal, add database & instance to agGroup: {}",
                environment.getName(), instance.getName(), database.getName(), agGroup.getName());
            database.setParentUuid(availableGroupId);
            database.setParentName(extendInfo.get(SqlServerConstants.AG_NAME));
            Map<String, List<ProtectedResource>> dependencies = agGroup.getDependencies();
            List<ProtectedResource> databaseDependencies = dependencies.getOrDefault(SqlServerConstants.DATABASE,
                new ArrayList<>());
            database.setUuid(
                getLocalDatabaseIdMap(environment.getUuid(), availableGroupId).getOrDefault(extendInfo.get(DATABASE_ID),
                    UUIDGenerator.getUUID()));
            databaseDependencies.add(database);
            dependencies.put(SqlServerConstants.DATABASE, databaseDependencies);
        } else {
            database.setUuid(getLocalDatabaseIdMap(environment.getUuid(), instance.getUuid()).getOrDefault(
                extendInfo.get(DATABASE_ID), UUIDGenerator.getUUID()));

            // 添加集群实例关联
            database.setParentUuid(instance.getUuid());
            database.setParentName(instance.getName());
        }
        database.setRootUuid(environment.getUuid());
        database.setUserId(environment.getUserId());
        database.setAuthorizedUser(environment.getAuthorizedUser());
        database.setPath(database.getName() + SqlServerConstants.RESOURCE_NAME_SPLIT + database.getParentName()
            + SqlServerConstants.RESOURCE_NAME_SPLIT + environment.getName());
        databaseMap.put(database.getName(), database);
        log.info("[SQL Server] cluster: {}, instance: {}, dealWithDatabase end add database: {}", environment.getName(),
            instance.getName(), database.getName());
    }

    private void dealWithAvailableGroupResource(ProtectedEnvironment environment, ProtectedResource instance,
        Map<String, ProtectedResource> availableGroupMap, List<String> registerInstanceHostNames,
        ProtectedResource resource) {
        log.info("[SQL Server] deal with available group: {} to cluster: {}", resource.getName(),
            environment.getName());
        String endpoint = Optional.ofNullable(resource.getExtendInfoByKey(END_POINT)).orElse(StringUtils.EMPTY);
        if (VerifyUtil.isEmpty(endpoint)) {
            log.warn("[SQL Server] available group: {} don't have host, abandoned.", resource.getName());
            return;
        }
        List<String> hostNames = Arrays.asList(endpoint.split(SPLIT_CHAR));
        log.info("[SQL Server] available group: {} hostNames size: {}", resource.getName(), hostNames.size());

        // 已注册为集群实例的主机名列表如果不全包含可用性组的主机列表则不注册
        if (!registerInstanceHostNames.containsAll(hostNames)) {
            if (availableGroupMap.containsKey(resource.getUuid())) {
                log.warn("[SQL Server] available group: {} uuid: {} not all register to instance, will be removed.",
                    resource.getName(), resource.getUuid());
                availableGroupMap.remove(resource.getUuid());
            }
            log.warn("[SQL Server] available group: {} host size: {} not all register to instance.", resource.getName(),
                hostNames.size());
            return;
        }
        ProtectedResource agResource = availableGroupMap.getOrDefault(resource.getUuid(), resource);
        log.info("[SQL Server] add available group: {} to cluster: {}", agResource.getName(), environment.getName());
        agResource.setRootUuid(environment.getUuid());
        agResource.setParentUuid(environment.getUuid());
        agResource.setParentName(environment.getName());
        boolean isInstanceBelongToAgGroup = Arrays.stream(
                Optional.ofNullable(instance.getExtendInfoByKey(END_POINT)).orElse(StringUtils.EMPTY).split(SPLIT_CHAR))
            .anyMatch(hostNames::contains);
        if (isInstanceBelongToAgGroup) {
            Map<String, List<ProtectedResource>> instanceNewMap = new HashMap<>();
            List<ProtectedResource> instances = new ArrayList<>();
            instanceNewMap.put(SqlServerConstants.INSTANCE, instances);
            Map<String, List<ProtectedResource>> instanceMap = Optional.ofNullable(agResource.getDependencies())
                .orElse(instanceNewMap);
            boolean isUuidNoneMatch = Optional.ofNullable(instanceMap.get(SqlServerConstants.INSTANCE))
                .orElse(new ArrayList<>())
                .stream()
                .noneMatch(instanceOld -> instance.getUuid().equals(instanceOld.getUuid()));
            if (isUuidNoneMatch) {
                ProtectedResource protectedResource = new ProtectedResource();
                protectedResource.setUuid(instance.getUuid());
                instanceMap.get(SqlServerConstants.INSTANCE).add(protectedResource);
            }
            agResource.setDependencies(instanceMap);
            log.info("[SQL Server] add cluster: {} instance: {} uuid: {} to available group: {}", environment.getName(),
                instance.getName(), instance.getUuid(), agResource.getName());
        }
        agResource.setPath(agResource.getName() + SqlServerConstants.RESOURCE_NAME_SPLIT + environment.getName());
        availableGroupMap.put(resource.getUuid(), agResource);
    }

    private List<String> getHostNameListOfClusterInstance(String clusterUuid) {
        return sqlServerBaseService.getResourceOfClusterByType(clusterUuid,
                ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType(), false)
            .stream()
            .map(clusterInstance -> clusterInstance.getDependencies()
                .get(AGENTS)
                .stream()
                .map(ProtectedResource::getName)
                .collect(Collectors.toList()))
            .flatMap(Collection::stream)
            .collect(Collectors.toList());
    }

    private Map<Boolean, List<ProtectedResource>> getInstanceResourceMapFromAgent(ProtectedEnvironment environment,
        ProtectedResource instance, String endpoint, Integer port) {
        List<ProtectedResource> resourceList = sqlServerBaseService.getDatabaseInfoByAgent(environment, endpoint, port,
            instance, ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        if (VerifyUtil.isEmpty(resourceList)) {
            log.warn("[SQL Server] cluster: {} instance: {} uuid: {} resource scan result is empty.",
                environment.getName(), instance.getName(), instance.getUuid());
            return new HashMap<>(0);
        }
        log.info("[SQL Server] The cluster instance: {} scan resource from agent size: {}.", instance.getUuid(),
            resourceList.size());
        return resourceList.stream()
            .collect(Collectors.groupingBy(
                resource -> ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType().equals(resource.getSubType())));
    }

    /**
     * 受保护环境健康状态检查，
     * 状态异常抛出com.huawei.oceanprotect.data.protection.access.provider.sdk.exception.DataProtectionAccessException，
     * 并返回具体的错误码
     *
     * @param environment 受保护环境
     */
    @Override
    public void validate(ProtectedEnvironment environment) {
        log.info("[SQL Server health check] check start, environment: {}", environment.getName());
        List<ProtectedEnvironment> agents = checkAndGetAgentsFromParam(environment);
        try {
            agents.forEach(this::checkNodeOnline);
        } catch (LegoCheckedException exception) {
            log.error("[SQL Server health check] exception: ", exception);
            if (exception.getErrorCode() == CommonErrorCode.HOST_OFFLINE) {
                throw new DataProtectionAccessException(CommonErrorCode.HOST_OFFLINE, new String[] {},
                    "Protected environment is offLine!");
            }
        }
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("[SQL Server] environment check, environment: {}", environment.getName());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(environment.getName());
        if (ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType().equals(environment.getSubType())) {
            return;
        }
        List<ProtectedEnvironment> agents = checkAndGetAgentsFromParam(environment);
        environment.setEndpoint(
            agents.stream().map(ProtectedEnvironment::getEndpoint).collect(Collectors.joining(SPLIT_CHAR)));

        // 校验集群环境是否存在主机已经被创建，避免重复或者交叉创建
        checkClusterIsExists(environment);
        agents.forEach(this::checkNodeOnline);
        sqlServerBaseService.checkAllNodesBelongToCluster(agents);
        environment.setUuid(UUIDGenerator.getUUID());
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    private List<ProtectedEnvironment> checkAndGetAgentsFromParam(ProtectedEnvironment environment) {
        List<ProtectedResource> agents = Optional.ofNullable(environment.getDependencies())
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.ERR_PARAM, "[SQL Server] cluster agent is empty."))
            .get(AGENTS);
        if (agents == null || agents.size() <= 1) {
            throw new LegoCheckedException(DatabaseErrorCode.CLUSTER_NODE_NUMBER_ERROR,
                "[SQL Server] cluster node number error.");
        }
        return sqlServerBaseService.getProtectedEnvironmentByResourceList(agents);
    }

    private void checkNodeOnline(ProtectedEnvironment protectedEnvironment) {
        if (LinkStatusEnum.OFFLINE.getStatus()
            .toString()
            .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment))) {
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "[SQL Server] cluster agent is offLine!");
        }
    }

    private void checkClusterIsExists(ProtectedEnvironment environment) {
        HashMap<String, Object> condition = new HashMap<>();
        condition.put(SUB_TYPE, ResourceSubTypeEnum.SQL_SERVER_CLUSTER.getType());
        PageListResponse<ProtectedResource> clusterResult = resourceService.query(0, Integer.MAX_VALUE, condition);
        if (Objects.isNull(clusterResult) || CollectionUtils.isEmpty(clusterResult.getRecords())) {
            return;
        }
        List<String> hostIds = environment.getDependencies()
            .get(AGENTS)
            .stream()
            .map(ResourceBase::getUuid)
            .collect(Collectors.toList());
        clusterResult.getRecords()
            .stream()
            .filter(cluster -> !cluster.getUuid().equals(environment.getUuid()))
            .forEach(cluster -> checkClusterHostExists(hostIds, cluster));
    }

    private void checkClusterHostExists(List<String> hostIds, ProtectedResource cluster) {
        protectedEnvironmentService.getEnvironmentById(cluster.getUuid())
            .getDependencies()
            .get(AGENTS)
            .forEach(agent -> {
                if (hostIds.contains(agent.getUuid())) {
                    log.info("[SQL Server] Host uuid:{} is already in sql server cluster uuid: {}", agent.getUuid(),
                        cluster.getUuid());
                    throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                        "[SQL Server] Host is already in sql server cluster!");
                }
            });
    }

    /**
     * detect object applicable
     *
     * @param resourceSubType 资源子类型
     * @return detect result
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return Arrays.asList(ResourceSubTypeEnum.SQL_SERVER_CLUSTER.getType(),
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType()).contains(resourceSubType);
    }
}
