/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tpops.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tpops.protection.access.service.TpopsGaussDBService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Objects;

/**
 * GaussDB资源类提供者
 *
 * @author x30021699
 * @since 2023-05-09
 */
@Component
@Slf4j
public class TpopsGaussDBResourceProvider implements ResourceProvider {
    @Autowired
    private TpopsGaussDBService tpopsGaussDbService;

    /**
     * 联通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType()
            .equals(object.getSubType());
    }

    /**
     * 设置path信息，否则复制的时候会报错
     *
     * @param resource 资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("TpopsGaussDBResourceProvider beforeCreate setPath:{}, resource.getEndpoint:{}", resource.getPath(),
            resource.getEndpoint());
    }

    /**
     * 集群环境修改的时候会去检查连通性、实例和数据库资源属于自动扫描。更新资源时不再去检查
     *
     * @param resource 资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    /**
     * 不支持lanfree的应用实现ResourceProvider接口
     *
     * @return GaussDB资源是否更新主机信息配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = ResourceProvider.super.getResourceFeature();
        resourceFeature.setSupportedLanFree(false);
        return resourceFeature;
    }
}
