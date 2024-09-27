package openbackup.db2.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.service.Db2InstanceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * {@link Db2InstanceProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2022-12-29
 */
public class Db2InstanceProviderTest {
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = PowerMockito.mock(
        ResourceConnectionCheckProvider.class);

    private final Db2InstanceService db2InstanceService = PowerMockito.mock(Db2InstanceService.class);

    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private final Db2InstanceProvider db2InstanceProvider = new Db2InstanceProvider(providerManager,
        db2InstanceService, instanceResourceService);

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_db2_instance_provider_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.DB2_INSTANCE.getType());
        Assert.assertTrue(db2InstanceProvider.applicable(resource));
        resource.setSubType(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        Assert.assertFalse(db2InstanceProvider.applicable(resource));
    }

    /**
     * 用例场景：创建db2实例前检查
     * 前置条件：实例没有被创建
     * 检查点：无异常抛出
     */
    @Test
    public void create_db2_instance_execute_before_create_success() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any()))
            .thenReturn(mockContext());
        ProtectedResource resource = mockResource();
        db2InstanceProvider.beforeCreate(resource);
        Assert.assertEquals("v10.5.0.5", resource.getVersion());
    }

    /**
     * 用例场景：修改db2实例前检查
     * 前置条件：信息正确
     * 检查点：无异常抛出
     */
    @Test
    public void update_db2_instance_execute_before_update_success() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any()))
            .thenReturn(mockContext());
        db2InstanceProvider.beforeUpdate(mockResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：创建db2实例前连通性检查
     * 前置条件：检查结果为空
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_result_is_empty_when_before_create() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        ResourceCheckContext context = mockContext();
        context.setActionResults(new ArrayList<>());
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2InstanceProvider.beforeUpdate(mockResource()));
        Assert.assertEquals("check connection result is empty.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：创建db2实例前连通性检查
     * 前置条件：检查结果为失败
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_result_is_failed_when_before_create() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        ResourceCheckContext context = mockContext();
        context.getActionResults().get(IsmNumberConstant.ZERO).setCode(IsmNumberConstant.TWO);
        context.getActionResults().get(IsmNumberConstant.ZERO).setBodyErr("12345");
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2InstanceProvider.beforeUpdate(mockResource()));
        Assert.assertEquals("check connection failed.", legoCheckedException.getMessage());
        Assert.assertEquals(12345, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：获取db2实例名称重复配置
     * 前置条件：资源信息正确
     * 检查点：返回false
     */
    @Test
    public void get_resource_feature_db2_instance_name_duplicate_configure() {
        ResourceFeature resourceFeature = db2InstanceProvider.getResourceFeature();
        Assert.assertFalse(resourceFeature.isShouldCheckResourceNameDuplicate());
    }

    /**
     * 用例场景：db2单实例健康检查
     * 前置条件：资源信息正确
     * 检查点：检查通过
     */
    @Test
    public void execute_health_check() {
        db2InstanceProvider.healthCheck(mockResource());
        Assert.assertTrue(true);
    }

    private ProtectedResource mockResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setParentUuid(UUID.randomUUID().toString());
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("127.0.0.1");
        resource.setEnvironment(environment);
        return resource;
    }

    private ResourceCheckContext mockContext() {
        Map<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.VERSION, "v10.5.0.5");
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(DatabaseConstants.SUCCESS_CODE);
        actionResult.setMessage(JSONObject.fromObject(map).toString());
        List<ActionResult> list = new ArrayList<>();
        list.add(actionResult);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(list);
        return context;
    }
}