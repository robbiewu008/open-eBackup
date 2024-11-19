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
package openbackup.eccoracle.resource;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 连通性校验
 *
 */
@Component
@Slf4j
public class EccOracleConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param resourceService resourceService
     */
    public EccOracleConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, ResourceService resourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.equalsSubType(object.getSubType());
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        log.info("EccOracleConnectionChecker generateCheckResult start.");
        ProtectedEnvironment environment = protectedResource.getEnvironment();
        CheckResult<Object> checkResult = new CheckResult<>();
        ActionResult actionResult = new ActionResult();
        try {
            AgentBaseDto agentBaseDto = agentUnifiedService.checkApplication(protectedResource, environment);
            checkResult.setEnvironment(environment);

            if (Long.parseLong(agentBaseDto.getErrorCode()) == DatabaseConstants.SUCCESS_CODE) {
                actionResult.setCode(DatabaseConstants.SUCCESS_CODE);
                actionResult.setMessage(DatabaseConstants.SUCCESS);
                checkResult.setResults(actionResult);
                return checkResult;
            }

            actionResult = JSONObject.toBean(agentBaseDto.getErrorMessage(), ActionResult.class);
        } catch (LegoCheckedException e) {
            log.error("EccOracleConnectionChecker Fail to generateCheckResult checkApplication.", e);
            actionResult.setCode(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT);
            actionResult.setMessage(e.getMessage());
        }
        checkResult.setResults(actionResult);
        log.info("EccOracleConnectionChecker generateCheckResult end.");
        return checkResult;
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        Map<ProtectedResource, List<ProtectedEnvironment>> result = new HashMap<>();
        List<ProtectedEnvironment> agents = Collections.singletonList(resource.getEnvironment());
        result.put(resource, agents);
        return result;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        return super.collectActionResults(updateResourceLinkStatus(checkReport), context);
    }

    private List<CheckReport<Object>> updateResourceLinkStatus(List<CheckReport<Object>> checkReport) {
        String resourceId = checkReport.get(0).getResource().getUuid();
        // 注册时，检查连通性之后不做任何操作，直接返回
        if (!resourceService.getResourceById(resourceId).isPresent()) {
            return checkReport;
        }
        log.info("Start update SAP_ON_ORACLE_SINGLE link status after check connection, resource id: {}", resourceId);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(resourceId);
        // 对于集群数据库来说，只要有一个实例在线，集群数据库就在线
        List<CheckReport<Object>> online = checkReport.stream()
            .filter(report -> isOnline(report.getResults()))
            .collect(Collectors.toList());
        if (online.size() > 0) {
            protectedResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                LinkStatusEnum.ONLINE.getStatus().toString());
            checkReport.get(0)
                .getResults()
                .forEach(checkResult -> checkResult.getResults().setCode(DatabaseConstants.SUCCESS_CODE));
            resourceService.updateSourceDirectly(Collections.singletonList(protectedResource));
            log.info("Finished update SAP_ON_ORACLE_SINGLE Online after check connection, resource id: {}", resourceId);
            return Collections.singletonList(checkReport.get(0));
        } else {
            protectedResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                LinkStatusEnum.OFFLINE.getStatus().toString());
            resourceService.updateSourceDirectly(Collections.singletonList(protectedResource));
            log.info("Finished update SAP_ON_ORACLE_SINGLE Offline after check connection, resource id: {}",
                resourceId);
            return checkReport;
        }
    }

    private boolean isOnline(List<CheckResult<Object>> results) {
        return results.stream()
            .anyMatch(checkResult -> checkResult.getResults().getCode() == DatabaseConstants.SUCCESS_CODE);
    }
}
