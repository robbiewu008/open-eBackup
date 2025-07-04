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
package openbackup.access.framework.resource.service.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.utils.JSONObject;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * The UnifiedClusterResourceInterityChecker
 *
 */
@Slf4j
@Component("unifiedClusterResourceIntegrityChecker")
public class UnifiedClusterResourceIntegrityChecker extends AbstractResourceChecker<AppEnvResponse> {
    private static final int SUCCESS_CODE = 0;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     */
    public UnifiedClusterResourceIntegrityChecker(
        final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final AgentUnifiedService agentUnifiedService) {
        super(environmentRetrievalsService);
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public CheckResult<AppEnvResponse> generateCheckResult(ProtectedResource protectedResource) {
        ProtectedEnvironment environment = protectedResource.getEnvironment();
        AppEnvResponse clusterInfo = agentUnifiedService.getClusterInfo(protectedResource, environment);
        CheckResult<AppEnvResponse> checkResult = new CheckResult<>();
        checkResult.setEnvironment(environment);
        checkResult.setData(clusterInfo);
        return checkResult;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<AppEnvResponse>> checkReports,
        Map<String, Object> context) {
        return checkReports.stream().map(this::checkClusterIntegrity).collect(Collectors.toList());
    }

    private ActionResult checkClusterIntegrity(CheckReport<AppEnvResponse> checkReport) {
        Set<Set<String>> hostNameDuplicateSet = checkReport.getResults()
            .stream()
            .map(CheckResult::getData)
            .map(AppEnvResponse::getNodes)
            .filter(Objects::nonNull)
            .map(this::toHostNameList)
            .collect(Collectors.toSet());

        log.info("check cluster integrity, cluster host name set: {}.",
            JSONObject.writeValueAsString(hostNameDuplicateSet));

        if (hostNameDuplicateSet.size() > 1) {
            return new ActionResult(CommonErrorCode.OPERATION_FAILED,
                "The host is not the same cluster.");
        }

        Optional<Set<String>> hostNameSetOpt = hostNameDuplicateSet.stream().findFirst();
        if (!hostNameSetOpt.isPresent()) {
            return new ActionResult(CommonErrorCode.OPERATION_FAILED, "The cluster nodes info don't be find.");
        }
        Set<String> hostNameSet = hostNameSetOpt.get();

        if (hostNameSet.size() <= 0) {
            return new ActionResult(CommonErrorCode.OPERATION_FAILED, "The cluster nodes info don't be find.");
        }

        Set<String> endPointSet = checkReport.getResults()
            .stream()
            .map(CheckResult::getEnvironment)
            .map(ProtectedEnvironment::getName)
            .collect(Collectors.toSet());

        if (!Objects.equals(hostNameSet, endPointSet)) {
            return new ActionResult(CommonErrorCode.OPERATION_FAILED, "The cluster nodes info don't match.");
        }

        return new ActionResult(SUCCESS_CODE, "SUCCESS");
    }

    private Set<String> toHostNameList(List<NodeInfo> nodeInfoList) {
        return nodeInfoList.stream().map(NodeInfo::getName).filter(Objects::nonNull).collect(Collectors.toSet());
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return false;
    }
}
