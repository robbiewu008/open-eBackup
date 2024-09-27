/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.db2.protection.access.provider.resource;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.constant.Db2ErrorCode;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2TablespaceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * db2表空间创建provider
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2022-12-30
 */
@Component
@Slf4j
public class Db2TablespaceProvider implements ResourceProvider {
    private final ResourceService resourceService;

    private final ProtectedEnvironmentService environmentService;

    private final Db2TablespaceService db2TablespaceService;

    public Db2TablespaceProvider(ResourceService resourceService, ProtectedEnvironmentService environmentService,
        Db2TablespaceService db2TablespaceService) {
        this.resourceService = resourceService;
        this.environmentService = environmentService;
        this.db2TablespaceService = db2TablespaceService;
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("Start create db2 tablespace parameters check. resource name: {}, database uuid: {}",
            resource.getName(), resource.getParentUuid());
        checkTablespaceSupportClusterType(resource);
        List<String> newTablespaceList = getTablespace(resource);
        checkTablespaceNum(newTablespaceList);
        checkTablespaceIsRegistered(newTablespaceList, resource.getParentUuid());
        checkTablespaceIsExist(newTablespaceList, resource);
        setTablespaceProperties(resource);
        resource.setPath(environmentService.getEnvironmentById(resource.getRootUuid()).getEndpoint());
        log.info("End create db2 tablespace parameters check. resource name: {}, database uuid: {}", resource.getName(),
            resource.getParentUuid());
    }

    private void checkTablespaceSupportClusterType(ProtectedResource resource) {
        String clusterType = resource.getEnvironment().getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE);
        if (Db2ClusterTypeEnum.HADR.getType().equals(clusterType)) {
            log.error("Hadr don't support the creation of tablespace. resource name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Hadr don't support the creation of tablespace.");
        }
    }

    private void checkTablespaceIsExist(List<String> newTablespaceList, ProtectedResource resource) {
        PageListResponse<ProtectedResource> detailPageList = queryTablespace(resource);
        List<String> tablespaceList = detailPageList.getRecords()
            .stream()
            .map(ProtectedResource::getName)
            .collect(Collectors.toList());
        if (!tablespaceList.containsAll(newTablespaceList)) {
            log.error("Select tablespace is not exists. name: {}, query tablespace: {}, new tablespace: {}",
                resource.getName(), tablespaceList, newTablespaceList);
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Select tablespace is not exists.");
        }
    }

    private PageListResponse<ProtectedResource> queryTablespace(ProtectedResource resource) {
        ProtectedEnvironment environment = resource.getEnvironment();
        BrowseEnvironmentResourceConditions browseConditions = buildBrowseConditions(resource.getParentUuid());
        if (ResourceSubTypeEnum.DB2_CLUSTER.equalsSubType(environment.getSubType())) {
            return db2TablespaceService.queryClusterTablespace(environment, browseConditions);
        }
        return db2TablespaceService.querySingleTablespace(environment, browseConditions);
    }

    private BrowseEnvironmentResourceConditions buildBrowseConditions(String databaseId) {
        BrowseEnvironmentResourceConditions browseConditions = new BrowseEnvironmentResourceConditions();
        browseConditions.setResourceType(ResourceSubTypeEnum.DB2_TABLESPACE.getType());
        browseConditions.setParentId(databaseId);
        return browseConditions;
    }

    private List<String> getTablespace(ProtectedResource resource) {
        String tablespace = resource.getExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY);
        if (StringUtils.isBlank(tablespace)) {
            log.error("Select tablespace is empty. resource id: {}, name: {}", resource.getUuid(), resource.getName());
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Select tablespace is empty.");
        }
        return Arrays.asList(tablespace.split(DatabaseConstants.SPLIT_CHAR));
    }

    private void checkTablespaceNum(List<String> tablespaceList) {
        if (tablespaceList.size() > Db2Constants.TABLESPACE_SPECIFICATION) {
            throw new LegoCheckedException(Db2ErrorCode.CHECK_RESOURCES_SIZE_ERROR,
                new String[] {String.valueOf(Db2Constants.TABLESPACE_SPECIFICATION)},
                "Db2 tablespace size > " + Db2Constants.TABLESPACE_SPECIFICATION + ".");
        }
    }

    private void checkTablespaceIsRegistered(List<String> tablespaceList, String databaseId) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.DB2_TABLESPACE.getType());
        conditions.put(DatabaseConstants.PARENT_UUID, databaseId);
        int pageNo = 0;
        PageListResponse<ProtectedResource> data;
        do {
            data = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, conditions);
            data.getRecords().forEach(resource -> checkTablespace(resource, tablespaceList));
            pageNo++;
        } while (data.getRecords().size() >= IsmNumberConstant.HUNDRED);
    }

    private void checkTablespace(ProtectedResource resource, List<String> tablespaceList) {
        String registeredTablespace = resource.getExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY);
        List<String> registeredTablespaceList = Arrays.asList(registeredTablespace.split(DatabaseConstants.SPLIT_CHAR));
        if (!Collections.disjoint(tablespaceList, registeredTablespaceList)) {
            log.error("The select tablespace has been registered. uuids: {}", tablespaceList);
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "The tablespace is registered");
        }
    }

    private void setTablespaceProperties(ProtectedResource resource) {
        ProtectedResource database = getResourceById(resource.getParentUuid());
        resource.setExtendInfoByKey(Db2Constants.CATALOG_IP_KEY,
            database.getExtendInfoByKey(Db2Constants.CATALOG_IP_KEY));
        resource.setExtendInfoByKey(DatabaseConstants.INSTANCE_UUID_KEY, database.getParentUuid());
        resource.setExtendInfoByKey(DatabaseConstants.INSTANCE, database.getParentName());
        resource.setVersion(database.getVersion());
        resource.setExtendInfoByKey(DatabaseConstants.AGENTS, database.getExtendInfoByKey(DatabaseConstants.AGENTS));
        resource.setExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY,
            database.getExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY));
        resource.setExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE,
            database.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE));
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("Start update db2 tablespace parameters check. resource name: {}, database uuid: {}",
            resource.getName(), resource.getParentUuid());
        List<String> newTablespaceList = getTablespace(resource);
        checkTablespaceNum(newTablespaceList);
        checkUpdateTablespaceIsRegistered(resource);
        checkTablespaceIsExist(newTablespaceList, resource);
        setTablespaceProperties(resource);
        resource.setPath(environmentService.getEnvironmentById(resource.getRootUuid()).getEndpoint());
        log.info("End update db2 tablespace parameters check. resource name: {}, database uuid: {}", resource.getName(),
            resource.getParentUuid());
    }

    private void checkUpdateTablespaceIsRegistered(ProtectedResource resource) {
        ProtectedResource oldTablespace = getResourceById(resource.getUuid());
        List<String> oldTablespaceList = new ArrayList<>(Arrays.asList(
            oldTablespace.getExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY).split(DatabaseConstants.SPLIT_CHAR)));
        List<String> newTablespaceList = new ArrayList<>(Arrays.asList(
            resource.getExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY).split(DatabaseConstants.SPLIT_CHAR)));
        newTablespaceList.removeAll(oldTablespaceList);
        if (newTablespaceList.isEmpty()) {
            return;
        }
        checkTablespaceIsRegistered(newTablespaceList, resource.getParentUuid());
    }

    private ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This resource is not exists."));
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.DB2_TABLESPACE.equalsSubType(protectedResource.getSubType());
    }
}
