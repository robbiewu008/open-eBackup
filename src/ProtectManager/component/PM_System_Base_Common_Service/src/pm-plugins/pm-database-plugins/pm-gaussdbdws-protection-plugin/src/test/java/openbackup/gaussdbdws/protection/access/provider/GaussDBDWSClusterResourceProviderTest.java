package openbackup.gaussdbdws.protection.access.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

/**
 * DWS集群 资源测试类
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.5.0]
 * @since 2023-02-22
 */
public class GaussDBDWSClusterResourceProviderTest {
    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);

    private final GaussDBDWSClusterResourceProvider gaussDBDWSClusterResourceProvider
        = new GaussDBDWSClusterResourceProvider(this.providerManager, pluginConfigManager);

    /**
     * 用例场景：检测DWS 集群 是否支持lanfree的应用
     * 前置条件：NA
     * 检查点: 不支持
     */
    @Test
    public void get_resource_feature_success() {
        Assert.assertFalse(gaussDBDWSClusterResourceProvider.getResourceFeature().isSupportedLanFree());
    }

}
