package openbackup.data.access.framework.protection.plugin;

import openbackup.data.access.framework.core.plugin.DefaultPluginConfigManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;

import org.junit.Assert;
import org.junit.Test;

public class PluginConfigManagerTest {
    private static final String subType = "Mysql";

    private PluginConfigManager pluginConfigManager = new DefaultPluginConfigManager();

    @Test
    public void test_resolve_plugin_file() {
        pluginConfigManager.init();
        PluginConfig pluginConfig = pluginConfigManager.getPluginConfig(subType).orElse(null);
        Assert.assertNotNull(pluginConfig);
        Assert.assertEquals(pluginConfig.getSubType(), subType);
        Assert.assertTrue(pluginConfig.getConfigMap().size() > 0);
    }
}
