package openbackup.tpops.protection.access.interceptor;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;
import openbackup.tpops.protection.access.provider.TpopsGaussDBAgentProvider;
import openbackup.tpops.protection.access.service.TpopsGaussDBService;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * GaussDb 恢复任务基础数据Provider测试类
 *
 * @author x30021699
 * @since 2023-05-09
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {EnvironmentLinkStatusHelper.class})
public class TpopsGaussDBRestoreInterceptorTest {
    private final ProtectedEnvironmentService environmentService = Mockito.mock(ProtectedEnvironmentService.class);

    private final TpopsGaussDBService tpopsGaussDbService = Mockito.mock(TpopsGaussDBService.class);

    private final ProtectedEnvironmentRetrievalsService protectedResourceChecker = Mockito.mock(
        ProtectedEnvironmentRetrievalsService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final ClusterNativeApi clusterNativeApi = PowerMockito.mock(ClusterNativeApi.class);

    private final EncryptorService encryptorService = PowerMockito.mock(EncryptorService.class);

    private TpopsGaussDBRestoreInterceptor restoreInterceptor;

    private ResourceService resourceService;

    private JobCenterRestApi jobCenterRestApi;

    @Before
    public void initTest() {
        resourceService = Mockito.mock(ResourceService.class);
        TpopsGaussDBAgentProvider gaussDBAgentProvider = new TpopsGaussDBAgentProvider(protectedResourceChecker,
            resourceService);
        restoreInterceptor = new TpopsGaussDBRestoreInterceptor(environmentService, tpopsGaussDbService, copyRestApi,
            clusterNativeApi, gaussDBAgentProvider, resourceService, jobCenterRestApi);
        restoreInterceptor.setEncryptorService(encryptorService);
    }

    /**
     * 用例场景：GaussDb恢复参数处理
     * 前置条件：1.没有agent信息
     * 检  查  点：返回备份类型是否为快照备份
     */
    @Test
    public void supply_restore_task_not_agents_success() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.isOnlineAdaptMultiCluster(any(ProtectedEnvironment.class)))
            .thenReturn(true);
        Mockito.when(environmentService.getEnvironmentById("env")).thenReturn(initProtectedEnvironment());
        Mockito.when(tpopsGaussDbService.supplyNodes(Mockito.anyString())).thenReturn(new ArrayList<>());
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("uuid");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8088);
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        Mockito.when(tpopsGaussDbService.getEnvironmentById(Mockito.anyString())).thenReturn(protectedEnvironment);
        List<ProtectedEnvironment> protectedEnvironments = Lists.newArrayList(protectedEnvironment);
        ProtectedResource protectedResource = initProtectedResource();
        protectedResourceMap.put(protectedResource, protectedEnvironments);
        Mockito.when(protectedResourceChecker.collectConnectableResources(Mockito.anyString()))
            .thenReturn(protectedResourceMap);
        RestoreTask task = initRestoreTask();
        RestoreTask restoreTask = restoreInterceptor.supplyRestoreTask(task);
        Assert.assertEquals("env", restoreTask.getTargetEnv().getUuid());
        Endpoint endpoint = new Endpoint("uuid", "127.0.0.1", 8088);
        assertThat(task.getAgents()).usingRecursiveComparison().isEqualTo(Lists.newArrayList(endpoint));
    }

    /**
     * 用例场景：回填副本恢复类型到恢复任务中成功
     * 前置条件：副本正常，下发任务正常
     * 检查点：回填成功-LocalRestore
     *
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void supply_task_restore_mode_with_LocalRestore_type_success() throws Exception {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setCopyId("test_copy");
        Whitebox.invokeMethod(restoreInterceptor, "supplyRestoreMode", restoreTask);
        Assert.assertEquals(RestoreModeEnum.LOCAL_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：回填副本恢复类型到恢复任务中成功
     * 前置条件：副本正常，下发任务正常
     * 检查点：回填成功-DownloadRestore
     *
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void supply_task_restore_mode_with_DownloadRestore_type_success() throws Exception {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setCopyId("test_copy");
        Whitebox.invokeMethod(restoreInterceptor, "supplyRestoreMode", restoreTask);
        Assert.assertEquals(RestoreModeEnum.DOWNLOAD_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：回填副本恢复类型到恢复任务中成功
     * 前置条件：副本正常，下发任务正常
     * 检查点：回填成功-RemoteRestore
     *
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void supply_task_restore_mode_with_RemoteRestore_type_success() throws Exception {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setCopyId("test_copy");
        Whitebox.invokeMethod(restoreInterceptor, "supplyRestoreMode", restoreTask);
        Assert.assertEquals(RestoreModeEnum.REMOTE_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：GaussDb 新位置恢复参数下发
     * 前置条件：1.新位置恢复 2.副本正常
     * 检查点：下发新位置高级参数成功
     *
     * @throws Exception 异常捕获
     */
    @Test
    public void supply_restore_task_advance_params_success() throws Exception {
        RestoreTask restoreTask = initRestoreTask();
        restoreTask.setTargetLocation(RestoreLocationEnum.NEW);
        restoreTask.setCopyId("2222");
        restoreTask.setAdvanceParams(new HashMap<>());
        Mockito.when(copyRestApi.queryCopyByID(restoreTask.getCopyId())).thenReturn(new Copy());
        Whitebox.invokeMethod(restoreInterceptor, "supplyAdvancedParams", restoreTask);
        Assert.assertEquals(RestoreLocationEnum.NEW.getLocation(),
            restoreTask.getAdvanceParams().get(TpopsGaussDBConstant.EXTEND_INFO_KEY_TARGET_LOCATION));
    }

    /**
     * 用例场景：GaussDb 恢复参数下发
     * 前置条件：1.副本正常
     * 检查点：下发认证参数成功
     *
     * @throws Exception 异常捕获
     */
    @Test
    public void supply_restore_task_auth_success() throws Exception {
        RestoreTask restoreTask = initRestoreTask();
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setId("22222222");
        restoreTask.setRepositories(Lists.newArrayList(storageRepository));
        ClusterDetailInfo localCluster = new ClusterDetailInfo();
        SourceClustersParams sourceClustersParams = new SourceClustersParams();
        sourceClustersParams.setMgrIpList(Lists.newArrayList("127.0.0.1"));
        localCluster.setSourceClusters(sourceClustersParams);
        StorageSystemInfo storageSystemInfo = new StorageSystemInfo();
        storageSystemInfo.setStorageEsn("22222222");
        storageRepository.setProtocol(100);
        localCluster.setStorageSystem(storageSystemInfo);
        Mockito.when(clusterNativeApi.queryCurrentGroupClusterDetails()).thenReturn(localCluster);
        Whitebox.invokeMethod(restoreInterceptor, "supplyAuth", restoreTask);
        Assert.assertEquals("127.0.0.1", restoreTask.getRepositories().get(0).getEndpoint().getIp());
    }

    /**
     * 用例场景：GaussDb恢复参数处理
     * 前置条件：1.类型判断
     * 检  查  点：是否匹配
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(restoreInterceptor.applicable(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType()));
    }

    private RestoreTask initRestoreTask() {
        RestoreTask restoreTask = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("env");
        restoreTask.setCopyId("copy_id");
        restoreTask.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        restoreTask.setTargetObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("env");
        taskEnvironment.setExtendInfo(new HashMap<>());
        restoreTask.setTargetEnv(taskEnvironment);
        restoreTask.setAdvanceParams(new HashMap<>());
        restoreTask.getAdvanceParams()
            .put(TpopsGaussDBConstant.EXTEND_INFO_KEY_SUB_TYPE, ResourceSubTypeEnum.GAUSSDB.getType());
        restoreTask.setRepositories(new ArrayList<>());
        return restoreTask;
    }

    private ProtectedEnvironment initProtectedEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Authentication authentication = new Authentication();
        authentication.setAuthKey("auth");
        environment.setSubType(ResourceSubTypeEnum.GAUSSDB.getType());
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        environment.setRootUuid("env");
        environment.setAuth(authentication);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> protectedResources = new ArrayList<>();
        protectedResources.add(initProtectedResource());
        dependencies.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, protectedResources);
        environment.setDependencies(dependencies);
        environment.setExtendInfo(new HashMap<>());
        return environment;
    }

    private ProtectedResource initProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("1");
        return protectedResource;
    }
}
