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

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * The UnifiedResourceConnectionChecker
 *
 * @author g30003063
 * @since 2022-05-20
 */
@Slf4j
@Component("unifiedResourceConnectionChecker")
public class UnifiedResourceConnectionChecker extends AbstractResourceChecker<Object> {
    // agent app check任务成功状态码
    private static final long SUCCESS_CODE = 0L;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     */
    public UnifiedResourceConnectionChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final AgentUnifiedService agentUnifiedService) {
        super(environmentRetrievalsService);
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        ProtectedEnvironment environment = protectedResource.getEnvironment();
        AgentBaseDto agentBaseDto = agentUnifiedService.checkApplicationNoRetry(protectedResource, environment);

        CheckResult<Object> checkResult = new CheckResult<>();
        checkResult.setEnvironment(environment);

        if (Long.parseLong(agentBaseDto.getErrorCode()) == SUCCESS_CODE) {
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(SUCCESS_CODE);
            actionResult.setBodyErr(String.valueOf(SUCCESS_CODE));
            actionResult.setMessage(agentBaseDto.getErrorMessage());
            checkResult.setResults(actionResult);
            return checkResult;
        }

        ActionResult actionResult = getActionResultFromErrorMessage(agentBaseDto.getErrorMessage());
        // 转化agent的默认失败操作
        if (Objects.equals(actionResult.getCode(), CommonErrorCode.OPERATION_FAILED)) {
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        }
        checkResult.setResults(actionResult);
        return checkResult;
    }

    private ActionResult getActionResultFromErrorMessage(String errorMessage) {
        JSONObject jsonObject = JSONObject.fromObject(errorMessage);
        String bodyErr = jsonObject.getString("bodyErr");
        ActionResult actionResult;
        if (VerifyUtil.isEmpty(bodyErr)) {
            actionResult = new ActionResult(CommonErrorCode.AGENT_NETWORK_ERROR, "check application failed.");
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        } else {
            actionResult = JSONObject.toBean(errorMessage, ActionResult.class);
        }
        actionResult.setCode(Long.parseLong(actionResult.getBodyErr()));
        return actionResult;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        return checkReport.stream()
            .map(CheckReport::getResults)
            .flatMap(List::stream)
            .map(CheckResult::getResults)
            .collect(Collectors.toList());
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return false;
    }
}