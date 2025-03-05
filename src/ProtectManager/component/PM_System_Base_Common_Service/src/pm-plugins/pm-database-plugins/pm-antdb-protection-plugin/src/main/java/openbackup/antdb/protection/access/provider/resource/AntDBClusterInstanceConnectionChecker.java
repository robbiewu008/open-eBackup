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
package openbackup.antdb.protection.access.provider.resource;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.antdb.protection.access.common.AntDBConstants;
import openbackup.antdb.protection.access.service.AntDBInstanceService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.stream.Collectors;

/**
 * AntDBClusterInstanceConnectionChecker连接检查
 *
 */
@Slf4j
@Component
public class AntDBClusterInstanceConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final InstanceResourceService instanceResourceService;

    private final AntDBInstanceService antDBInstanceService;

    private final ResourceService resourceService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param protectedEnvironmentService 环境服务
     * @param instanceResourceService 实例资源服务
     * @param antDBInstanceService antdb实例服务
     * @param resourceService 资源服务
     */
    public AntDBClusterInstanceConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, ProtectedEnvironmentService protectedEnvironmentService,
        InstanceResourceService instanceResourceService, AntDBInstanceService antDBInstanceService,
        ResourceService resourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.instanceResourceService = instanceResourceService;
        this.antDBInstanceService = antDBInstanceService;
        this.resourceService = resourceService;
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        log.info("Start collecting connectable resources.");
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(resource.getParentUuid());
        environment.setDependencies(resource.getDependencies()
            .get(DatabaseConstants.CHILDREN).get(0).getDependencies());
        resource.setEnvironment(environment);
        resource.getExtendInfo().put(DatabaseConstants.VIRTUAL_IP, resource.getEnvironment().getEndpoint());
        try {
            instanceResourceService.setClusterInstanceNodeRole(resource);
        } catch (LegoCheckedException | NullPointerException e) {
            log.error("Collecting antdb cluster instance check error", ExceptionUtil.getErrorMessage(e));
            // 当无法收集到集群实例中子实例的信息时，认为集群实例状态异常，需要更新数据库中集群实例在线信息
            ProtectedResource protectedResource = resourceService.getResourceById(resource.getUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not exist"));
            String status = LinkStatusEnum.OFFLINE.getStatus().toString();
            ProtectedResource updateResource = new ProtectedResource();
            updateResource.setUuid(protectedResource.getUuid());
            updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
            resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
        List<ProtectedResource> children = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        // 如果没有主节点直接报错
        Optional<ProtectedResource> childOptional = children.stream()
            .filter(
                item -> StringUtils.equals(AntDBConstants.PRIMARY, item.getExtendInfo().get(DatabaseConstants.ROLE)))
            .findFirst();
        if (!childOptional.isPresent()) {
            // 集群实例无主节点时认为集群实例状态异常，更新数据库中资源信息
            ProtectedResource protectedResource = resourceService.getResourceById(resource.getUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not exist"));
            String status = LinkStatusEnum.OFFLINE.getStatus().toString();
            ProtectedResource updateResource = new ProtectedResource();
            updateResource.setUuid(protectedResource.getUuid());
            updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
            resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
        return super.collectConnectableResources(resource);
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        try {
            return super.generateCheckResult(protectedResource);
        } catch (LegoCheckedException | NullPointerException e) {
            // 当检测集群实例下的子实例发生异常时，认为集群实例状态异常
            log.error("Generate Check Result antdb cluster instance error", ExceptionUtil.getErrorMessage(e));
            ProtectedResource instanceResource = resourceService.getResourceById(protectedResource.getParentUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not exist"));
            String status = LinkStatusEnum.OFFLINE.getStatus().toString();
            ProtectedResource updateResource = new ProtectedResource();
            updateResource.setUuid(instanceResource.getUuid());
            updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
            resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        log.info("To deal with antdb cluster instance collectActionResults");
        return super.collectActionResults(updateLinkStatus(checkReport), context);
    }

    private List<CheckReport<Object>> updateLinkStatus(List<CheckReport<Object>> checkReportList) {
        String instanceUuid = Optional.ofNullable(checkReportList.get(0))
            .map(CheckReport::getResource)
            .map(ProtectedResource::getParentUuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "not find antdb cluster instance resource"));
        log.info("start update antdb cluster instance {} linkStatus, checkReport length is {}", instanceUuid,
            checkReportList.size());

        try {
            ProtectedResource resource = antDBInstanceService.getResourceById(instanceUuid);
        } catch (LegoCheckedException | NullPointerException e) {
            log.info("No need to update the resource status when registering,", ExceptionUtil.getErrorMessage(e));
            return checkReportList;
        }

        ProtectedResource resource = antDBInstanceService.getResourceById(instanceUuid);
        AtomicBoolean isInstanceOnline = new AtomicBoolean(true);
        resource.getDependencies().get(DatabaseConstants.CHILDREN).forEach(childNode -> {
            String uuid = childNode.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
            if (getCheckResult(uuid, checkReportList)) {
                log.info("the subinstacne {}, of antdb cluster instance {} ONLINE", childNode.getUuid(), instanceUuid);
            } else {
                log.warn("the subinstacne {}, of antdb cluster instance {} OFFLINE", childNode.getUuid(), instanceUuid);
                isInstanceOnline.set(false);
            }
        });

        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(instanceUuid);
        updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, isInstanceOnline.get()
            ? LinkStatusEnum.ONLINE.getStatus().toString()
            : LinkStatusEnum.OFFLINE.getStatus().toString());

        resourceService.updateSourceDirectly(Lists.newArrayList(updateResource));
        log.info("end update antdb cluster instance {} linkStatus", instanceUuid);

        return checkReportList;
    }

    private boolean getCheckResult(String uuid, List<CheckReport<Object>> checkReportList) {
        List<ActionResult> actionResultList = checkReportList.stream()
            .filter(item -> Objects.equals(uuid, item.getResource().getEnvironment().getUuid()))
            .flatMap(item -> item.getResults().stream())
            .map(CheckResult::getResults)
            .collect(Collectors.toList());
        return ResourceCheckContextUtil.isSuccess(actionResultList);
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.ANT_DB_CLUSTER_INSTANCE.getType()
            .equals(object.getSubType());
    }
}