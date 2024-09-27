package openbackup.tdsql.resources.access.provider;

import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;

/**
 * TdsqlCapabilityProviderTest
 *
 * @author z00445440
 * @since 2023-12-19
 */
public class TdsqlCapabilityProviderTest {
    /**
     * 用例场景：测试获取Tdsql副本支持的能力
     * 前置条件：无
     * 检 查 点：获取Tdsql副本支持的能力正确
     */
    @Test
    public void test_support_features() {
        TdsqlCapabilityProvider provider = new TdsqlCapabilityProvider();
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType()));
        List<CopyFeatureEnum> copyFeatureEnums = provider.supportFeatures();
        Assert.assertTrue(copyFeatureEnums.containsAll(Arrays.asList(CopyFeatureEnum.RESTORE, CopyFeatureEnum.MOUNT)));
    }

}