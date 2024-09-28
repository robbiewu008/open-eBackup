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
package openbackup.tpops.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * GaussDb 集群环境联通性校验覆写collectActionResults，异常抛出
 *
 */
@Component
@Slf4j
public class TpopsGaussDBConnectionChecker extends UnifiedResourceConnectionChecker {
    @Autowired
    private ProtectedEnvironmentService protectedEnvironmentService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     */
    public TpopsGaussDBConnectionChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final AgentUnifiedService agentUnifiedService) {
        super(environmentRetrievalsService, agentUnifiedService);
    }

    /**
     * 联通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.getType()
            .equals(object.getSubType());
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        log.info("start to generate checked result");
        CheckResult<Object> checkResult = new CheckResult<>();
        ProtectedEnvironment environment = protectedResource.getEnvironment();
        checkResult.setEnvironment(environment);
        addAgentIp(protectedResource);
        return super.generateCheckResult(protectedResource);
    }

    private void addAgentIp(ProtectedResource protectedResource) {
        // cluster_ip
        String clusterIp = Optional.ofNullable(
                protectedResource.getDependencies().get(TpopsGaussDBConstant.GAUSSDB_AGENTS))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .map(protectedEnvironmentService::getEnvironmentById)
            .map(this::getIps)
            .collect(Collectors.joining(";"));
        Map<String, String> extendInfo = Optional.ofNullable(protectedResource.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, clusterIp);
        protectedResource.setExtendInfo(extendInfo);
    }

    private String getIps(ProtectedEnvironment protectedEnvironment) {
        Map<String, String> extendInfo = Optional.ofNullable(protectedEnvironment.getExtendInfo())
            .orElse(new HashMap<>());
        String ips = extendInfo.get(ResourceConstants.AGENT_IP_LIST);
        if (StringUtils.isEmpty(ips)) {
            return protectedEnvironment.getEndpoint();
        }
        return ips;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        log.info("start to collect action result");
        List<ActionResult> checkResultList = checkReport.stream()
            .map(CheckReport::getResults)
            .flatMap(List::stream)
            .map(CheckResult::getResults)
            .map(this::modifyCode)
            .collect(Collectors.toList());

        // 轻量化gaussdb检查资源联通性, 做代理冗余, 如果有一个返回结果是成功, 则检查联通性成功
        boolean isCodeZero = checkResultList.stream()
            .anyMatch(actionResult -> actionResult.getCode() == ActionResult.SUCCESS_CODE);
        if (isCodeZero) {
            log.info("check connect action success");
            checkResultList.forEach(actionResult -> actionResult.setCode(ActionResult.SUCCESS_CODE));
        }
        return checkResultList;
    }

    private ActionResult modifyCode(ActionResult actionResult) {
        if (StringUtils.isEmpty(actionResult.getBodyErr())) {
            return actionResult;
        }
        actionResult.setCode(Integer.parseInt(actionResult.getBodyErr()));
        return actionResult;
    }
}
