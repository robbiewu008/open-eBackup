/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.cnware.protection.access.service.impl;

import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.cnware.protection.access.service.CnwareCommonService;
import openbackup.cnware.protection.access.util.CnwareUtil;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.util.List;

/**
 * CNware类型服务
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-12-09
 */
@Service
@Slf4j
public class CnwareCommonServiceImpl implements CnwareCommonService {
    private final AgentUnifiedService agentUnifiedService;
    private final ResourceService resourceService;

    /**
     * CnwareCommonServiceImpl
     *
     * @param agentUnifiedService agentUnifiedService
     * @param resourceService resourceService
     */
    public CnwareCommonServiceImpl(AgentUnifiedService agentUnifiedService, ResourceService resourceService) {
        this.agentUnifiedService = agentUnifiedService;
        this.resourceService = resourceService;
    }

    @Override
    public void checkEnvName(String name) {
        CnwareUtil.verifyEnvName(name);
    }

    @Override
    public AppEnvResponse queryClusterInfo(ProtectedEnvironment environment, ProtectedEnvironment agentEnv) {
        return agentUnifiedService.getClusterInfo(environment, agentEnv);
    }

    /**
     * 获取Agent环境信息
     *
     * @param envId 环境uuid
     * @return Agent环境信息
     */
    @Override
    public ProtectedEnvironment getEnvironmentById(String envId) {
        return resourceService.getBasicResourceById(envId)
            .filter(env -> env instanceof ProtectedEnvironment)
            .map(env -> (ProtectedEnvironment) env)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST,
                "Protected environment is not exists!"));
    }

    /**
     * 校验Agent连通性
     *
     * @param environment 环境信息
     * @param agentEnvList agent环境信息列表
     */
    @Override
    public void checkConnectivity(ProtectedEnvironment environment, List<ProtectedEnvironment> agentEnvList) {
        log.info("Start to check connectivity of environment: {}", environment.getUuid());
        AgentBaseDto response = null;
        for (ProtectedEnvironment agentEnv : agentEnvList) {
            response = agentUnifiedService.checkApplication(environment, agentEnv);
            if (!VerifyUtil.isEmpty(response) && CnwareConstant.SUCCESS.equals(response.getErrorCode())) {
                log.info("CNware check success, uuid: {}, name: {}.", environment.getUuid(), environment.getName());
                return;
            }
        }
        log.error("CNware check failed, uuid: {}, name: {}.", environment.getUuid(), environment.getName());
        if (VerifyUtil.isEmpty(response) || VerifyUtil.isEmpty(response.getErrorMessage())) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent is empty");
        }
        ActionResult result = JsonUtil.read(response.getErrorMessage(), ActionResult.class);
        throw new LegoCheckedException(Long.parseLong(result.getBodyErr()),
            new String[] {result.getDetailParams().get(0)}, result.getMessage());
    }
}
