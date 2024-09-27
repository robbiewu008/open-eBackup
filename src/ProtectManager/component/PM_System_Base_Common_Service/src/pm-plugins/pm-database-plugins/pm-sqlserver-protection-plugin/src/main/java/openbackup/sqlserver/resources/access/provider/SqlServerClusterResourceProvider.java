/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.sqlserver.resources.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Objects;

/**
 * sqlserver集群 新加provider实现resourceprovider
 *
 * @author twx1009756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2023-1-5
 */
@Component
@Slf4j
public class SqlServerClusterResourceProvider implements ResourceProvider {
    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    /**
     * 资源重名检查配置
     *
     * @return SQL Server实例注册重名检查配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = ResourceProvider.super.getResourceFeature();
        // SQL Server集群扫描不需要刷新主机信息
        resourceFeature.setShouldUpdateDependencyHostInfoWhenScan(false);
        return resourceFeature;
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return Objects.nonNull(protectedResource) && Objects.equals(protectedResource.getSubType(),
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER.getType());
    }
}