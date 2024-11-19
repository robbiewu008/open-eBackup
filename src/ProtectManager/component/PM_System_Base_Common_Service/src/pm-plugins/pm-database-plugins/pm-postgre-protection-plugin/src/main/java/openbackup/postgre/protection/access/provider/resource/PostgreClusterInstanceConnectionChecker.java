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
package openbackup.postgre.protection.access.provider.resource;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.postgre.protection.access.common.PostgreConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * PostgreClusterInstanceConnectionChecker连接检查
 *
 */
@Slf4j
@Component
public class PostgreClusterInstanceConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final InstanceResourceService instanceResourceService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param protectedEnvironmentService 环境服务
     * @param instanceResourceService 实例资源服务
     */
    public PostgreClusterInstanceConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, ProtectedEnvironmentService protectedEnvironmentService,
        InstanceResourceService instanceResourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.instanceResourceService = instanceResourceService;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        log.info("Start collecting connectable resources.");
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(resource.getParentUuid());
        resource.setEnvironment(environment);
        resource.getExtendInfo().put(DatabaseConstants.VIRTUAL_IP, resource.getEnvironment().getEndpoint());
        try {
            String installDeployType = resource.getExtendInfo().get(PostgreConstants.INSTALL_DEPLOY_TYPE);
            if (StringUtils.equals(installDeployType, PostgreConstants.CLUP)) {
                Map<String, List<NodeInfo>> appEnvMap = queryClusterInstanceNodeRoleByClupServer(resource, environment);
                resource.getDependencies()
                    .get(DatabaseConstants.CHILDREN)
                    .forEach(childNode -> buildClusterNodeRole(childNode, appEnvMap));
            } else {
                instanceResourceService.setClusterInstanceNodeRole(resource);
            }
        } catch (LegoCheckedException | NullPointerException e) {
            log.error("Collecting postgre cluster instance check error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
        List<ProtectedResource> children = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        // 如果没有主节点直接报错
        Optional<ProtectedResource> childOptional = children.stream()
            .filter(
                item -> StringUtils.equals(PostgreConstants.PRIMARY, item.getExtendInfo().get(DatabaseConstants.ROLE)))
            .findFirst();
        if (!childOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
        return super.collectConnectableResources(resource);
    }

    private Map<String, List<NodeInfo>> queryClusterInstanceNodeRoleByClupServer(ProtectedResource resource,
        ProtectedEnvironment environment) {
        CheckAppReq checkAppReq = buildCheckAppReq(resource);
        List<ProtectedResource> clupServers = environment.getDependencies().get(PostgreConstants.CLUP_SERVERS);
        ProtectedEnvironment clupServerEnvironment = protectedEnvironmentService.getEnvironmentById(
            clupServers.get(IsmNumberConstant.ZERO).getUuid());
        AppEnvResponse clusterInstanceInfo = agentUnifiedService.getClusterInfo(
            ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType(), clupServerEnvironment, checkAppReq, false);
        String clupClusterState = clusterInstanceInfo.getExtendInfo().get("clupClusterState");
        if (StringUtils.equals(clupClusterState, PostgreConstants.CLUP_CLUSTER_OFFLINE)) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target clup cluster is offline.");
        }
        return clusterInstanceInfo.getNodes().stream().collect(Collectors.groupingBy(NodeInfo::getEndpoint));
    }

    private CheckAppReq buildCheckAppReq(ProtectedResource resource) {
        AppEnv appEnv = BeanTools.copy(resource, AppEnv::new);
        appEnv.setSubType((ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType()));
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(appEnv);
        Application application = BeanTools.copy(resource, Application::new);
        application.setSubType(resource.getSubType());
        if (VerifyUtil.isEmpty(application.getAuth())) {
            application.setAuth(resource.getAuth());
        }
        checkAppReq.setApplication(application);
        return checkAppReq;
    }

    private void buildClusterNodeRole(ProtectedResource childNode, Map<String, List<NodeInfo>> appEnvMap) {
        NodeInfo nodeInfo = appEnvMap.get(childNode.getExtendInfoByKey(DatabaseConstants.SERVICE_IP))
            .get(IsmNumberConstant.ZERO);
        Map<String, String> extendInfo = nodeInfo.getExtendInfo();
        String role = StringUtils.equals(MapUtils.getString(extendInfo, PostgreConstants.IS_PRIMARY),
            PostgreConstants.PRIMARY) ? PostgreConstants.PRIMARY : PostgreConstants.SLAVE;
        childNode.getExtendInfo().put(DatabaseConstants.ROLE, role);
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType()
            .equals(object.getSubType());
    }
}