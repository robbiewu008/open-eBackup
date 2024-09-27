/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.sqlserver.resources.access.provider;

import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Objects;

/**
 * SQL Server AlwaysOnProvider
 *
 * @author swx1010572
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-05
 */
@Component
@Slf4j
public class SqlServerAlwaysOnProvider extends DefaultResourceProvider {
    @Override
    public void cleanUnmodifiableFieldsWhenUpdate(ProtectedResource resource) {
        resource.setRootUuid(null);
        resource.setSubType(null);
        resource.setUserId(null);
        resource.setAuthorizedUser(null);
        resource.setParentUuid(null);
        resource.setCreatedTime(null);
        resource.setProtectionStatus(null);
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param object object 受保护资源
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && Objects.equals(object.getSubType(),
            ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType());
    }
}
