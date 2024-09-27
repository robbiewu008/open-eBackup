package openbackup.tpops.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;

import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.access.framework.resource.validator.JsonSchemaValidatorImpl;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.common.constants.LocalStorageInfoRes;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;
import openbackup.tpops.protection.access.service.impl.TpopsGaussDBServiceImpl;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author x30021699
 * @since 2023-05-09
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {TpopsGaussDBClusterEnvironmentProvider.class, JsonSchemaValidator.class})
public class TpopsGaussDBClusterEnvironmentProviderTest {
    private TpopsGaussDBClusterEnvironmentProvider tpopsGaussDBClusterEnvironmentProvider;

    private ProviderManager providerManager;

    private ResourceService resourceService;

    private LocalStorageService localStorageService;

    private UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = Mockito.mock(
        ResourceConnectionCheckProvider.class);

    private final ResourceConnectionCheckProvider provider = Mockito.mock(ResourceConnectionCheckProvider.class);

    private TpopsGaussDBServiceImpl gaussDbService;

    @Rule
    private ExpectedException expectedException = ExpectedException.none();

    private TaskRepositoryManager taskRepositoryManager;

    private AgentUnifiedService agentUnifiedService;

    private JsonSchemaValidator jsonSchemaValidator;

    @Before
    public void init() throws IllegalAccessException {
        providerManager = Mockito.mock(ProviderManager.class);
        resourceService = Mockito.mock(ResourceService.class);
        agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
        clusterIntegrityChecker = Mockito.mock(UnifiedClusterResourceIntegrityChecker.class);
        localStorageService = Mockito.mock(LocalStorageService.class);
        PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);
        taskRepositoryManager = Mockito.mock(TaskRepositoryManager.class);
        tpopsGaussDBClusterEnvironmentProvider = new TpopsGaussDBClusterEnvironmentProvider(providerManager,
            pluginConfigManager, agentUnifiedService);
        gaussDbService = new TpopsGaussDBServiceImpl(resourceService, providerManager, resourceConnectionCheckProvider,
            clusterIntegrityChecker, taskRepositoryManager);
        MemberModifier.field(TpopsGaussDBClusterEnvironmentProvider.class, "tpopsGaussDbService")
            .set(tpopsGaussDBClusterEnvironmentProvider, gaussDbService);
        MemberModifier.field(TpopsGaussDBClusterEnvironmentProvider.class, "localStorageService")
            .set(tpopsGaussDBClusterEnvironmentProvider, localStorageService);
        MemberModifier.field(TpopsGaussDBClusterEnvironmentProvider.class, "tpopsGaussDBAgentProvider")
            .set(tpopsGaussDBClusterEnvironmentProvider, new TpopsGaussDBAgentProvider(null, resourceService));
        LocalStorageInfoRes localStorageInfoRes = new LocalStorageInfoRes();
        localStorageInfoRes.setEsn("xxxxxxxxxxxxxxxxx");
        PowerMockito.when(localStorageService.getStorageInfo()).thenReturn(localStorageInfoRes);
        PowerMockito.when(providerManager.findProviderOrDefault(any(), any(), any())).thenReturn(provider);
    }

    /**
     * 用例场景：GaussDB 集群环境 联通性provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable() {
        Assert.assertTrue(
            tpopsGaussDBClusterEnvironmentProvider.applicable(ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.getType()));
    }

    /**
     * 用例场景：环境已有资源校验，联通检查
     * 前置条件：无
     * 检查点：设置正确，返回成功
     *
     * @throws IllegalAccessException 参数错误
     */
    @Test
    public void check() throws IllegalAccessException {
        checkSuccess();
        jsonSchemaValidator = new JsonSchemaValidatorImpl();
        MemberModifier.field(TpopsGaussDBClusterEnvironmentProvider.class, "jsonSchemaValidator")
            .set(tpopsGaussDBClusterEnvironmentProvider, jsonSchemaValidator);
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        tpopsGaussDBClusterEnvironmentProvider.register(protectedEnvironment);
    }

    /**
     * 用例场景：环境已有资源校验，联通检查。项目id重复报错
     * 前置条件：无
     * 检查点：设置正确，返回失败
     *
     * @throws IllegalAccessException 参数错误
     */
    @Test
    public void checkFail() throws IllegalAccessException {
        jsonSchemaValidator = new JsonSchemaValidatorImpl();
        MemberModifier.field(TpopsGaussDBClusterEnvironmentProvider.class, "jsonSchemaValidator")
            .set(tpopsGaussDBClusterEnvironmentProvider, jsonSchemaValidator);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("55555");
        protectedEnvironment.setName("qqqqqq");
        protectedEnvironment.setVersion("3.2.0");
        protectedEnvironment.setExtendInfoByKey("version", "3.2.0");
        protectedEnvironment.setExtendInfoByKey("status", "status");
        protectedEnvironment.setExtendInfoByKey("projectId", "projectId");
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("22222");
        resources.add(protectedResource);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        resourceMap.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, resources);
        protectedEnvironment.setDependencies(resourceMap);
        List<ProtectedResource> list = new ArrayList<>();
        list.add(protectedEnvironment);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(1);
        pageListResponse.setRecords(list);

        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedEnvironment));
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(pageListResponse);
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(any(), any(), any(), any(), eq(false)))
            .thenReturn(pageListResponse);

        protectedEnvironment.setUuid("");
        Assert.assertThrows("register is duplicate", LegoCheckedException.class,
            () -> tpopsGaussDBClusterEnvironmentProvider.register(protectedEnvironment));
    }

    /**
     * 用例场景：健康检查，在线校验
     * 前置条件：无
     * 检查点: 返回状态，在线状态
     *
     * @throws IllegalAccessException IllegalAccessException
     */
    @Test
    public void testHealthCheckSuccess() throws IllegalAccessException {
        checkSuccess();
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        Optional<String> response = tpopsGaussDBClusterEnvironmentProvider.healthCheckWithResultStatus(
            protectedEnvironment);
        Assert.assertEquals("1", response.get());
    }

    /**
     * 用例场景：健康检查，在线校验
     * 前置条件：无
     * 检查点: 返回状态，离线状态
     *
     * @throws IllegalAccessException IllegalAccessException
     */
    @Test
    public void testHealthCheckFail() throws IllegalAccessException {
        checkSuccess();
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(any(), any(), any(), any(), eq(false)))
            .thenThrow(new LegoCheckedException("database operation upsert resources fail"));
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        Optional<String> response = tpopsGaussDBClusterEnvironmentProvider.healthCheckWithResultStatus(
            protectedEnvironment);
        Assert.assertEquals("0", response.get());
    }

    /**
     * 用例场景：扫描资源信息，状态正常
     * 前置条件：有信息
     * 检查点: 返回信息，状态正常
     */
    @Test
    public void testScan() {
        buildQueryResponse("ACTIVE");
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        List<ProtectedResource> resources = tpopsGaussDBClusterEnvironmentProvider.scan(protectedEnvironment);
        Assert.assertEquals("1", resources.get(0).getExtendInfo().get("status"));
    }

    /**
     * 用例场景：扫描资源信息，观察是否进入更新状态
     * 前置条件：有信息
     * 检查点: 返回信息，查看日志信息，是否更新
     */
    @Test
    public void testScanWhenUpdateResources() {
        buildQueryResponse("ACTIVE");
        List<ProtectedResource> sameResources = new ArrayList<>();
        ProtectedResource sameProtectedResource = new ProtectedResource();
        sameProtectedResource.setUuid("2122222");
        sameResources.add(sameProtectedResource);

        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        protectedEnvironment.getDependencies().get("agents").addAll(sameResources);

        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        List<ProtectedResource> resources = tpopsGaussDBClusterEnvironmentProvider.scan(protectedEnvironment);
        Assert.assertEquals("1", resources.get(0).getExtendInfo().get("status"));
    }

    /**
     * 用例场景：扫描资源信息，状态为不可用
     * 前置条件：有信息
     * 检查点: 返回信息，转态不可用
     */
    @Test
    public void testScanUnvilalble() {
        buildQueryResponse("Unvilalble");
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        List<ProtectedResource> resources = tpopsGaussDBClusterEnvironmentProvider.scan(protectedEnvironment);
        Assert.assertEquals("9", resources.get(0).getExtendInfo().get("status"));
    }

    private void checkSuccess() {
        buildQueryResponse("ACTIVE");
        PowerMockito.spy(resourceService).delete((String[]) any());
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("11111111");
        Optional<ProtectedResource> resourceOptional = Optional.of(resource);
        PowerMockito.when(resourceService.getResourceById("11111111")).thenReturn(resourceOptional);
        CheckResult<AppEnvResponse> appEnvResponseCheckResult = new CheckResult<>();
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setSubType(ResourceSubTypeEnum.GAUSSDB.getType());
        List<NodeInfo> nodes = new ArrayList<>();
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setExtendInfo(new HashMap<>());
        nodeInfo.getExtendInfo().put(TpopsGaussDBConstant.EXTEND_INFO_KEY_VERSION, null);
        nodeInfo.getExtendInfo().put(TpopsGaussDBConstant.EXTEND_INFO_KEY_STATE, "ONLINE");
        nodes.add(nodeInfo);
        appEnvResponse.setNodes(nodes);
        appEnvResponse.setExtendInfo(new HashMap<String, String>() {
            {
                put(TpopsGaussDBConstant.EXTEND_INFO_KEY_VERSION, "3.1");
            }
        });
        appEnvResponseCheckResult.setData(appEnvResponse);
        PowerMockito.when(clusterIntegrityChecker.generateCheckResult(any())).thenReturn(appEnvResponseCheckResult);
    }

    private ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("666666");
        protectedEnvironment.setName("register");
        protectedEnvironment.setType("dataBase");

        protectedEnvironment.setSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.getType());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(TpopsGaussDBConstant.EXTEND_INFO_KEY_PROJECT_NAME, "PROJECT_NAME");
        extendInfo.put(TpopsGaussDBConstant.EXTEND_INFO_KEY_ACCOUNT_NAME, "ACCOUNT_NAME");
        extendInfo.put(TpopsGaussDBConstant.EXTEND_INFO_KEY_PM_ADDRESS, "https://1.1.1.1:1");
        extendInfo.put("businessAddr", "https://1.1.1.1");
        extendInfo.put("projectId", "projectId");

        protectedEnvironment.setExtendInfo(extendInfo);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("11111111");
        protectedResource.setExtendInfoByKey("version", "3.0.0");
        resources.add(protectedResource);
        resourceMap.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, resources);
        protectedEnvironment.setDependencies(resourceMap);

        Authentication authentication = new Authentication();
        authentication.setAuthKey("omm");
        authentication.setAuthPwd("TEST");
        protectedEnvironment.setAuth(authentication);
        return protectedEnvironment;
    }

    private void buildQueryResponse(String status) {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("55555");
        protectedEnvironment.setName("qqqqqq");
        protectedEnvironment.setVersion("3.2.0");
        protectedEnvironment.setExtendInfoByKey("version", "3.2.0");
        protectedEnvironment.setExtendInfoByKey("status", status);
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("22222");
        resources.add(protectedResource);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        resourceMap.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, resources);
        protectedEnvironment.setDependencies(resourceMap);
        List<ProtectedResource> list = new ArrayList<>();
        list.add(protectedEnvironment);
        pageListResponse.setTotalCount(1);
        pageListResponse.setRecords(list);

        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedEnvironment));
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(pageListResponse);
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(any(), any(), any(), any(), eq(false)))
            .thenReturn(pageListResponse);
    }
}