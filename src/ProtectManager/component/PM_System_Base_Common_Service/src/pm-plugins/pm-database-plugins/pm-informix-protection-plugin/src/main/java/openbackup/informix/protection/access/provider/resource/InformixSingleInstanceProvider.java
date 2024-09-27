/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.informix.protection.access.provider.resource;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * informix单实例注册provider
 *
 * @author zwx951267
 * @version [DataBackup 1.5.0]
 * @since 2023-05-10
 */
@Component
@Slf4j
public class InformixSingleInstanceProvider implements ResourceProvider {
    private final InformixService informixService;
    private final InstanceResourceService instanceResourceService;

    /**
     * InformixSingleInstanceProvider
     *
     * @param informixService informixService
     * @param instanceResourceService instanceResourceService
     */
    public InformixSingleInstanceProvider(
            InformixService informixService, InstanceResourceService instanceResourceService) {
        this.informixService = informixService;
        this.instanceResourceService = instanceResourceService;
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
     * 检查informix实例资源连通性检查
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("Start to create informix single instance. resource name: {}, resource subtype : {}",
                resource.getName(), resource.getSubType());
        // 设置path信息，否则复制的时候会报错
        String path = resource.getEnvironment().getEndpoint();
        resource.setPath(path);
        informixService.doSingleInstanceAction(resource, true);
    }

    /**
     * 检查受保护资源informix实例修改
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("Start to update informix single instance. resource name: {}, resource subtype : {}",
                resource.getName(), resource.getSubType());
        informixService.doSingleInstanceAction(resource, false);
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param protectedResource object 受保护资源
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType().equals(protectedResource.getSubType());
    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        instanceResourceService.healthCheckSingleInstance(resource);
    }
}
