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
package openbackup.tidb.resources.access.provider;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.service.TidbService;
import openbackup.tidb.resources.access.util.TidbUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * TidbTableProvider
 *
 */

@Slf4j
@Component
public class TidbTableProvider implements ResourceProvider {
    private final TidbService tidbService;

    private final JsonSchemaValidator jsonSchemaValidator;

    /**
     * 构造函数
     *
     * @param tidbService tidbService
     * @param jsonSchemaValidator jsonSchemaValidator
     */
    public TidbTableProvider(TidbService tidbService, JsonSchemaValidator jsonSchemaValidator) {
        this.tidbService = tidbService;
        this.jsonSchemaValidator = jsonSchemaValidator;
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.TIDB_TABLE.getType().equals(object.getSubType());
    }

    /**
     * 检查受保护资源，创建逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        validTableInfo(resource);

        // 检查注册表数量
        List<String> newTablespaceList = getTableList(resource);
        checkTablespaceNum(newTablespaceList);

        tidbService.checkDuplicateResource(newTablespaceList, resource.getParentUuid());

        String path = StringUtils.joinWith("/", resource.getExtendInfo().get(TidbConstants.DATABASE_NAME));
        resource.setPath(path);
    }

    private void validTableInfo(ProtectedResource resource) {
        TidbUtil.checkTidbReqParam(resource);
        jsonSchemaValidator.doValidate(resource, ResourceSubTypeEnum.TIDB_TABLE.getType());
        ProtectedResource databaseResource = tidbService.getResourceByCondition(resource.getParentUuid());
        ProtectedResource clusterResource = tidbService.getResourceByCondition(databaseResource.getParentUuid());
        tidbService.checkResourceStatus(clusterResource);

        // 检查注册表的健康状态
        TidbUtil.wrapExtendInfo2Add(resource, clusterResource);
        String realTableName = resource.getExtendInfo().get(TidbConstants.TABLE_NAME);
        resource.getExtendInfo().put(TidbConstants.TABLE_NAME_LIST, realTableName);

        List<String> registeredTablespaceList = JsonUtil.read(realTableName, List.class);
        log.info("registeredTablespaceList is {}", JsonUtil.json(registeredTablespaceList));
        if (CollectionUtils.isEmpty(registeredTablespaceList)
            || registeredTablespaceList.size() > TidbConstants.TABLE_REGISTER_LIMIT_NUM) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "The table name list is invalid.");
        }

        resource.setAuth(clusterResource.getAuth());
        ProtectedResource agentResource = tidbService.getAgentResource(clusterResource);
        tidbService.checkHealth(resource, agentResource, ResourceSubTypeEnum.TIDB_TABLE.getType(),
            TidbConstants.CHECK_TABLE);
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("Start update tidb tablespace parameters check. resource name: {}, database uuid: {}",
            resource.getName(), resource.getParentUuid());
        validTableInfo(resource);

        List<String> newTablespaceList = getTableList(resource);
        checkTablespaceNum(newTablespaceList);
        checkUpdateTablespaceIsRegistered(resource);

        log.info("End update tidb tablespace parameters check. resource name: {}, database uuid: {}",
            resource.getName(), resource.getParentUuid());
    }

    private void checkUpdateTablespaceIsRegistered(ProtectedResource resource) {
        ProtectedResource oldTablespace = tidbService.getResourceByCondition(resource.getUuid());
        List<String> oldTablespaceList = JsonUtil.read(oldTablespace.getExtendInfoByKey(TidbConstants.TABLE_NAME),
            List.class);
        List<String> newTablespaceList = JsonUtil.read(resource.getExtendInfoByKey(TidbConstants.TABLE_NAME),
            List.class);
        newTablespaceList.removeAll(oldTablespaceList);
        if (newTablespaceList.isEmpty()) {
            return;
        }
        tidbService.checkDuplicateResource(newTablespaceList, resource.getParentUuid());
    }

    private List<String> getTableList(ProtectedResource resource) {
        String tablespace = resource.getExtendInfoByKey(TidbConstants.TABLE_NAME);
        if (StringUtils.isBlank(tablespace)) {
            log.error("Select tablespace is empty. resource id: {}, name: {}", resource.getUuid(), resource.getName());
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Select tablespace is empty.");
        }
        return Arrays.asList(tablespace.split(DatabaseConstants.SPLIT_CHAR));
    }

    private void checkTablespaceNum(List<String> tablespaceList) {
        if (tablespaceList.size() > TidbConstants.TABLE_REGISTER_LIMIT_NUM) {
            log.error("TiDB register table num exceed 256 .");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
    }
}
