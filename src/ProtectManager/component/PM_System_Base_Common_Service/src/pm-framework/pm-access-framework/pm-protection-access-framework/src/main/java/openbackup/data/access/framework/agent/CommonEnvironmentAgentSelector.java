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
package openbackup.data.access.framework.agent;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;

/**
 * 传统数据库资源的保护代理选择器，备份/恢复该类资源时保护代理和受保护环境在一起
 *
 */
@Component
@Slf4j
public class CommonEnvironmentAgentSelector implements ProtectAgentSelector {
    private final ProtectedEnvironmentService environmentService;

    public CommonEnvironmentAgentSelector(ProtectedEnvironmentService environmentService) {
        this.environmentService = environmentService;
    }

    @Override
    public List<Endpoint> select(ProtectedResource protectedResource, Map<String, String> parameters) {
        ProtectedEnvironment environment = protectedResource.getEnvironment();
        if (environment == null) {
            // agent资源手动触发资源扫描，走到此分支，返回空列表
            if (ResourceTypeEnum.HOST.getType().equals(protectedResource.getType())) {
                return Collections.singletonList(new Endpoint());
            }
            log.error("Protected env is null!");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
        // 从数据库里查询最新的环境信息，适配environment里面字段值不全的场景
        ProtectedEnvironment queryEnvironment = environmentService.getEnvironmentById(environment.getUuid());
        Endpoint endPoint = new Endpoint(queryEnvironment.getUuid(), queryEnvironment.getEndpoint(),
            queryEnvironment.getPort(), queryEnvironment.getOsType());

        return Collections.singletonList(endPoint);
    }

    @Override
    public boolean applicable(String object) {
        return ResourceTypeEnum.HOST.getType().equals(object);
    }
}
