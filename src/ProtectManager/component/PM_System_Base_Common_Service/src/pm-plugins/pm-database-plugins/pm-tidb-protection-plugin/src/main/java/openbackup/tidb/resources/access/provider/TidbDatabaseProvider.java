/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tidb.resources.access.provider;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.service.TidbService;
import openbackup.tidb.resources.access.util.TidbUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * TidbDatabaseProvider
 *
 * @author w00426202
 * @since 2023-07-15
 */
@Slf4j
@Component
public class TidbDatabaseProvider implements ResourceProvider {
    private final TidbService tidbService;

    private final JsonSchemaValidator jsonSchemaValidator;

    private final ResourceService resourceService;

    /**
     * 构造函数
     *
     * @param tidbService tidbService
     * @param jsonSchemaValidator jsonSchemaValidator
     * @param resourceService resourceService
     */
    public TidbDatabaseProvider(TidbService tidbService, JsonSchemaValidator jsonSchemaValidator,
        ResourceService resourceService) {
        this.tidbService = tidbService;
        this.jsonSchemaValidator = jsonSchemaValidator;
        this.resourceService = resourceService;
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.TIDB_DATABASE.getType().equals(object.getSubType());
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
        String path = StringUtils.joinWith("/", resource.getExtendInfo().get(TidbConstants.DATABASE_NAME));
        resource.setPath(path);

        TidbUtil.checkTidbReqParam(resource);
        jsonSchemaValidator.doValidate(resource, ResourceSubTypeEnum.TIDB_DATABASE.getType());

        // 检查数据库对应的集群是否在线
        ProtectedResource clusterResource = tidbService.getResourceByCondition(resource.getParentUuid());
        tidbService.checkResourceStatus(clusterResource);

        // 检查已选择的集群名字是否重复
        tidbService.checkDuplicateResource(resource, TidbConstants.DATABASE_NAME);

        ProtectedResource agentResource = tidbService.getAgentResource(clusterResource);

        // 检查数据库的健康状态
        TidbUtil.wrapExtendInfo2Add(resource, clusterResource);
        resource.setAuth(clusterResource.getAuth());
        tidbService.checkHealth(resource, agentResource, ResourceSubTypeEnum.TIDB_DATABASE.getType(),
            TidbConstants.CHECK_DB);
        Map<String, Object> updateEnty = new HashMap<>();
        updateEnty.put(TidbConstants.PARENT_NAME, resource.getName());
        log.info("TidbDatabaseProvider# beforeCreate resource info , uuid is {}, parentName is {}", resource.getUuid(),
            resource.getParentName());
        resourceService.updateSubResource(Arrays.asList(resource.getUuid()), updateEnty);
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        beforeCreate(resource);
    }
}
