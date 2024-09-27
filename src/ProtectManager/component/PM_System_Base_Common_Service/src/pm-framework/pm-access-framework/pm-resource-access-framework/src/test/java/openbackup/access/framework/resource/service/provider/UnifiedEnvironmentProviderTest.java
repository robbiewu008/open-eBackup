package openbackup.access.framework.resource.service.provider;

import openbackup.access.framework.resource.service.provider.UnifiedEnvironmentCheckProvider;
import openbackup.access.framework.resource.service.provider.UnifiedEnvironmentProvider;
import openbackup.access.framework.resource.service.provider.UnifiedHealthCheckProvider;
import openbackup.access.framework.resource.service.provider.UnifiedResourceScanProvider;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.core.plugin.DefaultPluginConfigManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.mock;

/**
 * 功能描述: UnifiedEnvironmentProviderTest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-01
 */
public class UnifiedEnvironmentProviderTest {
    private PluginConfigManager pluginConfigManager;
    private ProviderManager providerManager;
    private UnifiedResourceScanProvider unifiedScanProvider;
    private UnifiedEnvironmentCheckProvider unifiedCheckProvider;
    private UnifiedHealthCheckProvider unifiedHealthCheckProvider;
    private UnifiedEnvironmentProvider unifiedEnvironmentProvider;

    @Before
    public void init() {
        pluginConfigManager = new DefaultPluginConfigManager();
        pluginConfigManager.init();
        ProviderManager providerManager = mock(ProviderManager.class);
        UnifiedResourceScanProvider unifiedScanProvider = mock(UnifiedResourceScanProvider.class);
        UnifiedEnvironmentCheckProvider unifiedCheckProvider = mock(UnifiedEnvironmentCheckProvider.class);
        UnifiedHealthCheckProvider unifiedHealthCheckProvider = mock(UnifiedHealthCheckProvider.class);
        unifiedEnvironmentProvider = new UnifiedEnvironmentProvider(pluginConfigManager, providerManager,
                unifiedScanProvider, unifiedCheckProvider, unifiedHealthCheckProvider);
    }

    /**
     * 用例场景：测试读取配置文件中是否使用unifiedEnvironmentProvider的配置开关
     * 前置条件：已存在特性的plugin_json配置文件
     * 检查点：applicable方法返回配置文件中配置的useUnifiedProvider值
     */
    @Test
    public void test_applicable_return_true() {
        boolean applicable = unifiedEnvironmentProvider.applicable(ResourceSubTypeEnum.KUBERNETES.getType());
        assertTrue(applicable);
    }
}