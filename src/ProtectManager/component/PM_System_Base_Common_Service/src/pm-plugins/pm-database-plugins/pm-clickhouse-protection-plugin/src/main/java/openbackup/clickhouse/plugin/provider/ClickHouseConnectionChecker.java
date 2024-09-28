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
package openbackup.clickhouse.plugin.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.clickhouse.plugin.service.ClickHouseService;
import openbackup.clickhouse.plugin.util.ClickHouseValidator;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.AuthParamUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * ClickHouse连接检查
 *
 */
@Slf4j
@Component
public class ClickHouseConnectionChecker extends UnifiedResourceConnectionChecker {
    @Autowired
    private ClickHouseService clickHouseService;

    @Autowired
    private KerberosService kerberosService;

    @Autowired
    private EncryptorService encryptorService;

    @Autowired
    private ProtectedEnvironmentService environmentService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     */
    public ClickHouseConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService) {
        super(environmentRetrievalsService, agentUnifiedService);
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        ClickHouseValidator.checkNode(protectedResource);
        clickHouseService.checkNodeExists(protectedResource);
        AuthParamUtil.convertKerberosAuth(protectedResource.getAuth(), kerberosService,
            protectedResource.getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID), encryptorService);
        CheckResult<Object> checkResult = super.generateCheckResult(protectedResource);
        // 敏感信息最后一次使用结束后未在内存中移除
        AuthParamUtil.removeSensitiveInfo(protectedResource);
        return checkResult;
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(
        ProtectedResource environment) {
        log.info("ClickHouse collect connectable resources, environment: {}", environment.getName());
        Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap = Maps.newLinkedHashMap();
        List<ProtectedResource> children = environment.getDependencies().get(ResourceConstants.CHILDREN);
        for (ProtectedResource child : children) {
            String uuid = child.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
            ProtectedEnvironment host = environmentService.getEnvironmentById(uuid);
            nodeHostMap.put(child, Lists.newArrayList(host));
        }
        return nodeHostMap;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.CLICK_HOUSE.getType().equals(object.getSubType());
    }
}
