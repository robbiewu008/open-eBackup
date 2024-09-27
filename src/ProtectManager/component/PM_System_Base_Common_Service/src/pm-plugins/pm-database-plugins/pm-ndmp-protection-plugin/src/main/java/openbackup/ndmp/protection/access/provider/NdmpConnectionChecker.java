/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.ndmp.protection.access.provider;

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
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;
import org.springframework.util.StringUtils;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Ndmp 集群环境联通性校验覆写collectActionResults，异常抛出
 *
 * @author t30021437
 * @version [OceanProtect X8000 1.5.0]
 * @since 2023-05-09
 */
@Component
@Slf4j
public class NdmpConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param protectedEnvironmentService protectedEnvironmentService
     */
    public NdmpConnectionChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final AgentUnifiedService agentUnifiedService, ProtectedEnvironmentService protectedEnvironmentService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.protectedEnvironmentService = protectedEnvironmentService;
    }

    /**
     * 联通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.NDMP.getType().equals(object.getSubType());
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
        String clusterIp = Optional.ofNullable(protectedResource.getDependencies().get(NdmpConstant.AGENTS))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .map(protectedEnvironmentService::getEnvironmentById)
            .map(this::getIps)
            .collect(Collectors.joining(";"));
        Map<String, String> extendInfo = Optional.ofNullable(protectedResource.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(NdmpConstant.AGENTS, clusterIp);
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
