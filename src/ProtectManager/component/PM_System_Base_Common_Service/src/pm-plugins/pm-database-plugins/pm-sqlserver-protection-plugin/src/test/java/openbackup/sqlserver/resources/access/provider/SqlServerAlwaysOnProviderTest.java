/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
package openbackup.sqlserver.resources.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import org.junit.Assert;
import org.junit.Test;

/**
 * SQL Server AlwaysOnProvider
 *
 * @author swx1010572
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-05
 */
public class SqlServerAlwaysOnProviderTest {
    private final SqlServerAlwaysOnProvider sqlServerAlwaysOnProvider = new SqlServerAlwaysOnProvider();

    /**
     * 用例场景：SQL Server 清理固有属性
     * 前置条件：传入parentName和parentUuid
     * 检查点：parentName返回不为空允许更改,parentUuid返回为空不允许更改
     */
    @Test
    public void clean_unmodifiable_fields_when_update_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setParentUuid("111");
        protectedResource.setParentName("sqlServer_cluster");
        sqlServerAlwaysOnProvider.cleanUnmodifiableFieldsWhenUpdate(protectedResource);
        Assert.assertEquals("sqlServer_cluster",protectedResource.getParentName());
        Assert.assertNull(protectedResource.getParentUuid());
    }
}
