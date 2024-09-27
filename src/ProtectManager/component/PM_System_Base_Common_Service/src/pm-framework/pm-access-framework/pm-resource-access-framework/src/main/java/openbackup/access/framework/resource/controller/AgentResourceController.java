/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.controller;

import openbackup.access.framework.resource.dto.AgentResourceQueryParam;
import openbackup.access.framework.resource.dto.DeliverTaskReq;
import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.log.constants.EventTarget;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.hibernate.validator.constraints.Length;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

import javax.validation.Valid;

/**
 * 功能描述: AgentResourceController
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-26
 */
@Slf4j
@RestController
public class AgentResourceController {
    private final AgentUnifiedService agentService;
    private final ResourceService resourceService;
    private final ProviderManager providerManager;
    private final AgentBusinessService agentBusinessService;

    /**
     * 构造器注入
     *
     * @param agentService agentService
     * @param resourceService resourceService
     * @param providerManager providerManager
     * @param agentBusinessService agent业务处理
     */
    public AgentResourceController(AgentUnifiedService agentService, ResourceService resourceService,
        ProviderManager providerManager, AgentBusinessService agentBusinessService) {
        this.agentService = agentService;
        this.resourceService = resourceService;
        this.providerManager = providerManager;
        this.agentBusinessService = agentBusinessService;
    }

    /**
     * 通过指定的代理主机实时浏览受保护环境的资源
     *
     * @param agentId 代理主机uuid
     * @param queryParam 查询参数
     * @return 受保护环境的资源
     */
    @ExterAttack
    @GetMapping("/v2/agents/{agentId}/resources")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resources = "resource:$1;$2.envId",
        resourceSetType = ResourceSetTypeEnum.AGENT, operation = OperationTypeEnum.QUERY,
        target = "#agentId")
    public PageListResponse<ProtectedResource> queryResources(
        @PathVariable("agentId") @Length(min = 1, max = 256) String agentId,
        @Valid AgentResourceQueryParam queryParam) {
        ListResourceV2Req agentRequest = new ListResourceV2Req();
        agentRequest.setPageNo(queryParam.getPageNo());
        agentRequest.setPageSize(queryParam.getPageSize());
        agentRequest.setOrders(Collections.singletonList(queryParam.getOrders()));
        agentRequest.setConditions(queryParam.getConditions());
        ProtectedEnvironment protectedEnvironment = queryEnvironmentById(queryParam.getEnvId());
        agentRequest.setAppEnv(BeanTools.copy(protectedEnvironment, AppEnv::new));
        List<Application> applications = queryParam.getResourceIds().stream()
            .map(resourceService::getBasicResourceById)
            .filter(Optional::isPresent)
            .map(Optional::get)
            .map(resource -> BeanTools.copy(resource, Application::new))
            .collect(Collectors.toList());
        agentRequest.setApplications(applications);
        String appType =
            VerifyUtil.isEmpty(queryParam.getAppType()) ? protectedEnvironment.getSubType() : queryParam.getAppType();
        ProtectedEnvironment agentEnv = queryEnvironmentById(agentId);
        return agentService.getDetailPageList(appType, agentEnv.getEndpoint(), agentEnv.getPort(), agentRequest);
    }

    /**
     * 查询配置信息
     *
     * @param subType 应用类型
     * @param script 脚本
     * @param hostUuids 主机ID
     * @return 配置信息
     */
    @ExterAttack
    @GetMapping("/v1/agents/config")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
        resourceSetType = ResourceSetTypeEnum.AGENT, operation = OperationTypeEnum.QUERY,
        target = "#hostUuids")
    public Map<String, Object> queryAppConf(@RequestParam("subType") String subType,
        @RequestParam(value = "script", required = false) String script,
        @RequestParam("hostUuids") String[] hostUuids) {
        if (VerifyUtil.isEmpty(hostUuids)) {
            return new HashMap<>();
        }
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(subType);
        ResourceProvider provider = providerManager.findProvider(ResourceProvider.class, protectedResource, null);
        if (provider != null) {
            return provider.queryAppConf(script, hostUuids);
        }
        return new HashMap<>();
    }

    /**
     * 传递任务信息
     *
     * @param deliverTaskReq 传递任务信息请求体
     */
    @ExterAttack
    @PutMapping("/v1/agents/task/status")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN})
    @Logging(
        name = "0x206400700001",
        target = EventTarget.JOB_NEW,
        details = {"$1.taskId", "$1.status"})
    public void deliverTaskStatus(@RequestBody DeliverTaskReq deliverTaskReq) {
        agentBusinessService.deliverTaskStatus(deliverTaskReq);
    }

    private ProtectedEnvironment queryEnvironmentById(String uuid) {
        Optional<ProtectedResource> optResource = resourceService.getBasicResourceById(uuid);
        if (!optResource.isPresent() || !(optResource.get() instanceof ProtectedEnvironment)) {
            log.error("Cannot find environment by uuid: {}", uuid);
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Environment does not exist");
        }
        return (ProtectedEnvironment) optResource.get();
    }
}