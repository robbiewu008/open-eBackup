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
package openbackup.db2.protection.access.provider.resource;

import openbackup.access.framework.resource.service.provider.AbstractResourceScanProvider;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.DatabaseScannerUtils;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;

/**
 * db2数据库扫描
 *
 */
@Component
@Slf4j
public class Db2DatabaseScanProvider extends AbstractResourceScanProvider {
    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * 构造器注入
     *
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     */
    public Db2DatabaseScanProvider(ResourceService resourceService, AgentUnifiedService agentUnifiedService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
    }

    /**
     * 扫描db2环境下数据库信息
     *
     * @param environment environment
     * @return 数据库列表
     */
    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        log.info("Start scan db2 instance. env id: {}", environment.getUuid());
        List<ProtectedResource> instances = DatabaseScannerUtils.getInstancesByEnvironment(environment.getUuid(),
            ResourceSubTypeEnum.DB2_INSTANCE.getType(), resourceService);
        List<ProtectedResource> resources = new ArrayList<>();
        instances.stream()
            .filter(instance -> DatabaseConstants.TOP_INSTANCE.equals(
                instance.getExtendInfoByKey(DatabaseConstants.IS_TOP_INSTANCE)))
            .forEach(instance -> {
                checkInstance(environment, instance);
                AgentDetailDto database = queryDatabaseByAgent(environment, instance);
                resources.addAll(convertDatabaseResource(database, instance, environment));
            });
        return resources;
    }

    private void checkInstance(ProtectedEnvironment environment, ProtectedResource instance) {
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        checkAppReq.setApplication(BeanTools.copy(instance, Application::new));
        AgentBaseDto checkResult = agentUnifiedService.check(ResourceSubTypeEnum.DB2_INSTANCE.getType(), environment,
            checkAppReq);
        if (!VerifyUtil.isEmpty(instance.getAuth())) {
            StringUtil.clean(instance.getAuth().getAuthPwd());
        }
        if (Long.parseLong(checkResult.getErrorCode()) == DatabaseConstants.SUCCESS_CODE) {
            return;
        }
        throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "check failed.");
    }

    private AgentDetailDto queryDatabaseByAgent(ProtectedEnvironment environment, ProtectedResource instance) {
        ListResourceReq db2DbsRequest = new ListResourceReq();
        db2DbsRequest.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        db2DbsRequest.setApplication(BeanTools.copy(instance, Application::new));
        return agentUnifiedService.getDetail(ResourceSubTypeEnum.DB2_INSTANCE.getType(), environment.getEndpoint(),
            environment.getPort(), db2DbsRequest);
    }

    private List<ProtectedResource> convertDatabaseResource(AgentDetailDto database, ProtectedResource instance,
        ProtectedEnvironment environment) {
        List<ProtectedResource> resources = new ArrayList<>();
        database.getResourceList().forEach(resource -> {
            ProtectedResource databaseResource = BeanTools.copy(resource, ProtectedResource::new);
            databaseResource.setParentName(instance.getName());
            databaseResource.setPath(environment.getEndpoint());
            databaseResource.setVersion(instance.getVersion());
            databaseResource.setExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY,
                instance.getExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY));
            databaseResource.setExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.SINGLE.getType());
            resources.add(databaseResource);
        });
        return resources;
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(ProtectedEnvironment object) {
        return ResourceSubTypeEnum.U_BACKUP_AGENT.equalsSubType(object.getSubType());
    }
}