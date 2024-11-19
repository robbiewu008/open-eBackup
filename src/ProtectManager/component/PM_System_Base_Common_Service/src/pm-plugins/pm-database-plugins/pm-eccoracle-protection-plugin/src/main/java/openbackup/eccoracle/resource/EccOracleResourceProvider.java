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
package openbackup.eccoracle.resource;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.eccoracle.service.EccOracleBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * ecc oracle单实例资源注册provider
 *
 */
@Component
@Slf4j
public class EccOracleResourceProvider implements ResourceProvider {
    private final ResourceService resourceService;

    private final ProviderManager providerManager;

    private final InstanceResourceService instanceResourceService;

    private final EccOracleBaseService oracleBaseService;

    public EccOracleResourceProvider(ResourceService resourceService, ProviderManager providerManager,
        InstanceResourceService instanceResourceService, EccOracleBaseService oracleBaseService) {
        this.resourceService = resourceService;
        this.providerManager = providerManager;
        this.instanceResourceService = instanceResourceService;
        this.oracleBaseService = oracleBaseService;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.getType().equals(object.getSubType());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("EccOracleResourceProvider start beforeCreate.");
        checkConnect(resource);
        String envId = resource.getExtendInfo().get(DatabaseConstants.HOST_ID);
        ProtectedEnvironment environment = oracleBaseService.getEnvironmentById(envId);
        // 校验是否已经存在实例 并 刷新子实例的集群状态
        oracleBaseService.refreshClusterInstanceActiveStandby(resource, environment, true);
        // 设置path信息，否则复制的时候会报错
        resource.setPath(environment.getEndpoint());
        // 设置实例认证状态
        resource.getExtendInfo().put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("EccOracleResourceProvider end beforeCreate.");
    }

    private void checkConnect(ProtectedResource resource) {
        ResourceConnectionCheckProvider provider = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            resource);
        ResourceCheckContext context = provider.tryCheckConnection(resource);
        checkConnectResult(resource, context);
    }

    private static void checkConnectResult(ProtectedResource resource, ResourceCheckContext context) {
        if (VerifyUtil.isEmpty(context.getActionResults())) {
            log.error("oracle instance check connection result is empty. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check connection result is empty.");
        }
        ActionResult actionResult = context.getActionResults().get(IsmNumberConstant.ZERO);
        if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
            log.error("oracle instance check connection failed. name: {}", resource.getName());
            if (VerifyUtil.isEmpty(actionResult.getDetailParams())) {
                throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), "check connection failed.");
            }
            String[] params = Optional.ofNullable(actionResult.getDetailParams())
                .map(e -> e.toArray(new String[0]))
                .orElse(new String[0]);
            throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), params,
                "check connection failed.");
        }
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> agents = resourceService.queryDependencyResources(true, DatabaseConstants.AGENTS,
            Collections.singletonList(resource.getUuid()));
        dependencies.put(DatabaseConstants.AGENTS, agents);
        resource.setDependencies(dependencies);
        return true;
    }

    @Override
    public ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        log.info("start SAP_ON_ORACLE_SINGLE single node database delete pre check, resourceId:{}", resource.getUuid());
        if (!oracleBaseService.isAnonymizationDeletable(resource.getUuid())) {
            throw new LegoCheckedException(CommonErrorCode.ANONYMIZATION_JOB_IS_RUNNING, new String[] {},
                "resource has running anonymization job");
        }
        return ResourceDeleteContext.defaultValue();
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        checkConnect(resource);
        // 刷新子实例的集群状态
        oracleBaseService.refreshClusterInstanceActiveStandby(resource,
            getAgentBySingleInstanceUuid(resource.getUuid()), false);
        // 设置实例认证状态
        resource.getExtendInfo().put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Override
    public List<ProtectedResource> scan(ProtectedResource resource) {
        log.info("Oracle single scan started, resource id :{}", resource.getUuid());
        oracleBaseService.refreshClusterInstanceActiveStandby(resource, resource.getEnvironment(), false);
        log.info("Oracle single scan finished, resource id :{}", resource.getUuid());
        return Collections.singletonList(resource);
    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        instanceResourceService.healthCheckSingleInstance(resource);
    }

    private ProtectedEnvironment getAgentBySingleInstanceUuid(String singleInstanceUuid) {
        ProtectedResource singleInstanceResources = resourceService.getResourceById(singleInstanceUuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "object not exists"));
        return singleInstanceResources.getEnvironment();
    }

    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = new ResourceFeature();
        // oracle实例名称可以重复，不检查实例重名
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        return resourceFeature;
    }
}
