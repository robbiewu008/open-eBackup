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
package openbackup.kingbase.protection.access.provider.resource;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.ClusterInstanceOnlinePolicy;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.StreamUtil;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.stream.Collectors;

/**
 * kingbase集群provider
 *
 */
@Component
@Slf4j
public class KingBaseClusterProvider implements EnvironmentProvider {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final ClusterEnvironmentService clusterEnvironmentService;

    private final InstanceResourceService instanceResourceService;

    public KingBaseClusterProvider(ProtectedEnvironmentService protectedEnvironmentService,
        ClusterEnvironmentService clusterEnvironmentService, InstanceResourceService instanceResourceService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.clusterEnvironmentService = clusterEnvironmentService;
        this.instanceResourceService = instanceResourceService;
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("Start check kingbase cluster. name: {}", environment.getName());
        List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
        clusterEnvironmentService.checkClusterNodeNum(agents);
        List<ProtectedEnvironment> agentList = queryEnvironments(agents);
        clusterEnvironmentService.checkClusterNodeStatus(agentList);
        clusterEnvironmentService.checkClusterNodeOsType(agentList);
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            checkKingBaseRegisterEnvironment(environment);
            setKingBaseClusterUuid(environment);
        } else {
            checkKingBaseUpdateEnvironment(environment);
        }
        setKingBaseCluster(environment, agentList);
        log.info("End check kingbase cluster. name: {}", environment.getName());
    }

    private void setKingBaseClusterUuid(ProtectedEnvironment environment) {
        environment.setUuid(UUIDGenerator.getUUID());
    }

    private void setKingBaseCluster(ProtectedEnvironment environment, List<ProtectedEnvironment> envs) {
        environment.setEndpoint(envs.stream()
            .map(ProtectedEnvironment::getEndpoint)
            .collect(Collectors.joining(DatabaseConstants.SPLIT_CHAR)));
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    private void checkKingBaseRegisterEnvironment(ProtectedEnvironment environment) {
        // check注册的集群节点是否被注册
        clusterEnvironmentService.checkRegisterNodeIsRegistered(environment);
    }

    private List<ProtectedEnvironment> queryEnvironments(List<ProtectedResource> agents) {
        return agents.stream()
            .map(host -> protectedEnvironmentService.getEnvironmentById(host.getUuid()))
            .collect(Collectors.toList());
    }

    private void checkKingBaseUpdateEnvironment(ProtectedEnvironment environment) {
        // check集群是否注册实例
        clusterEnvironmentService.checkClusterIsRegisteredInstance(environment);

        // check更新的集群节点是否被注册
        clusterEnvironmentService.checkUpdateNodeIsRegistered(environment);
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        instanceResourceService.healthCheckClusterInstanceOfEnvironment(environment,
            ClusterInstanceOnlinePolicy.ALL_NODES_ONLINE);
        List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
        boolean isOffline = agents.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .anyMatch(childNode -> LinkStatusEnum.OFFLINE.getStatus()
            .toString().equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(childNode)));
        if (isOffline) {
            log.error("The database cluster health check fail. uuid: {}", environment.getUuid());
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "Cluster host is offLine.");
        }
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.KING_BASE_CLUSTER.equalsSubType(resourceSubType);
    }
}
