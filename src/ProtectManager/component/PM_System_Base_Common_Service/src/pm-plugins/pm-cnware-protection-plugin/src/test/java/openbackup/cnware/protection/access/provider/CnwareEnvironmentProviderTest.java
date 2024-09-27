package openbackup.cnware.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;

import openbackup.cnware.protection.access.service.CnwareCommonService;
import openbackup.cnware.protection.access.mock.CnwareMockUtil;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;
import java.util.Optional;

/**
 * CnwareEnvironmentProvider测试类
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-12-19
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(TokenBo.class)
@Slf4j
public class CnwareEnvironmentProviderTest {
    @Mock
    private CnwareCommonService mockCnwareCommonService;
    @Mock
    private ResourceService mockResourceService;
    @Mock
    private AgentUnifiedService mockAgentUnifiedService;
    @Mock
    private ProtectedEnvironmentRetrievalsService mockEnvRetrievalsService;
    @Mock
    private AgentUnifiedService mockAgentService;
    private CnwareEnvironmentProvider cnwareEnvironmentProviderTest;

    @Before
    public void setUp() {
        cnwareEnvironmentProviderTest = new CnwareEnvironmentProvider(mockCnwareCommonService, mockResourceService,
            mockAgentUnifiedService, mockEnvRetrievalsService, mockAgentService);
    }

    /**
     * 用例场景：CNware环境检查类过滤
     * 前置条件：无
     * 检查点：类型为CNware时过滤成功
     */
    @Test
    public void test_application() {
        Assert.assertTrue(cnwareEnvironmentProviderTest.applicable(ResourceSubTypeEnum.CNWARE.getType()));
        Assert.assertFalse(cnwareEnvironmentProviderTest.applicable("object"));
    }

    /**
     * 用例场景：检查环境上的CNware资源是否已达上限
     * 前置条件：环境中已注册最大数量的CNware资源
     * 检查点：检查环境上的CNware资源是否已达上限，当已达上限时注册新的CNware资源则抛出异常
     */
    @Test
    public void test_checkCnwareCount_should_throw_exception_when_cnware_count_exceeds_maximum()
        throws NoSuchMethodException{
        ProtectedEnvironment environment = new ProtectedEnvironment();
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(8);
        PowerMockito.when(mockResourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);
        Class<CnwareEnvironmentProvider> providerClass = CnwareEnvironmentProvider.class;
        Method privateMethod = providerClass.getDeclaredMethod("checkCnwareCount", ProtectedEnvironment.class);
        privateMethod.setAccessible(true);
        Assert.assertThrows(InvocationTargetException.class,
            () -> privateMethod.invoke(cnwareEnvironmentProviderTest, environment));
    }

    /**
     * 用例场景：检查环境信息
     * 前置条件：环境uuid值为空
     * 检查点：检查CNware环境信息，若uuid为空则生成uuid
     */
    @Test
    public void test_checkEnvironment_should_pass_when_env_uuid_is_blank()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        PowerMockito.when(mockResourceService.query(any())).thenReturn(null);
        Class<CnwareEnvironmentProvider> providerClass = CnwareEnvironmentProvider.class;
        Method privateMethod = providerClass.getDeclaredMethod("checkEnvironment", ProtectedEnvironment.class);
        privateMethod.setAccessible(true);
        privateMethod.invoke(cnwareEnvironmentProviderTest, environment);
        Assert.assertNotNull(environment.getUuid());
    }

    /**
     * 用例场景：检查环境信息是否重复
     * 前置条件：环境uuid值为空
     * 检查点：检查CNware环境信息是否重复，若uuid为空则抛出异常
     */
    @Test
    public void test_checkEnvironmentRepeat_should_throw_exception_when_env_uuid_is_blank()
        throws NoSuchMethodException{
        ProtectedEnvironment environment = new ProtectedEnvironment();
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(1);
        PowerMockito.when(mockResourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);
        Class<CnwareEnvironmentProvider> providerClass = CnwareEnvironmentProvider.class;
        Method privateMethod = providerClass.getDeclaredMethod("checkEnvironmentRepeat", ProtectedEnvironment.class);
        privateMethod.setAccessible(true);
        Assert.assertThrows(InvocationTargetException.class,
            () -> privateMethod.invoke(cnwareEnvironmentProviderTest, environment));
    }

    /**
     * 用例场景：检查环境ip和port信息
     * 前置条件：环境uuid值为空
     * 检查点：检查CNware环境信息，若port为空则应抛出异常
     */
    @Test
    public void test_checkIpAndPort_should_throw_exception_when_port_is_blank()
        throws NoSuchMethodException{
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("www.cnware.com");
        environment.setPort(65536);
        Class<CnwareEnvironmentProvider> providerClass = CnwareEnvironmentProvider.class;
        Method privateMethod = providerClass.getDeclaredMethod("checkEndpointAndPort", ProtectedEnvironment.class);
        privateMethod.setAccessible(true);
        Assert.assertThrows(InvocationTargetException.class,
            () -> privateMethod.invoke(cnwareEnvironmentProviderTest, environment));
    }

    /**
     * 用例场景：检查check正常情况下是否会更新在线状态
     * 前置条件：无
     * 检查点：环境信息中更新在线状态
     */
    @Test
    public void test_check_should_update_link_status() {
        ProtectedEnvironment environment = CnwareMockUtil.mockEnvironment();
        PageListResponse<ProtectedResource> registeredEnv = new PageListResponse<>();
        registeredEnv.setTotalCount(0);
        PowerMockito.when(mockResourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(registeredEnv);
        CnwareMockUtil.mockTokenBo(UserTypeEnum.COMMON.getValue());
        PowerMockito.when(mockCnwareCommonService.queryClusterInfo(any(), any()))
            .thenReturn(CnwareMockUtil.mockAppEnvResponse());
        cnwareEnvironmentProviderTest.register(environment);
        Assert.assertEquals(environment.getLinkStatus(), LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 用例场景：资源扫描场景
     * 前置条件：agent信息扫描失败
     * 检查点：资源扫描时，若无法获取agent环境信息，则抛出异常
     */
    @Test
    public void test_scan_should_throw_exception_when_scan_by_agent_failed() {
        Assert.assertThrows(LegoCheckedException.class,
            () -> cnwareEnvironmentProviderTest.scan(CnwareMockUtil.mockScanEnvironment()));
    }

    /**
     * 用例场景：资源扫描场景
     * 前置条件：根据uuid查询agent信息失败
     * 检查点：资源扫描时，根据uuid查询agent信息失败，则抛出异常
     */
    @Test
    public void test_scan_should_throw_exception_when_get_agent_resource_by_id_failed() {
        PowerMockito.when(mockEnvRetrievalsService.collectConnectableResources(anyString())).thenReturn(
            CnwareMockUtil.mockCollectConnectableResources());
        Assert.assertThrows(LegoCheckedException.class,
            () -> cnwareEnvironmentProviderTest.scan(CnwareMockUtil.mockScanEnvironment()));
    }

    /**
     * 用例场景：资源扫描场景
     * 前置条件：根据uuid查询agent信息失败
     * 检查点：资源扫描时，成功扫描出CNware资源
     */
    @Test
    public void test_scan_should_throw_exception_when_get_agent_info_failed() {
        PowerMockito.when(mockEnvRetrievalsService.collectConnectableResources(anyString())).thenReturn(
            CnwareMockUtil.mockCollectConnectableResources());
        Optional mock = PowerMockito.mock(Optional.class);
        PowerMockito.when(mock.isPresent()).thenReturn(true);
        PowerMockito.when(mockResourceService.getResourceById(anyString()))
            .thenReturn(Optional.of(CnwareMockUtil.mockScanEnvironment()));
        PowerMockito.when(mockAgentService.getDetailPageList(any(), any(), any(), any()))
            .thenReturn(CnwareMockUtil.mockResponseCnwareHostPool());
        ProtectedEnvironment protectedEnvironment = CnwareMockUtil.mockScanEnvironment();
        List<ProtectedResource>  protectedResourceList = cnwareEnvironmentProviderTest.scan(protectedEnvironment);
        Assert.assertNotNull(protectedResourceList);
    }
}