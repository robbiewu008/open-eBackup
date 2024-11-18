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
package openbackup.redis.plugin.provider;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceScanProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.AuthParamUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.redis.plugin.service.RedisService;
import openbackup.redis.plugin.service.impl.RedisServiceImpl;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * 集群资源扫描
 *
 */
@Component
@Slf4j
public class RedisResourceScanProvider implements ResourceScanProvider {
    private final AgentUnifiedService agentUnifiedService;

    private final ProtectedEnvironmentService environmentService;

    private final KerberosService kerberosService;

    private final EncryptorService encryptorService;

    private final RedisService redisService;

    /**
     * 构造器注入
     *
     * @param agentUnifiedService agentUnifiedService
     * @param environmentService environmentService
     * @param kerberosService kerberosService
     * @param encryptorService encryptorService
     * @param redisService redisService
     */
    public RedisResourceScanProvider(AgentUnifiedService agentUnifiedService,
        ProtectedEnvironmentService environmentService, KerberosService kerberosService,
        EncryptorService encryptorService, RedisService redisService) {
        this.agentUnifiedService = agentUnifiedService;
        this.environmentService = environmentService;
        this.kerberosService = kerberosService;
        this.encryptorService = encryptorService;
        this.redisService = redisService;
    }

    @Override
    public boolean applicable(ProtectedEnvironment object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.REDIS.getType().equals(object.getSubType());
    }

    /**
     * 扫描集群的节点信息
     *
     * @param environment 集群
     * @return 更新扩展信息后的集群节点
     */
    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        if (!StringUtils.equals(environment.getSubType(), ResourceSubTypeEnum.REDIS.getType())) {
            return Collections.emptyList();
        }
        List<ProtectedResource> children = environment.getDependencies().get(ResourceConstants.CHILDREN);
        log.info("scan children.size:{}", children.size());
        List<ProtectedResource> updatedChildren = new ArrayList<>();
        for (ProtectedResource child : children) {
            Endpoint endpoint = redisService.selectAgent(child);
            ProtectedEnvironment host = environmentService.getEnvironmentById(endpoint.getId());
            AuthParamUtil.convertKerberosAuth(child.getAuth(), kerberosService,
                child.getAuth().getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID), encryptorService);
            PageListResponse<ProtectedResource> response = agentUnifiedService.getDetailPageListNoRetry(
                ResourceSubTypeEnum.REDIS.getType(), host.getEndpoint(), host.getPort(),
                RedisServiceImpl.generateListResourceV2Req(host, child), false);
            log.info("scan getDetailPageListNoRetry return info");
            if (Objects.isNull(response) || response.getTotalCount() < 1) {
                continue;
            }
            ProtectedResource updateChild = new ProtectedResource();
            updateChild.setUuid(child.getUuid());
            updateChild.setExtendInfo(response.getRecords().get(0).getExtendInfo());
            updatedChildren.add(updateChild);
        }
        return updatedChildren;
    }
}