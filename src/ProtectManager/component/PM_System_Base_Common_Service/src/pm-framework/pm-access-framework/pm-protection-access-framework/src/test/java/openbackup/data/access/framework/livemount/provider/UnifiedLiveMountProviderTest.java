package openbackup.data.access.framework.livemount.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;

import openbackup.data.access.client.sdk.api.framework.dee.DeeLiveMountRestApi;
import openbackup.data.access.client.sdk.api.framework.dme.DmeBackupClone;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedService;
import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.livemount.common.enums.OperationEnums;
import openbackup.data.access.framework.livemount.common.model.LiveMountCreateCheckParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountExecuteParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.common.model.LiveMountRefreshParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountUnmountParam;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.provider.CloneCopyParam;
import openbackup.data.access.framework.livemount.provider.DefaultLiveMountServiceProvider;
import openbackup.data.access.framework.livemount.provider.UnifiedLiveMountProvider;
import openbackup.data.access.framework.livemount.service.impl.PerformanceValidator;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.strategies.NativeNfsRepositoryStrategy;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.access.framework.servitization.util.OpServiceHelper;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.anti.ransomware.CopyRansomwareService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.ClustersInfoVo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.StorageDevice;
import openbackup.system.base.sdk.resource.EnvironmentRestApi;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.service.AvailableAgentManagementDomainService;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.SensitiveDataEliminateService;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.jupiter.api.Assertions;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Unified Live Mount Provider Test
 *
 * @author l00272247
 * @since 2022-01-03
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {UnifiedLiveMountProvider.class, ProviderManager.class})
@MockBean( {
        DmeUnifiedRestApi.class, CopyRestApi.class, PerformanceValidator.class, ProtectedEnvironmentService.class,
        ResourceService.class, LiveMountInterceptorProvider.class, RedissonClient.class,
        RepositoryStrategyManager.class, DefaultProtectAgentSelector.class, SensitiveDataEliminateService.class,
        DefaultLiveMountServiceProvider.class})
@PrepareForTest({VerifyUtil.class})
public class UnifiedLiveMountProviderTest {
    @Rule
    public final ExpectedException expectedException = ExpectedException.none();

    @Autowired
    private UnifiedLiveMountProvider unifiedLiveMountProvider;

    @Autowired
    private DmeUnifiedRestApi dmeUnifiedRestApi;

    @MockBean
    private RepositoryStrategyManager repositoryStrategyManager;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private ClusterNativeApi clusterNativeApi;

    @MockBean
    private IStorageDeviceRepository repository;

    @MockBean
    private JobService jobService;

    @MockBean
    private OpServiceHelper opServiceHelper;

    @MockBean
    private UserService userService;

    @MockBean
    private DefaultLiveMountServiceProvider defaultLiveMountServiceProvider;

    @MockBean
    private DefaultProtectAgentSelector defaultProtectAgentSelector;

    @MockBean
    private IVpcService vpcService;

    @MockBean
    private DmeUnifiedService dmeUnifiedService;

    @Autowired
    private LiveMountInterceptorProvider liveMountInterceptorProvider;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    @Qualifier("defaultProtectAgentSelector")
    private ProtectAgentSelector defaultSelector;

    @MockBean
    private DeployTypeService deployTypeService;

    @MockBean
    private EnvironmentRestApi environmentRestApi;

    @MockBean
    private AvailableAgentManagementDomainService domainService;

    @MockBean
    private CommonAgentService commonAgentService;

    @MockBean
    private CopyService copyService;

    @MockBean
    private LiveMountEntityDao liveMountEntityDao;

    @MockBean
    private DeeLiveMountRestApi deeLiveMountRestApi;

    @MockBean
    private CopyRansomwareService copyRansomwareService;

    @Test
    public void test_clone_backup() {
        AtomicReference<DmeBackupClone> reference = new AtomicReference<>();
        PowerMockito.when(dmeUnifiedRestApi.cloneBackup(any())).thenAnswer(invocation -> {
            reference.set(invocation.getArgument(0));
            return "";
        });
        PowerMockito.when(providerManager.findProviderOrDefault(any(),anyString(),any())).thenReturn(defaultLiveMountServiceProvider);
        PowerMockito.when(defaultLiveMountServiceProvider.buildDmeCloneCopyRequest(any())).thenReturn(new DmeBackupClone());
        unifiedLiveMountProvider.cloneBackup(
                new CloneCopyParam("source-backup-id", "clone-backup-id", "resource-type", Collections.emptyList(),
                        null));
        Assertions.assertNotNull(reference.get());
    }

