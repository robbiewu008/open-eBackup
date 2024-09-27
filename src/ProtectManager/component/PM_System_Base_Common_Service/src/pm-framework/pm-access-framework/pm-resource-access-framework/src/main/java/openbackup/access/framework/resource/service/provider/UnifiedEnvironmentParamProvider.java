/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.access.framework.resource.service.provider;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentParamProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.Map;

/**
 * 功能描述: DefaultEnvironmentParamProvider
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-29
 */
@Slf4j
@Component("unifiedEnvironmentParamProvider")
public class UnifiedEnvironmentParamProvider implements EnvironmentParamProvider {
    /**
     * resourceService
     */
    protected final ResourceService resourceService;

    /**
     * 构造器注入
     *
     * @param resourceService resourceService
     */
    public UnifiedEnvironmentParamProvider(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    @Override
    public boolean applicable(ProtectedEnvironment environment) {
        return false;
    }

    @Override
    public void checkAndPrepareParam(ProtectedEnvironment environment) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(environment.getName());
    }

    @Override
    public void updateEnvironment(ProtectedEnvironment environment) {
        log.info("Default environment param provider does not need to update param.");
    }

    /**
     * 默认通过endpoint进行重复性校验
     *
     * @param env 受保护环境
     */
    @Override
    public void checkEnvironmentRepeat(ProtectedEnvironment env) {
        if (!VerifyUtil.isEmpty(env.getUuid())) {
            log.info("Update env(uuid: {}, name: {}), no need check repeat.", env.getUuid(), env.getName());
            return;
        }
        Map<String, Object> filter = new HashMap<>();
        filter.put("endpoint", env.getEndpoint());
        filter.put("type", env.getType());
        filter.put("subType", env.getSubType());
        PageListResponse<ProtectedResource> registeredEnv = resourceService.query(0, 1, filter);
        if (registeredEnv.getTotalCount() > 0) {
            log.error("The env with endpoint({}) has been registered.", env.getEndpoint());
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "Env has registered");
        }
    }
}