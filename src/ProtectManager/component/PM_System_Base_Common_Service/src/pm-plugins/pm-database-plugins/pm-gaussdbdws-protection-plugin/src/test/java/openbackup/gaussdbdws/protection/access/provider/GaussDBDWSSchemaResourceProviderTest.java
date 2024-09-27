package openbackup.gaussdbdws.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.Optional;

/**
 * DWS集群 Schema 资源测试类
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-17
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({EnvironmentLinkStatusHelper.class})
public class GaussDBDWSSchemaResourceProviderTest {
    private GaussDBDWSSchemaResourceProvider gaussDBDWSSchemaResourceProvider;

    private ProviderManager providerManager;

    private ResourceService resourceService;

    private GaussDBBaseService gaussDBBaseService;

    private ProtectedResourceChecker protectedResourceChecker;

    private TaskRepositoryManager taskRepositoryManager;

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = Mockito.mock(
        ResourceConnectionCheckProvider.class);

    @Before
    public void init() {
        this.protectedResourceChecker = Mockito.mock(ProtectedResourceChecker.class);
        this.providerManager = Mockito.mock(ProviderManager.class);
        this.taskRepositoryManager = Mockito.mock(TaskRepositoryManager.class);
        PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);
        this.resourceService = Mockito.mock(ResourceService.class);
        this.gaussDBBaseService = new GaussDBBaseService(resourceService, protectedResourceChecker, providerManager,
            resourceConnectionCheckProvider, taskRepositoryManager);
        this.gaussDBDWSSchemaResourceProvider = new GaussDBDWSSchemaResourceProvider(this.providerManager,
            pluginConfigManager, gaussDBBaseService);
        ProtectedResource protectedResource = MockProviderParameter.getProtectedResource();
        PageListResponse<ProtectedResource> pageListResponse
            = MockProviderParameter.getProtectedResourcePageListResponse();
        PowerMockito.when(resourceService.query(LegoNumberConstant.ZERO, LegoNumberConstant.ONE,
            Collections.singletonMap("uuid", protectedResource.getRootUuid()))).thenReturn(pageListResponse);
        PowerMockito.when(resourceService.query(LegoNumberConstant.ZERO, LegoNumberConstant.ONE,
            ImmutableMap.of(DatabaseConstants.ROOT_UUID, protectedResource.getRootUuid(), "subType",
                ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()))).thenReturn(pageListResponse);
        PowerMockito.when(resourceService.query(LegoNumberConstant.ZERO, DwsConstant.QUERY_QUANTITY_SPECIFICATIONS,
            ImmutableMap.of(DatabaseConstants.ROOT_UUID, protectedResource.getRootUuid(), "subType",
                ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()))).thenReturn(pageListResponse);
        Optional<ProtectedResource> protectedResources = Optional.of(protectedResource);
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("xxxxxx")).thenReturn(protectedResources);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn(
            LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 用例场景：GaussDB(DWS) Schema环境检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType());
        Assert.assertTrue(gaussDBDWSSchemaResourceProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：创建Schema备份集前检查
     * 前置条件：NA
     * 检查点: 检查成功
     */
    @Test
    public void check_before_create_success() {
        gaussDBDWSSchemaResourceProvider.beforeCreate(MockProviderParameter.getProtectedResource());
    }

    /**
     * 用例场景：更新 Schema备份集前检查
     * 前置条件：NA
     * 检查点: 检查成功,不报错
     */
    @Test
    public void check_before_update_success() {
        gaussDBDWSSchemaResourceProvider.beforeUpdate(MockProviderParameter.getProtectedResource());
    }

    /**
     * 用例场景：检测DWS Schema 是否支持lanfree的应用
     * 前置条件：NA
     * 检查点: 不支持
     */
    @Test
    public void get_resource_feature_success() {
        Assert.assertFalse(gaussDBDWSSchemaResourceProvider.getResourceFeature().isSupportedLanFree());
    }

    /**
     * 用例场景：创建schema集时 查询父资源查询失败报错;
     * 前置条件：NA
     * 检查点: 检查失败抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_no_find_resource_of_id_when_before_create() {
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("xxxxxx")).thenReturn(Optional.empty());
        Assert.assertThrows(LegoCheckedException.class,
            () -> gaussDBDWSSchemaResourceProvider.beforeCreate(MockProviderParameter.getProtectedResource()));
    }
}
