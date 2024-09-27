package openbackup.gaussdbt.protection.access.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

/**
 * GaussDBTResourceProvider测试类
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-13
 */
public class GaussDBTResourceProviderTest {
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = PowerMockito.mock(PluginConfigManager.class);

    private GaussDBTResourceProvider gaussDBTResourceProvider = new GaussDBTResourceProvider(providerManager,
        pluginConfigManager);

    /**
     * 用例场景：gaussDBT类型识别
     * 前置条件：类型参数为GaussDBT
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        Assert.assertTrue(gaussDBTResourceProvider.applicable(resource));
    }

    /**
     * 用例场景：GaussDBT健康检查更新节点信息，不使用数据库框架再次校验
     * 前置条件：更新存在的GaussDBT资源
     * 检查点: 调用成功
     */
    @Test
    public void before_update_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        resource.setUuid("test_uuid");
        gaussDBTResourceProvider.beforeUpdate(resource);
    }
}
