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
package openbackup.db2.protection.access.service.impl;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.LockedValueEnum;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.service.Db2InstanceService;
import openbackup.db2.protection.access.service.Db2TablespaceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import com.google.common.collect.Lists;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * db2表空间服务
 *
 */
@Service
public class Db2TablespaceServiceImpl implements Db2TablespaceService {
    private final ResourceService resourceService;

    private final AgentUnifiedService agentService;

    private final Db2InstanceService db2instanceService;

    public Db2TablespaceServiceImpl(ResourceService resourceService, AgentUnifiedService agentService,
        Db2InstanceService db2instanceService) {
        this.resourceService = resourceService;
        this.agentService = agentService;
        this.db2instanceService = db2instanceService;
    }

    @Override
    public void setTablespaceLockedStatus(String databaseId, PageListResponse<ProtectedResource> tablespaceList) {
        List<ProtectedResource> registeredTablespace = registeredTablespace(databaseId);
        List<String> lockedTablespace = lockedTablespace(registeredTablespace);
        tablespaceList.getRecords().forEach(tablespace -> {
            if (lockedTablespace.contains(tablespace.getName())) {
                tablespace.setExtendInfoByKey(DatabaseConstants.EXTEND_INFO_KEY_IS_LOCKED,
                    LockedValueEnum.OPTIONAL.getLocked());
            }
        });
    }

    private List<ProtectedResource> registeredTablespace(String databaseId) {
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.DB2_TABLESPACE.getType());
        filter.put(DatabaseConstants.PARENT_UUID, databaseId);
        PageListResponse<ProtectedResource> data;
        int pageNo = 0;
        List<ProtectedResource> resources = new ArrayList<>();
        do {
            data = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, filter);
            resources.addAll(data.getRecords());
            pageNo++;
        } while (data.getRecords().size() >= IsmNumberConstant.HUNDRED);
        return resources;
    }

    private List<String> lockedTablespace(List<ProtectedResource> registeredTablespace) {
        return registeredTablespace.stream()
            .map(tablespace -> Arrays.asList(
                tablespace.getExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY).split(DatabaseConstants.SPLIT_CHAR)))
            .flatMap(Collection::stream)
            .collect(Collectors.toList());
    }

    @Override
    public PageListResponse<ProtectedResource> querySingleTablespace(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        ListResourceV2Req listResourceV2Req = buildListResourceReq(environment, environmentConditions);
        return agentService.getDetailPageList(
            environmentConditions.getResourceType(), environment.getEndpoint(), environment.getPort(),
            listResourceV2Req);
    }

    private ListResourceV2Req buildListResourceReq(ProtectedEnvironment env,
        BrowseEnvironmentResourceConditions environmentConditions) {
        ListResourceV2Req req = new ListResourceV2Req();
        req.setPageNo(environmentConditions.getPageNo());
        req.setPageSize(environmentConditions.getPageSize());
        req.setAppEnv(BeanTools.copy(env, AppEnv::new));
        req.setApplications(Lists.newArrayList(buildApplication(environmentConditions.getParentId())));
        return req;
    }

    private Application buildApplication(String parentId) {
        Application application = new Application();
        application.setType(ResourceTypeEnum.DATABASE.getType());
        application.setSubType(ResourceSubTypeEnum.DB2_TABLESPACE.getType());
        ProtectedResource database = resourceService.getResourceById(parentId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This database is not exist"));
        application.setParentName(database.getParentName());
        application.setName(database.getName());
        application.setUuid(database.getUuid());
        ProtectedResource instance = resourceService.getResourceById(database.getParentUuid())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This instance is not exist"));
        application.setAuth(instance.getAuth());
        return application;
    }

    @Override
    public PageListResponse<ProtectedResource> queryClusterTablespace(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        ListResourceV2Req listResourceV2Req = buildListResourceReq(environmentConditions);
        return agentService.getDetailPageList(
            environmentConditions.getResourceType(), listResourceV2Req.getAppEnv().getEndpoint(),
            listResourceV2Req.getAppEnv().getPort(), listResourceV2Req);
    }

    private ListResourceV2Req buildListResourceReq(BrowseEnvironmentResourceConditions environmentConditions) {
        ListResourceV2Req req = new ListResourceV2Req();
        req.setPageNo(environmentConditions.getPageNo());
        req.setPageSize(environmentConditions.getPageSize());
        req.setAppEnv(buildAppEnv(environmentConditions.getParentId()));
        req.setApplications(Lists.newArrayList(buildBrowseApp(environmentConditions.getParentId())));
        return req;
    }

    private Application buildBrowseApp(String parentId) {
        Application application = new Application();
        application.setType(ResourceTypeEnum.DATABASE.getType());
        application.setSubType(ResourceSubTypeEnum.DB2_TABLESPACE.getType());
        ProtectedResource database = resourceService.getResourceById(parentId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This resource is not exist"));
        application.setParentName(database.getParentName());
        application.setName(database.getName());
        application.setUuid(database.getUuid());
        ProtectedResource clusterInstance = resourceService.getResourceById(database.getParentUuid())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This resource is not exist"));
        db2instanceService.filterClusterInstance(clusterInstance);
        ProtectedResource subInstance = extractSubInstanceByCatalog(clusterInstance, database);
        application.setAuth(subInstance.getAuth());
        return application;
    }

    private AppEnv buildAppEnv(String parentId) {
        ProtectedResource database = resourceService.getResourceById(parentId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This database is not exist"));
        ProtectedResource clusterInstance = resourceService.getResourceById(database.getParentUuid())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This instance is not exist"));
        db2instanceService.filterClusterInstance(clusterInstance);
        ProtectedResource subInstance = extractSubInstanceByCatalog(clusterInstance, database);
        return BeanTools.copy(extractEnvironmentByInstance(subInstance), AppEnv::new);
    }

    private ProtectedResource extractSubInstanceByCatalog(ProtectedResource clusterInstance,
        ProtectedResource database) {
        List<ProtectedResource> subInstances = clusterInstance.getDependencies().get(DatabaseConstants.CHILDREN);
        String catalogIp = database.getExtendInfoByKey(Db2Constants.CATALOG_IP_KEY);
        for (ProtectedResource subInstance : subInstances) {
            ProtectedEnvironment agent = extractEnvironmentByInstance(subInstance);
            if (StringUtils.isNotBlank(catalogIp) && agent.getExtendInfoByKey(ResourceConstants.AGENT_IP_LIST)
                .contains(catalogIp)) {
                return subInstance;
            }
        }
        return extractSubInstance(clusterInstance);
    }

    private ProtectedEnvironment extractEnvironmentByInstance(ProtectedResource subInstance) {
        return subInstance.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException("This instance environment is empty."));
    }

    private ProtectedResource extractSubInstance(ProtectedResource clusterInstance) {
        return clusterInstance.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException("Don't have sub instance."));
    }
}