    @Test
    public void test_create_live_mount_pre_check() {
        PowerMockito.when(liveMountInterceptorProvider.applicable(any())).thenReturn(true);
        LiveMountCreateCheckParam param = new LiveMountCreateCheckParam();
        CopyResourceSummary resource = new CopyResourceSummary();
        param.setResource(resource);
        ResourceEntity resourceEntity = new ResourceEntity();
        param.setTargetResources(Collections.singletonList(resourceEntity));
        Copy copy = new Copy();
        param.setCopy(copy);
        LiveMountObject liveMountObject = new LiveMountObject();
        param.setLiveMountObject(liveMountObject);
        unifiedLiveMountProvider.createLiveMountPreCheck(param);
    }

    /**
     * 用例场景：测试即时挂载高级参数的校验
     * 前置条件：封装拦截层方法参数
     * 检查点：能够抛出异常,检验performance
     */
    @Test
    public void test_thrown_lego_checked_exception_for_performance_is_wrong() {
        // Then
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Performance parameters is invalid.");
        PowerMockito.when(liveMountInterceptorProvider.applicable(any())).thenReturn(true);
        LiveMountCreateCheckParam param = new LiveMountCreateCheckParam();
        param.setOperationEnums(OperationEnums.MODIFY);
        LiveMountObject liveMountObject = new LiveMountObject();
        Map<String, Object> parameters = new HashMap<>();
        Map<String, Object> performanceParams = new HashMap<>();
        performanceParams.put("burst_bandwidth", null);
        performanceParams.put("burst_iops", 245);
        performanceParams.put("burst_time", 2333);
        performanceParams.put("latency", 500);
        performanceParams.put("max_bandwidth", null);
        performanceParams.put("max_iops", 124);
        performanceParams.put("min_bandwidth", null);
        performanceParams.put("min_iops", 123);
        parameters.put("performance", performanceParams);
        liveMountObject.setParameters(parameters);
        param.setLiveMountObject(liveMountObject);
        CopyResourceSummary resourceSummary = new CopyResourceSummary();
        resourceSummary.setResourceSubType("NasFileSystem");
        param.setResource(resourceSummary);
        unifiedLiveMountProvider.createLiveMountPreCheck(param);
    }

    /**
     * 用例场景：测试即时挂载執行掛載接口清理敏感信息成功
     * 前置条件：封装拦截层方法参数
     * 检查点：即时挂载執行掛載接口清理敏感信息成功
     */
    @Test
    public void test_execute_live_mount_success() throws URISyntaxException {
        mockBefore();
        NativeNfsRepositoryStrategy strategy = Mockito.mock(NativeNfsRepositoryStrategy.class);
        PowerMockito.when(repositoryStrategyManager.getStrategy(any())).thenReturn(strategy);
        StorageRepository storageRepository = new StorageRepository();
        PowerMockito.when(strategy.getRepository(Mockito.any())).thenReturn(storageRepository);

        PowerMockito.mockStatic(VerifyUtil.class);
        PowerMockito.when(VerifyUtil.isEmpty(anyMap())).thenReturn(false);
        TestLiveMountInterceptorProvider provider = new TestLiveMountInterceptorProvider();
        List<Endpoint> agentList = getAgentList();
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);

        LiveMountExecuteParam liveMountExecuteParam = new LiveMountExecuteParam();
        ProtectedResource protectedResource = new ProtectedResource();
        Authentication authentication = new Authentication();
        authentication.setAuthPwd("authPwd");
        protectedResource.setAuth(authentication);
        liveMountExecuteParam.setTargetResource(protectedResource);
        liveMountExecuteParam.setMountedCopy(mockCopy());
        liveMountExecuteParam.setSourceCopy(mockCopy());
        liveMountExecuteParam.setCloneCopy(mockCopy());
        liveMountExecuteParam.setJobId("job_id");
        liveMountExecuteParam.setLiveMount(mockLiveMountEntity());
        PowerMockito.when(providerManager.findProvider(any(), any(), any())).thenReturn(provider);
        PowerMockito.when(providerManager.findProviderOrDefault(any(), any(), any())).thenReturn(defaultSelector);
        PowerMockito.when(defaultSelector.select(any(), any())).thenReturn(agentList);

