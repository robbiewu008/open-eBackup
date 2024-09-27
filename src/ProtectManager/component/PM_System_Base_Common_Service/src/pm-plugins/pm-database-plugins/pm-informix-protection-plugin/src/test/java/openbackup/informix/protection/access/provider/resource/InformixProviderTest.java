package openbackup.informix.protection.access.provider.resource;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;

/**
 * InformixProviderTest
 *
 * @author zWX951267
 * @version [DataBackup 1.5.0]
 * @since 2023-05-17
 */
public class InformixProviderTest {
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = PowerMockito.mock(PluginConfigManager.class);

    private final InformixService informixService = PowerMockito.mock(InformixService.class);

    private final JsonSchemaValidator jsonSchemaValidator = PowerMockito.mock(JsonSchemaValidator.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final InformixProvider provider = new InformixProvider(
            providerManager, pluginConfigManager, informixService, jsonSchemaValidator, resourceService);

    /**
     * 用例场景：informix环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.INFORMIX_SERVICE.getType()));
    }

    /**
     * 用例场景：informix服务注册
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void register_check_success() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("informix-service");
        provider.register(environment);
        Mockito.verify(informixService, Mockito.times(1)).checkHostInfo(environment);
        Mockito.verify(informixService, Mockito.times(1)).checkLogBackupItem(environment);
    }

    /**
     * 用例场景：健康检查
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void healthCheck() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        PageListResponse<ProtectedResource> data = new PageListResponse<>();
        data.setRecords(new ArrayList<>());
        Mockito.when(resourceService.query(Mockito.anyInt(), Mockito.anyInt(), Mockito.anyMap())).thenReturn(data);
        provider.validate(environment);
        Mockito.verify(informixService, Mockito.times(1)).checkHostInfo(environment);
    }
}
