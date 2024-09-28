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
package openbackup.database.base.plugin.service.impl;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 数据库集群环境服务
 *
 */
@Service
@Slf4j
public class ClusterEnvironmentServiceImpl implements ClusterEnvironmentService {
    private final ResourceService resourceService;

    public ClusterEnvironmentServiceImpl(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    @Override
    public void checkClusterNodeNum(List<ProtectedResource> agents) {
        Set<String> agentSet = agents.stream().map(ProtectedResource::getUuid).collect(Collectors.toSet());

        // check重复节点
        if (agents.size() != agentSet.size()) {
            log.error("There are duplicate hosts.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "There are duplicate hosts.");
        }

        // check集群节点个数
        if (agents.size() < IsmNumberConstant.TWO) {
            log.error("The number of nodes in the cluster is less than two.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Select cluster node num is error.");
        }
    }

    @Override
    public void checkClusterNodeStatus(List<ProtectedEnvironment> environments) {
        boolean isOffline = environments.stream()
            .anyMatch(env -> LinkStatusEnum.OFFLINE.getStatus().toString()
            .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env)));
        if (isOffline) {
            log.error("Select host is offLine.");
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "Select host is offLine.");
        }
    }

    @Override
    public void checkClusterNodeOsType(List<ProtectedEnvironment> environments) {
        Set<String> osTypeSet = environments.stream().map(ProtectedEnvironment::getOsType).collect(Collectors.toSet());
        if (osTypeSet.size() > IsmNumberConstant.ONE) {
            log.error("The operating system types of cluster nodes are inconsistent.");
            throw new LegoCheckedException(DatabaseErrorCode.DATABASE_OS_INCONSISTENT,
                "Select host os type inconsistencies.");
        }
    }

    @Override
    public void checkRegisterNodeIsRegistered(ProtectedResource resource) {
        List<String> newNodes = resource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        checkNodeIsRegistered(newNodes, resource.getSubType());
    }

    private void checkNodeIsRegistered(List<String> newNodes, String subType) {
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.SUB_TYPE, subType);
        List<ProtectedResource> resourceList = new ArrayList<>();
        PageListResponse<ProtectedResource> data;
        int pageNo = 0;
        do {
            data = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, filter);
            data.getRecords().forEach(resource -> checkNodes(resource, newNodes));
            pageNo++;
        } while (data.getRecords().size() >= IsmNumberConstant.HUNDRED);
    }

    private void checkNodes(ProtectedResource resource, List<String> newNodes) {
        ProtectedResource registeredResource = getResourceById(resource.getUuid());
        List<String> registeredNodes = registeredResource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(ResourceBase::getUuid)
            .collect(Collectors.toList());
        if (!Collections.disjoint(newNodes, registeredNodes)) {
            log.error("The select host has been registered. uuids: {}", newNodes);
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED, "The host has been registered.");
        }
    }

    private ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This resource is not exists."));
    }

    @Override
    public void checkUpdateNodeIsRegistered(ProtectedResource resource) {
        ProtectedResource oldCluster = getResourceById(resource.getUuid());
        List<String> oldNodes = oldCluster.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        List<String> newNodes = resource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        newNodes.removeAll(oldNodes);
        if (newNodes.isEmpty()) {
            log.info("The database cluster node is not modified. uuids: {}", oldNodes);
            return;
        }
        checkNodeIsRegistered(newNodes, resource.getSubType());
    }

    @Override
    public void checkClusterIsRegisteredInstance(ProtectedEnvironment environment) {
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.PARENT_UUID, environment.getUuid());
        int instanceNum = resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.ONE, filter).getTotalCount();
        if (instanceNum > IsmNumberConstant.ZERO) {
            log.error("The cluster has registered instances and cannot be modified. uuid: {}", environment.getUuid());
            throw new LegoCheckedException(CommonErrorCode.DB_CLUSTER_HAS_INSTANCE,
                "The cluster has registered instances and cannot be modified.");
        }
    }

    @Override
    public void checkClusterNodeCountLimit(int agentsNum, int clusterMaxNodeCount) {
        if (agentsNum > clusterMaxNodeCount) {
            log.error("Select cluster node number error. max num: {}, agents num: {}", clusterMaxNodeCount, agentsNum);
            throw new LegoCheckedException(DatabaseErrorCode.CLUSTER_NODE_NUMBER_ERROR,
                new String[] {String.valueOf(clusterMaxNodeCount)}, "Select cluster node number error.");
        }
    }
}
