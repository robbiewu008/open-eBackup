package openbackup.kingbase.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.kingbase.protection.access.service.KingBaseService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * {@link KingBaseClusterInstanceProvider} 测试类
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-25
 */
public class KingBaseClusterInstanceProviderTest {
    private static final String ENDPOINT = "8.40.99.176";

    private static final String TEST_PWD = "testPwd";

    private final ProtectedEnvironmentService envService = Mockito.mock(ProtectedEnvironmentService.class);

    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private final KingBaseService kingBaseService = PowerMockito.mock(KingBaseService.class);

    private KingBaseClusterInstanceProvider provider = new KingBaseClusterInstanceProvider(envService,
        instanceResourceService, kingBaseService);

    @Before
    public void init() {
        PowerMockito.when(envService.getEnvironmentById(any())).thenReturn(mockEnv());
    }

    /**
     * 用例场景：kingbase集群实例适配接口
     * 前置条件：输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_kingbase_cluster_instance_provider_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType());
        Assert.assertTrue(provider.applicable(resource));
        resource.setSubType(ResourceSubTypeEnum.MYSQL.getType());
        Assert.assertFalse(provider.applicable(resource));
    }

    /**
     * 用例场景：创建kingbase集群实例前检查
     * 前置条件：参数正常
     * 检查点：无异常抛出
     */
    @Test
    public void execute_check_success() {
        PowerMockito.when(instanceResourceService.checkIsClusterInstance(any())).thenReturn(mockCheckResult());
        ProtectedResource resource = mockResource(Authentication.NO_AUTH);
        provider.check(resource);
        Assert.assertEquals(ENDPOINT, resource.getPath());
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(),
            resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
    }

    /**
     * 用例场景：修改kingbase集群实例前检查
     * 前置条件：参数正常
     * 检查点：无异常抛出
     */
    @Test
    public void execute_before_update_success() {
        PowerMockito.when(instanceResourceService.checkIsClusterInstance(any())).thenReturn(mockCheckResult());
        PowerMockito.when(kingBaseService.getResourceById(any()))
            .thenReturn(mockInstance(Authentication.NO_AUTH, null));
        ProtectedResource resource = mockResource(Authentication.NO_AUTH);
        provider.beforeUpdate(resource);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(),
            resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
    }

    /**
     * 用例场景：修改kingbase集群实例
     * 前置条件：参数里面没有密码
     * 检查点：是否从数据库回填密码信息
     */
    @Test
    public void should_set_pwd_value_if_pwd_is_empty_when_before_update() {
        PowerMockito.when(instanceResourceService.checkIsClusterInstance(any())).thenReturn(mockCheckResult());
        ProtectedResource resource = mockResource(Authentication.APP_PASSWORD);
        PowerMockito.when(kingBaseService.getResourceById(any()))
            .thenReturn(mockInstance(Authentication.APP_PASSWORD, TEST_PWD));
        provider.beforeUpdate(resource);
        Assert.assertEquals(TEST_PWD,
            resource.getDependencies().get(DatabaseConstants.CHILDREN).get(0).getAuth().getAuthPwd());
    }

    private ProtectedResource mockInstance(int authType, String pwd) {
        Authentication auth = new Authentication();
        auth.setAuthType(authType);
        auth.setAuthKey("testUser");
        auth.setAuthPwd(pwd);
        ProtectedResource instance = new ProtectedResource();
        instance.setAuth(auth);
        return instance;
    }

    private ProtectedResource mockResource(int authType) {
        ProtectedResource resource = new ProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "54321");
        resource.setExtendInfo(extendInfo);
        resource.setEnvironment(mockEnv());
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, Collections.singletonList(mockInstance(authType, null)));
        resource.setDependencies(dependencies);
        return resource;
    }

    private ProtectedEnvironment mockEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint(ENDPOINT);
        return environment;
    }

    private AgentBaseDto mockCheckResult() {
        Map<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.VERSION, "V008R006C005B0054");
        AgentBaseDto checkResult = new AgentBaseDto();
        checkResult.setErrorMessage(JSONObject.fromObject(map).toString());
        return checkResult;
    }
}