/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.function.Consumer;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Openstack资源连通性检查器
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-1-10
 */
@Slf4j
@Component
public class OpenstackConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ResourceService resourceService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService          agent接口实现
     * @param resourceService              资源服务
     */
    public OpenstackConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, ResourceService resourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.resourceService = resourceService;
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.OPENSTACK_CONTAINER.equalsSubType(protectedResource.getSubType());
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        if (isDomain(protectedResource)) {
            resourceService.getResourceById(protectedResource.getUuid())
                .ifPresent(env -> protectedResource.getAuth().setExtendInfo(env.getAuth().getExtendInfo()));
        }
        ProtectedEnvironment agentEnv = protectedResource.getEnvironment();
        if (LinkStatusEnum.OFFLINE.getStatus().toString()
            .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(agentEnv))) {
            log.info("agent(id:{}) is offline, no need to check connection.", agentEnv.getUuid());
            return generateCheckResult();
        }
        CheckResult<Object> checkResult = super.generateCheckResult(protectedResource);
        updateLinkStatus(protectedResource, checkResult);
        log.info("Openstack generate connection check result success. envId:{}", protectedResource.getUuid());
        return checkResult;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        List<ActionResult> errorResults = Lists.newArrayList();
        List<ActionResult> successResults = checkReport.stream()
            .flatMap(item -> item.getResults().stream())
            .map(CheckResult::getResults)
            .peek(checkAnyErrorResult(errorResults))
            .filter(actionResult -> actionResult.getCode() == ActionResult.SUCCESS_CODE)
            .collect(Collectors.toList());
        if (successResults.isEmpty()) {
            log.error("OpenStack check connection failed.");
            return errorResults;
        }
        return successResults;
    }

    private boolean isDomain(ProtectedResource protectedResource) {
        String isDomain = Optional.of(protectedResource)
            .map(ProtectedResource::getExtendInfo)
            .map(extendInfo -> extendInfo.get(OpenstackConstant.IS_DOMAIN))
            .orElse(Boolean.FALSE.toString());
        return Boolean.parseBoolean(isDomain);
    }

    private CheckResult<Object> generateCheckResult() {
        CheckResult<Object> checkResult = new CheckResult<>();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
        actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        actionResult.setMessage("agent is offline, can not check connection.");
        checkResult.setResults(actionResult);
        return checkResult;
    }

    private Consumer<ActionResult> checkAnyErrorResult(List<ActionResult> errorActionResult) {
        return actionResult -> {
            if (actionResult.getCode() != ActionResult.SUCCESS_CODE) {
                errorActionResult.add(actionResult);
            }
        };
    }

    private void updateLinkStatus(ProtectedResource protectedResource, CheckResult<Object> checkResult) {
        if (isDomain(protectedResource)) {
            log.info("check domain user, no need to update link status.");
            return;
        }
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid(protectedResource.getUuid());
        if (checkResult.getResults().getCode() == ActionResult.SUCCESS_CODE) {
            resource.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        } else {
            resource.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        }
        resourceService.updateSourceDirectly(Stream.of(resource).collect(Collectors.toList()));
    }
}