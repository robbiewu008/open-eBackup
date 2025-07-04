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
package openbackup.db2.protection.access.provider.resource;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.enums.ClusterInstanceOnlinePolicy;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2InstanceService;
import openbackup.db2.protection.access.service.Db2TablespaceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.StreamUtil;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * db2集群provider
 *
 */
@Component
@Slf4j
public class Db2ClusterProvider implements EnvironmentProvider {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final ClusterEnvironmentService clusterEnvironmentService;

    private final ResourceService resourceService;

    private Db2TablespaceService db2TablespaceService;

    private Db2InstanceService db2instanceService;

    private InstanceResourceService instanceResourceService;

    public Db2ClusterProvider(ProtectedEnvironmentService protectedEnvironmentService,
        ClusterEnvironmentService clusterEnvironmentService, ResourceService resourceService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.clusterEnvironmentService = clusterEnvironmentService;
        this.resourceService = resourceService;
    }

    @Autowired
    public void setDb2TablespaceService(Db2TablespaceService db2TablespaceService) {
        this.db2TablespaceService = db2TablespaceService;
    }

    @Autowired
    public void setDb2InstanceService(Db2InstanceService db2InstanceService) {
        this.db2instanceService = db2InstanceService;
    }

    @Autowired
    public void setInstanceResourceService(InstanceResourceService instanceResourceService) {
        this.instanceResourceService = instanceResourceService;
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("Start check db2 cluster. name: {}", environment.getName());
        List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
        clusterEnvironmentService.checkClusterNodeNum(agents);
        checkClusterNodeCountLimit(environment.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE), agents.size());
        List<ProtectedEnvironment> agentList = queryEnvironments(agents);
        clusterEnvironmentService.checkClusterNodeStatus(agentList);
        clusterEnvironmentService.checkClusterNodeOsType(agentList);
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            checkClusterCount();
            checkDb2RegisterEnvironment(environment);
            setDb2ClusterUuid(environment);
        } else {
            checkDb2UpdateEnvironment(environment);
        }
        setDb2Cluster(environment, agentList);
        log.info("End check db2 cluster. name: {}", environment.getName());
    }

    private void checkClusterCount() {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.DB2_CLUSTER.getType());
        int clusterCount = resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.TWO_HUNDRED, conditions)
            .getTotalCount();
        if (clusterCount >= Db2Constants.DB2_CLUSTER_MAX_COUNT) {
            log.error("Db2 cluster register exceed max count: {}. register cluster count: {}",
                Db2Constants.DB2_CLUSTER_MAX_COUNT, clusterCount);
            throw new LegoCheckedException(DatabaseErrorCode.RESOURCE_REACHED_THE_UPPER_LIMIT,
                new String[] {String.valueOf(Db2Constants.DB2_CLUSTER_MAX_COUNT)},
                "Db2 cluster register exceed max count.");
        }
    }

    private void checkClusterNodeCountLimit(String clusterType, int agentsNum) {
        Db2ClusterTypeEnum clusterTypeEnum = Db2ClusterTypeEnum.getByType(clusterType);
        switch (clusterTypeEnum) {
            case DPF:
                clusterEnvironmentService.checkClusterNodeCountLimit(agentsNum, Db2Constants.DPF_NODE_MAX_COUNT);
                break;
            case POWER_HA:
            case RHEL_HA:
                clusterEnvironmentService.checkClusterNodeCountLimit(agentsNum, Db2Constants.POWER_HA_NODE_MAX_COUNT);
                break;
            case HADR:
                clusterEnvironmentService.checkClusterNodeCountLimit(agentsNum, Db2Constants.HADR_NODE_MAX_COUNT);
                break;
            default:
                break;
        }
    }

    private List<ProtectedEnvironment> queryEnvironments(List<ProtectedResource> agents) {
        return agents.stream()
            .map(host -> protectedEnvironmentService.getEnvironmentById(host.getUuid()))
            .collect(Collectors.toList());
    }

    private void setDb2ClusterUuid(ProtectedEnvironment environment) {
        environment.setUuid(UUIDGenerator.getUUID());
    }

    private void checkDb2RegisterEnvironment(ProtectedEnvironment environment) {
        clusterEnvironmentService.checkRegisterNodeIsRegistered(environment);
    }

    private void checkDb2UpdateEnvironment(ProtectedEnvironment environment) {
        clusterEnvironmentService.checkClusterIsRegisteredInstance(environment);
        clusterEnvironmentService.checkUpdateNodeIsRegistered(environment);
    }

    private void setDb2Cluster(ProtectedEnvironment environment, List<ProtectedEnvironment> envs) {
        environment.setEndpoint(envs.stream()
            .map(ProtectedEnvironment::getEndpoint)
            .collect(Collectors.joining(DatabaseConstants.SPLIT_CHAR)));
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        boolean isOffline;

        // HA集群主节点在线，即为在线
        if (Db2ClusterTypeEnum.POWER_HA.getType().equals(environment.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE))
            || Db2ClusterTypeEnum.RHEL_HA.getType()
            .equals(environment.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE))) {
            isOffline = validateHaCluster(environment);
        } else {
            // 除HA集群外，所有节点都在线即为在线
            instanceResourceService.healthCheckClusterInstanceOfEnvironment(environment,
                ClusterInstanceOnlinePolicy.ALL_NODES_ONLINE);
            List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);

            isOffline = agents.stream()
                .flatMap(StreamUtil.match(ProtectedEnvironment.class))
                .anyMatch(childNode -> LinkStatusEnum.OFFLINE.getStatus()
                    .toString()
                    .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(childNode)));
        }
        if (isOffline) {
            log.error("The db2 cluster health check fail. uuid: {}", environment.getUuid());
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "Cluster host is offLine.");
        }
    }

    private boolean validateHaCluster(ProtectedEnvironment environment) {
        // 检查集群下的集群实例的在线状态
        List<ProtectedResource> clusterInstance = getProtectedResources(environment);
        clusterInstance.forEach(this::healthCheckHaClusterInstanceByAgent);

        // 检查集群实例下的单实例在线状态
        instanceResourceService.healthCheckSubInstance(environment);

        // 集群关联的代理主机有一台在线，即为在线
        List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
        return agents.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .allMatch(childNode -> LinkStatusEnum.OFFLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(childNode)));
    }

    private void healthCheckHaClusterInstanceByAgent(ProtectedResource resource) {
        try {
            ProtectedResource fullResource = resourceService.getResourceById(resource.getUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This resource not exist."));
            ProtectedResource primaryNode = db2instanceService.queryHadrPrimaryNode(fullResource);
            instanceResourceService.healthCheckSingleInstanceByAgent(primaryNode, getEnvironment(primaryNode));
            resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                LinkStatusEnum.ONLINE.getStatus().toString());
        } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
            log.error("This sub resource health check is fail.", e);
            resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                LinkStatusEnum.OFFLINE.getStatus().toString());
        } finally {
            instanceResourceService.updateResourceStatus(resource);
        }
    }

    private ProtectedEnvironment getEnvironment(ProtectedResource protectedResource) {
        return protectedResource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This environment not exist."));
    }

    private List<ProtectedResource> getProtectedResources(ProtectedEnvironment environment) {
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
        return resources;
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        log.info("start db2 cluster instance scan. id:{}", environment.getUuid());
        List<ProtectedResource> clusterInstances = queryInstancesByEnvironment(environment.getUuid());
        List<ProtectedResource> resources = new ArrayList<>();
        clusterInstances.forEach(clusterInstance -> {
            resources.addAll(db2instanceService.scanDatabase(clusterInstance, environment));
        });
        log.info("End db2 cluster instance scan. id:{}, size: {}", environment.getUuid(), resources.size());
        return resources;
    }

    private List<ProtectedResource> queryInstancesByEnvironment(String environmentId) {
        Map<String, Object> filters = new HashMap<>();
        filters.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.DB2_CLUSTER_INSTANCE.getType());
        filters.put(DatabaseConstants.PARENT_UUID, environmentId);
        List<ProtectedResource> instances = new ArrayList<>();
        int pageNo = 0;
        PageListResponse<ProtectedResource> resources;
        do {
            resources = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, filters);
            instances.addAll(resources.getRecords());
            pageNo++;
        } while (resources.getRecords().size() >= IsmNumberConstant.HUNDRED);
        instances.forEach(item -> {
            Optional<ProtectedResource> resource = resourceService.getResourceById(item.getUuid());
            resource.ifPresent(protectedResource -> item.setDependencies(protectedResource.getDependencies()));
        });
        return instances;
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        log.info("Start browse db2 cluster tablespace. database id: {}", environmentConditions.getParentId());
        PageListResponse<ProtectedResource> detailPageList = db2TablespaceService.queryClusterTablespace(environment,
            environmentConditions);
        db2TablespaceService.setTablespaceLockedStatus(environmentConditions.getParentId(), detailPageList);
        log.info("End browse db2 cluster tablespace. database id: {}, size: {}", environmentConditions.getParentId(),
            detailPageList.getRecords().size());
        return detailPageList;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.DB2_CLUSTER.equalsSubType(resourceSubType);
    }
}
