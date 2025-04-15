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
package openbackup.db2.protection.access.provider.check;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2InstanceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * DB2连通性检查
 *
 */
@Component
@Slf4j
public class Db2ConnectionChecker extends UnifiedResourceConnectionChecker {
    @Autowired
    private ResourceService resourceService;

    @Autowired
    private Db2InstanceService db2InstanceService;

    @Autowired
    private ProtectedEnvironmentService environmentService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     */
    public Db2ConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService) {
        super(environmentRetrievalsService, agentUnifiedService);
    }

    /**
     * 监测对象适用，该provider适用于对象存储
     *
     * @param resource 资源
     * @return 检查结果
     */
    @Override
    public boolean applicable(ProtectedResource resource) {
        return Objects.equals(ResourceSubTypeEnum.DB2_CLUSTER_INSTANCE.getType(), resource.getSubType()) && (
            Db2ClusterTypeEnum.POWER_HA.getType().equals(resource.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE))
                || Db2ClusterTypeEnum.RHEL_HA.getType()
                .equals(resource.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE)));
    }

    /**
     * 获取检查结果
     *
     * @param checkReports 检查报告
     * @param context 上下文
     * @return 检查结果
     */
    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReports,
        Map<String, Object> context) {
        // 检查结果，只要有一个通过，即为ok，剔除异常结果
        List<ActionResult> actionResults = super.collectActionResults(checkReports, context);
        log.info("[db2] collect result, size is {}", actionResults.size());
        // 主节点故障，结果为空
        if (actionResults.isEmpty()) {
            actionResults.add(new ActionResult(CommonErrorCode.AGENT_NETWORK_ERROR, "query master node fail"));
        }
        List<ActionResult> filterResults = actionResults.stream()
            .filter(result -> result.getCode() == ActionResult.SUCCESS_CODE)
            .collect(Collectors.toList());

        // 连通性检查成功，修改集群实例在线
        updateInstanceStatus(checkReports, filterResults);

        // 如果过滤为空，则返回原有结果
        if (CollectionUtils.isEmpty(filterResults)) {
            return actionResults;
        }
        log.info("[db2] filter connet node, size is {}", actionResults.size());
        return filterResults;
    }

    private void updateInstanceStatus(List<CheckReport<Object>> checkReports, List<ActionResult> filterResults) {
        if (!CollectionUtils.isEmpty(filterResults)) {
            String instanceUuid = Optional.ofNullable(checkReports.get(0))
                .map(CheckReport::getResource)
                .map(ProtectedResource::getParentUuid)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    "not find cb2 cluster instance resource"));

            log.info("[db2] Change the status of the instance to Online, instance id is {}", instanceUuid);
            ProtectedResource updateResource = new ProtectedResource();
            updateResource.setUuid(instanceUuid);
            updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                String.valueOf(LinkStatusEnum.ONLINE.getStatus()));
            resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
        }
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        try {
            return super.generateCheckResult(protectedResource);
        } catch (LegoCheckedException | LegoUncheckedException e) {
            log.error("[db2] generate check result fail");
            CheckResult<Object> checkResult = new CheckResult<>();
            checkResult.setEnvironment(protectedResource.getEnvironment());
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            actionResult.setMessage(e.getMessage());
            checkResult.setResults(actionResult);
            return checkResult;
        }
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        // HA集群只检查主节点是否在线
        Map<ProtectedResource, List<ProtectedEnvironment>> resources = super.collectConnectableResources(
            resource);
        Map<ProtectedResource, List<ProtectedEnvironment>> result = new HashMap<>(1);
        for (Map.Entry<ProtectedResource, List<ProtectedEnvironment>> entry : resources.entrySet()) {
            ProtectedResource subInstance = entry.getKey();
            ProtectedEnvironment subEnv = environmentService.getEnvironmentById(
                subInstance.getExtendInfoByKey(DatabaseConstants.HOST_ID));
            try {
                Optional<NodeInfo> masterNode = db2InstanceService.queryMasterNode(subInstance, subEnv);
                if (masterNode.isPresent()) {
                    result.put(entry.getKey(), entry.getValue());
                }
            } catch (LegoCheckedException e) {
                log.error("query db2 master node fail", ExceptionUtil.getErrorMessage(e));
            }
        }
        return result;
    }
}
