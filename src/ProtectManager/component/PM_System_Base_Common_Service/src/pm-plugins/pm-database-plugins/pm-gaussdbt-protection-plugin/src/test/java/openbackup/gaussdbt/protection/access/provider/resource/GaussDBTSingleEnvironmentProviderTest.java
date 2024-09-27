package openbackup.gaussdbt.protection.access.provider.resource;


import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.gaussdbt.protection.access.provider.service.GaussDBTSingleService;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * {@link GaussDBTSingleEnvironmentProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/21
 */
public class GaussDBTSingleEnvironmentProviderTest {
    private ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private PluginConfigManager pluginConfigManager = PowerMockito.mock(PluginConfigManager.class);

    private GaussDBTSingleService gaussDBTSingleService = PowerMockito.mock(GaussDBTSingleService.class);

    private GaussDBTSingleEnvironmentProvider provider = new GaussDBTSingleEnvironmentProvider(providerManager,
        pluginConfigManager, gaussDBTSingleService);

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_gaussdbt_single_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.GAUSSDBT_SINGLE.getType()));
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.MYSQL_CLUSTER.getType()));
    }

    /**
     * 用例场景：GaussDBT单机健康检查
     * 前置条件：节点在线
     * 检查点: 检查成功
     */
    @Test
    public void gaussdbt_single_health_check_success() {
        provider.validate(new ProtectedEnvironment());
    }

    /**
     * 用例场景：GaussDBT单机环境注册
     * 前置条件：环境资源未注册
     * 检查点: 无异常抛出
     */
    @Test
    public void gaussdbt_single_environment_before_create_check() {
        ProtectedEnvironment environment = buildEnvironment();
        provider.register(environment);
    }

    /**
     * 用例场景：GaussDBT单机环境更新
     * 前置条件：环境资源已经注册
     * 检查点: 无异常抛出
     */
    @Test
    public void gaussdbt_single_environment_before_update_check() {
        ProtectedEnvironment environment = buildEnvironment();
        environment.setUuid(UUIDGenerator.getUUID());
        provider.register(environment);
    }

    private ProtectedEnvironment buildEnvironment() {
        ProtectedEnvironment host = new ProtectedEnvironment();
        host.setUuid(UUIDGenerator.getUUID());
        List<ProtectedResource> resources = new ArrayList<>();
        resources.add(host);
        Map<String, List<ProtectedResource>> agents = new HashMap<>();
        agents.put(DatabaseConstants.AGENTS, resources);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(agents);
        environment.setSubType(ResourceSubTypeEnum.GAUSSDBT_SINGLE.getType());
        return environment;
    }
}