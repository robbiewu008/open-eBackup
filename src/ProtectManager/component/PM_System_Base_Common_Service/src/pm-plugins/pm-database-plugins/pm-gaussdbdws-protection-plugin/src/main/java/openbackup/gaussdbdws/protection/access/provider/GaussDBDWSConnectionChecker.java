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
package openbackup.gaussdbdws.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.gaussdbdws.protection.access.util.DwsTaskEnvironmentUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * dws 集群环境联通性校验覆写collectActionResults，异常抛出
 *
 */
@Component
@Slf4j
public class GaussDBDWSConnectionChecker extends UnifiedResourceConnectionChecker {
    // agent app check任务成功状态码
    private static final int SUCCESS_CODE = 0;

    @Autowired
    private GaussDBBaseService gaussDBBaseService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     */
    public GaussDBDWSConnectionChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
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
        return Objects.nonNull(object) && ResourceSubTypeEnum.GAUSSDB_DWS.getType().equals(object.getSubType());
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        CheckResult<Object> checkResult = new CheckResult<>();
        ActionResult actionResult = new ActionResult();
        ProtectedEnvironment environment = protectedResource.getEnvironment();
        checkResult.setEnvironment(environment);
        addAgentIp(protectedResource);
        if (isHostAgent(protectedResource, environment) || selectOneClusterAgent(protectedResource,
            environment.getUuid())) {
            actionResult.setCode(SUCCESS_CODE);
            actionResult.setMessage("SUCCESS");
            checkResult.setResults(actionResult);
            return checkResult;
        }
        return super.generateCheckResult(protectedResource);
    }

    private void addAgentIp(ProtectedResource protectedResource) {
        // cluster_ip
        String clusterIp = Optional.ofNullable(protectedResource.getDependencies().get(DwsConstant.DWS_CLUSTER_AGENT))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .map(gaussDBBaseService::getEnvironmentById)
            .map(DwsTaskEnvironmentUtil::getIps)
            .collect(Collectors.joining(";"));
        String hostIp = Optional.ofNullable(protectedResource.getDependencies().get(DwsConstant.HOST_AGENT))
            .orElse(new ArrayList<>())
            .stream()
            .map(ProtectedResource::getUuid)
            .map(gaussDBBaseService::getEnvironmentById)
            .map(DwsTaskEnvironmentUtil::getIps)
            .collect(Collectors.joining(";"));
        Map<String, String> extendInfo = Optional.ofNullable(protectedResource.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(DwsConstant.DWS_CLUSTER_AGENT, clusterIp);
        extendInfo.put(DwsConstant.HOST_AGENT, hostIp);
        protectedResource.setExtendInfo(extendInfo);
    }

    private boolean isHostAgent(ProtectedResource protectedResource, ProtectedEnvironment environment) {
        List<String> hostUuids = Optional.ofNullable(protectedResource.getDependencies().get(DwsConstant.HOST_AGENT))
            .orElse(new ArrayList<>())
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        return hostUuids.contains(environment.getUuid());
    }

    private boolean selectOneClusterAgent(ProtectedResource protectedResource, String uuid) {
        List<String> clusterUuids = Optional.ofNullable(
            protectedResource.getDependencies().get(DwsConstant.DWS_CLUSTER_AGENT))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        for (String clusterUuid : clusterUuids) {
            ProtectedEnvironment environmentById = gaussDBBaseService.getEnvironmentById(clusterUuid);
            if (LinkStatusEnum.OFFLINE.getStatus().toString()
                    .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(environmentById))) {
                continue;
            }
            return !clusterUuid.equals(uuid);
        }
        throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "name is not exist");
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        return checkReport.stream()
            .map(CheckReport::getResults)
            .flatMap(List::stream)
            .map(CheckResult::getResults)
            .map(this::modifyCode)
            .collect(Collectors.toList());
    }

    private ActionResult modifyCode(ActionResult actionResult) {
        if (StringUtils.isEmpty(actionResult.getBodyErr())) {
            return actionResult;
        }
        actionResult.setCode(Integer.parseInt(actionResult.getBodyErr()));
        return actionResult;
    }
}