        unifiedLiveMountProvider.executeLiveMount(liveMountExecuteParam);
        Assert.assertNotEquals(liveMountExecuteParam.getTargetResource().getAuth().getAuthPwd(), "");
    }


    /**
     * 用例场景：测试即时挂载執行没有agent
     * 前置条件：封装拦截层方法参数
     * 检查点：抛出异常
     */
    @Test
    public void test_execute_live_mount_no_agent() throws URISyntaxException {
        mockBefore();
        TestLiveMountInterceptorProvider provider = new TestLiveMountInterceptorProvider();
        List<Endpoint> agentList = new ArrayList<>();
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);

        LiveMountExecuteParam liveMountExecuteParam = new LiveMountExecuteParam();
        ProtectedResource protectedResource = new ProtectedResource();
        Authentication authentication = new Authentication();
        authentication.setAuthPwd("authPwd");
        protectedResource.setAuth(authentication);
        liveMountExecuteParam.setTargetResource(protectedResource);
        liveMountExecuteParam.setMountedCopy(mockCopy());
        liveMountExecuteParam.setSourceCopy(mockCopy());
        liveMountExecuteParam.setCloneCopy(mockCopy());
        liveMountExecuteParam.setJobId("job_id");
        liveMountExecuteParam.setLiveMount(mockLiveMountEntity());
        PowerMockito.when(providerManager.findProvider(any(), any(), any())).thenReturn(provider);
        PowerMockito.when(providerManager.findProviderOrDefault(any(), any(), any())).thenReturn(defaultSelector);
        PowerMockito.when(defaultSelector.select(any(), any())).thenReturn(agentList);

        Assert.assertThrows(LegoCheckedException.class,
            () -> unifiedLiveMountProvider.executeLiveMount(liveMountExecuteParam));
    }

    /**
     * 用例场景：测试 刷新目标资源
     * 前置条件：mock
     * 检查点：刷新目标资源成功
     */
    @Test
    public void test_refresh_target_resource() throws URISyntaxException {
        // init
        LiveMountRefreshParam liveMountRefreshParam = new LiveMountRefreshParam();
        liveMountRefreshParam.setHasCleanProtection(false);
        liveMountRefreshParam.setLiveMount(mockLiveMountEntity());
        List<String> increasedResourceUuidList = new ArrayList<>();
        increasedResourceUuidList.add("1224353");
        // mock
        mockBefore();
        TestLiveMountInterceptorProvider provider = new TestLiveMountInterceptorProvider();
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.when(resourceService.scanProtectedResource(any())).thenReturn(increasedResourceUuidList);

        // then
        Assert.assertEquals(true,
                !VerifyUtil.isEmpty(unifiedLiveMountProvider.refreshTargetResource(liveMountRefreshParam)));
    }

    /**
     * 用例场景：测试 添加挂载文件系统名称
     * 前置条件：mock
     * 检查点：成功添加挂载文件系统名称
     */
    @Test
    public void test_addLiveMount_file_system_name() throws URISyntaxException {
        // init
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        List<LiveMountFileSystemShareInfo> infos = new ArrayList<>();
        infos.add(new LiveMountFileSystemShareInfo());
        JSONArray array = JSONArray.fromObject(infos);

        liveMountEntity.setFileSystemShareInfo(JSONObject.stringify(array));
        liveMountEntity.setId("1");
        // mock
        mockBefore();
        // then
        LiveMountEntity entity = unifiedLiveMountProvider.addLiveMountFileSystemName(liveMountEntity);
        List<LiveMountFileSystemShareInfo> infoList = JSONArray.fromObject(entity.getFileSystemShareInfo())
                .toBean(LiveMountFileSystemShareInfo.class);
        Assert.assertNotNull(infoList.get(0).getFileSystemName());
    }

    /**
     * 用例场景：验证解挂载成功
     * 前置条件：无
     * 检查点：不报错
     */
    @Test
    public void unmount_live_mount_success() throws Exception {
        // init
        LiveMountUnmountParam param = new LiveMountUnmountParam();
        param.setRequestId("request_id");
        param.setJobId("job_id");
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("1224353");
        param.setMountedCopy(mockCopy());

        LiveMountEntity liveMountEntity =  new LiveMountEntity();
        liveMountEntity.setResourceId("123");
        liveMountEntity.setTargetResourceId("targetResourceId");
        liveMountEntity.setMountJobId("mountJobId");
        param.setLiveMount(liveMountEntity);

        JSONObject jsonObject = new JSONObject();
        jsonObject.put("performance", new HashMap<>());
        jsonObject.put("config", "{\"memory\":{\"use_original\":false,\"memory_size\":1}," +
                "\"power_on\":false,\"startup_network_adaptor\":false," +
                "\"cpu\":{\"use_original\":false,\"num_virtual_sockets\":1,\"num_cores_per_virtual\":1}}");
        jsonObject.put("name", "name");
        liveMountEntity.setParameters(jsonObject.toString());
        List<Endpoint> agentList = getAgentList();
        TestLiveMountInterceptorProvider provider = new TestLiveMountInterceptorProvider();

        // mock
        mockBefore();
        PowerMockito.when(resourceService.getResourceById(anyBoolean(), anyString()))
                .thenReturn(java.util.Optional.of(resource));
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(java.util.Optional.of(resource));
        PowerMockito.when(providerManager.findProviderOrDefault(any(), any(), any())).thenReturn(defaultSelector);
        PowerMockito.when(defaultSelector.select(any(), any())).thenReturn(agentList);
        PowerMockito.mockStatic(VerifyUtil.class);
        PowerMockito.when(VerifyUtil.isEmpty(anyMap())).thenReturn(false);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.doNothing().when(dmeUnifiedRestApi).cancelLiveMountWithUri(any(), any());

        // do
        unifiedLiveMountProvider.unmountLiveMount(param);
        Mockito.verify(resourceService, times(1)).getResourceById(any());
    }

    private List<Endpoint> getAgentList() {
        List<Endpoint> agentList = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setPort(22);
        endpoint.setIp("10.0.0.1");
        agentList.add(endpoint);
        return agentList;
    }

    private void mockBefore() throws URISyntaxException {
        PowerMockito.when(liveMountInterceptorProvider.applicable(any())).thenReturn(true);
        PageListResponse<ProtectedResource> objectPageListResponse = new PageListResponse<>();
        objectPageListResponse.setTotalCount(1);
        ProtectedEnvironment protectedResource1 = new ProtectedEnvironment();
        protectedResource1.setUuid("1224353");
        objectPageListResponse.setRecords(Collections.singletonList(protectedResource1));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(objectPageListResponse);
        PowerMockito.when(repositoryStrategyManager.getStrategy(any()))
                .thenReturn(new NativeNfsRepositoryStrategy(clusterNativeApi, repository));
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        clusterDetailInfo.setStorageSystem(new StorageSystemInfo());
        clusterDetailInfo.setDataProtectionEngines(Collections.EMPTY_LIST);
        SourceClustersParams sourceClustersParams = new SourceClustersParams();
        sourceClustersParams.setMgrIpList(Arrays.asList("8.40.102.105", "8.40.102.106"));
        clusterDetailInfo.setSourceClusters(sourceClustersParams);
        PowerMockito.when(clusterNativeApi.queryDecryptCurrentGroupClusterDetails()).thenReturn(clusterDetailInfo);

        PowerMockito.when(defaultProtectAgentSelector.select(any(), anyMap())).thenReturn(mockAgents());
        ReflectionTestUtils.setField(unifiedLiveMountProvider, "defaultSelector", defaultProtectAgentSelector);
        PowerMockito.when(resourceService.getLanFreeConfig(any(), any())).thenReturn(mockAgentFcConfigMap());

        StorageDevice storageDevice = new StorageDevice();
        storageDevice.setUserName("name");
        storageDevice.setPassword("pwd");
        given(repository.findLocalStorage(true)).willReturn(storageDevice);

        PowerMockito.when(clusterNativeApi.getCurrentClusterVoInfo()).thenReturn(new ClustersInfoVo());
        PowerMockito.when(domainService.getUrlByAgents(any())).thenReturn(new URI("https://test"));
    }

    private Map<String, String> mockAgentFcConfigMap() {
        HashMap map = new HashMap();
        map.put("123", "true");
        return map;
    }

    private List<Endpoint> mockAgents() {
        List<Endpoint> agentList = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setId("123");
        endpoint.setIp("10.0.0.1");
        endpoint.setPort(22);
        agentList.add(endpoint);
        return agentList;
    }

    private Copy mockCopy() {
        Copy copy = new Copy();
        copy.setProperties("{\"backup_id\":\"123\",\"repositories\":[{\"type\":1,\"protocol\":5}]}");
        copy.setUuid("uuid");
        return copy;
    }

    private LiveMountEntity mockLiveMountEntity() {
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setCopyId("copyId");
        return liveMountEntity;
    }

    class TestLiveMountInterceptorProvider implements LiveMountInterceptorProvider{

        @Override
        public boolean applicable(String object) {
            return false;
        }

        @Override
        public boolean isRefreshTargetEnvironment() {
            return true;
        }

        @Override
        public void initialize(LiveMountCreateTask task) {

        }

        @Override
        public void finalize(LiveMountCancelTask task) {

        }
    }
}
