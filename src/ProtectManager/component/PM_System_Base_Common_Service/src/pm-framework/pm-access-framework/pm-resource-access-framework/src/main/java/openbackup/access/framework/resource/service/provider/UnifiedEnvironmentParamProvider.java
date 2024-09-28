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
