/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oracle.copy;

import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述: OracleCapabilityProviderTest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2023-03-08
 */
public class OracleCapabilityProviderTest {
    /**
     * 用例场景：测试获取Oracle副本支持的能力
     * 前置条件：无
     * 检 查 点：获取Oracle副本支持的能力正确
     */
    @Test
    public void test_support_features() {
        OracleCapabilityProvider provider = new OracleCapabilityProvider();
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.ORACLE.getType()));
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.ORACLE_CLUSTER.getType()));
        List<CopyFeatureEnum> copyFeatureEnums = provider.supportFeatures();
        Assert.assertTrue(copyFeatureEnums.containsAll(Arrays.asList(CopyFeatureEnum.RESTORE,
                CopyFeatureEnum.MOUNT, CopyFeatureEnum.INSTANT_RESTORE)));
    }
}