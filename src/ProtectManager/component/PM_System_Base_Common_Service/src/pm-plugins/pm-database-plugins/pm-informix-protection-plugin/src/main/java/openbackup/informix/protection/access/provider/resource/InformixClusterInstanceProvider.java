/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.informix.protection.access.provider.resource;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * informix集群实例注册provider
 *
 * @author zwx951267
 * @version [DataBackup 1.5.0]
 * @since 2023-05-10
 */
@Component
@Slf4j
public class InformixClusterInstanceProvider implements ResourceProvider {
    private final InformixService informixService;

    /**
     * InformixClusterInstanceProvider
     *
     * @param informixService informixService
     */
    public InformixClusterInstanceProvider(InformixService informixService) {
        this.informixService = informixService;
    }

    /**
     * 资源重名检查配置
     *
     * @return ResourceFeature
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = new ResourceFeature();
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        return resourceFeature;
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param protectedResource object 受保护资源
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType().equals(protectedResource.getSubType());
    }

    /**
     * 检查informix实例资源连通性检查
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void check(ProtectedResource resource) {
        log.info("Start to create informix cluster instance. resource name: {}, resource subtype : {}",
            resource.getName(), resource.getSubType());
        informixService.doClusterInstanceAction(resource, true);
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    /**
     * 检查受保护资源informix实例修改连通性检查
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    @Override
    public void updateCheck(ProtectedResource resource) {
        log.info("Start to update informix cluster instance. resource name: {}, resource subtype : {}",
            resource.getName(), resource.getSubType());
        informixService.doClusterInstanceAction(resource, false);
    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        log.info("Start to check cluster health.");
        informixService.healthCheckOfClusterInstance(resource);
    }

    @Override
    public ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        // 删除文件系统
        informixService.removeRepositoryOfResource(resource.getUuid());
        return ResourceDeleteContext.defaultValue();
    }
}
