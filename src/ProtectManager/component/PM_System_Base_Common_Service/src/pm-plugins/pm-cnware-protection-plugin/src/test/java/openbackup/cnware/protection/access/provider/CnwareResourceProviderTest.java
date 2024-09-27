/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
package openbackup.cnware.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import org.junit.Assert;
import org.junit.Test;

/**
 * CNware AlwaysOnProvider
 *
 * @author q30048244
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-12-30
 */
public class CnwareResourceProviderTest {
    private final CnwareResourceProvider cnwareResourceProvider = new CnwareResourceProvider();

    /**
     * 用例场景：SQL Server 清理固有属性
     * 前置条件：传入parentName和parentUuid
     * 检查点：parentName返回不为空允许更改,parentUuid返回为空不允许更改
     */
    @Test
    public void clean_unmodifiable_fields_when_update_success() {
        ProtectedResource protectedResource = mockObject();
        cnwareResourceProvider.cleanUnmodifiableFieldsWhenUpdate(protectedResource);
        Assert.assertEquals("cnware_testName",protectedResource.getParentName());
        Assert.assertNull(protectedResource.getParentUuid());
    }

    /**
     * 用例场景：Cnware备份插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        boolean cNwareVm = cnwareResourceProvider.applicable(mockObject());
        Assert.assertTrue(cNwareVm);
    }

    private ProtectedResource mockObject() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setParentUuid("111");
        protectedResource.setParentName("cnware_testName");
        protectedResource.setSubType("CNwareVm");
        return protectedResource;
    }
}
