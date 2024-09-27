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
package openbackup.mysql.resources.access.provider;

import static openbackup.database.base.plugin.common.DatabaseConstants.CLUSTER_TYPE;
import static openbackup.database.base.plugin.common.DatabaseConstants.LINK_STATUS_KEY;
import static openbackup.database.base.plugin.common.DatabaseConstants.VERSION;
import static java.util.Collections.singletonList;

import openbackup.access.framework.resource.service.provider.AbstractResourceScanProvider;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.utils.DatabaseScannerUtils;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ProtectionBatchOperationReq;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 数据库扫描通用
 *
 * @author xWX950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-25
 */
@Component
@Slf4j
public class MysqlDatabaseScanner extends AbstractResourceScanProvider {
    private final ResourceService resourceService;

    private final MysqlInstanceProvider mysqlInstanceProvider;

    @Autowired
    private ProtectObjectRestApi protectObjectRestApi;

    @Autowired
    private AgentUnifiedService agentUnifiedService;

    /**
     * 构造器注入
     *
     * @param resourceService 数据库资源服务
     * @param mysqlInstanceProvider 单实例provider
     */
    public MysqlDatabaseScanner(ResourceService resourceService, MysqlInstanceProvider mysqlInstanceProvider) {
        this.resourceService = resourceService;
        this.mysqlInstanceProvider = mysqlInstanceProvider;
    }

    /**
     * 扫描环境下数据库信息
     *
     * @param environment environment
     * @return 数据库列表
     */
    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        List<ProtectedResource> instances = DatabaseScannerUtils.getInstancesByEnvironment(environment.getUuid(),
            ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType(), resourceService);
        if (!needScan(instances)) {
            return instances;
        }
        List<ProtectedResource> resources = new ArrayList<>(instances);
        instances.forEach(instance -> {
            instance.setEnvironment(environment);
            // 扫描前先check，如果服务不可用，则报错框架则不会删除以前的资源
            CheckAppReq checkAppReq = new CheckAppReq();
            checkAppReq.setAppEnv(BeanTools.copy(environment, AppEnv::new));
            checkAppReq.setApplication(BeanTools.copy(instance, Application::new));
            AgentBaseDto checkResult = agentUnifiedService.check(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType(),
                environment, checkAppReq);
            if (Long.parseLong(checkResult.getErrorCode()) != 0) {
                log.error("mysql single instance check error before database scan. env id: {}", environment.getUuid());
                if (!VerifyUtil.isEmpty(instance.getAuth())) {
                    StringUtil.clean(instance.getAuth().getAuthPwd());
                }
                throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_AUTH_FAILED, "check error.");
            }
            ListResourceReq mysqlDbsRequest = new ListResourceReq();
            mysqlDbsRequest.setAppEnv(checkAppReq.getAppEnv());
            mysqlDbsRequest.setApplication(checkAppReq.getApplication());
            AgentDetailDto database = agentUnifiedService.getDetail(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType(),
                environment.getEndpoint(), environment.getPort(), mysqlDbsRequest);
            List<AppResource> resourceList = database.getResourceList();
            if (ObjectUtils.isEmpty(resourceList)) {
                log.warn("The instance {} server is in failure or auth is modified.", instance.getUuid());
                instance.getExtendInfo().put(LINK_STATUS_KEY, LinkStatusEnum.OFFLINE.getStatus().toString());
                return;
            }
            // 定期扫描数据库的过程中，更新实例信息
            mysqlInstanceProvider.refreshClusterInstance(instance, environment);
            resourceList.forEach(resource -> {
                ProtectedResource databaseResource = BeanTools.copy(resource, ProtectedResource::new);
                databaseResource.setParentName(instance.getName());
                databaseResource.setPath(environment.getEndpoint());
                Map<String, String> extendInfo = databaseResource.getExtendInfo();
                if (extendInfo != null) {
                    // 设置version
                    instance.setVersion(extendInfo.get(VERSION));
                    databaseResource.setVersion(extendInfo.get(VERSION));
                    // 设置数据库的集群类型
                    databaseResource.setExtendInfoByKey(CLUSTER_TYPE, instance.getExtendInfoByKey(CLUSTER_TYPE));
                }
                resources.add(databaseResource);
            });
            handleDestroyDatabase(instance, resources);
        });
        return resources;
    }

    /**
     * 对不存在的数据库进行删除操作
     *
     * @param instance 实例
     * @param currentResources 当前资源
     */
    public void handleDestroyDatabase(ProtectedResource instance, List<ProtectedResource> currentResources) {
        Set<String> resourceUuids = resourceService.queryRelatedResourceUuids(instance.getUuid(), new String[] {});
        List<String> currentDatabases = currentResources.stream()
            .map(ProtectedResource::getName)
            .collect(Collectors.toList());
        // 需要删除的资源
        resourceUuids.stream()
            .filter(s -> !StringUtils.equals(s, instance.getUuid()))
            .map(resourceService::getResourceById)
            .filter(Optional::isPresent)
            .map(Optional::get)
            .filter(protectedResource -> !currentDatabases.contains(protectedResource.getName()))
            .forEach(protectedResource -> {
                if (Objects.nonNull(protectedResource.getProtectedObject())) {
                    log.info("delete protect object,resource_id:{}", protectedResource.getUuid());
                    deleteProtectedObjects(protectedResource.getUuid());
                }
                ResourceDeleteParams resourceDeleteParams = new ResourceDeleteParams(true, true,
                    new String[] {protectedResource.getUuid()});
                resourceService.delete(resourceDeleteParams);
                log.info("delete resource success,resource_id:{}", protectedResource.getUuid());
            });
    }

    private void deleteProtectedObjects(String resourceId) {
        ProtectionBatchOperationReq req = new ProtectionBatchOperationReq();
        req.setResourceIds(singletonList(resourceId));
        log.info("delete protection resource_id:{}", resourceId);
        try {
            protectObjectRestApi.deleteProtectedObjects(req);
        } catch (FeignException | LegoCheckedException | LegoUncheckedException e) {
            log.error("delete protection error, resource_id: {},message:{}", resourceId, e.getMessage());
        }
    }

    /**
     * 判断是否需要扫描数据库
     *
     * @param instances 实例列表
     * @return 结果
     */
    private boolean needScan(List<ProtectedResource> instances) {
        if (CollectionUtils.isEmpty(instances)) {
            return false;
        }
        String clusterType = instances.get(0).getExtendInfo().get(CLUSTER_TYPE);
        return !MysqlConstants.EAPP.equals(clusterType);
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