package openbackup.tpops.protection.access.interceptor;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.common.constants.LocalStorageInfoRes;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;
import openbackup.tpops.protection.access.provider.TpopsGaussDBAgentProvider;
import openbackup.tpops.protection.access.service.impl.TpopsGaussDBServiceImpl;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

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
@PrepareForTest(value = {EnvironmentLinkStatusHelper.class})
public class TpopsGaussDBBackupInterceptorTest {
    private TpopsGaussDBBackupInterceptor gaussDBBackupInterceptor;

    private ProtectedResourceChecker protectedResourceChecker;

    private ResourceService resourceService;

    private LocalStorageService localStorageService;

    private UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    private BackupStorageApi backupStorageApi;

    private TpopsGaussDBServiceImpl gaussDbService;

    private ProviderManager providerManager;

    private ClusterNativeApi clusterNativeApi;

    private EncryptorService encryptorService;

    private ClusterBasicService clusterBasicService;

    private DeployTypeService deployTypeService;

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = Mockito.mock(
        ResourceConnectionCheckProvider.class);

    private final TaskRepositoryManager taskRepositoryManager = Mockito.mock(TaskRepositoryManager.class);

    @Before
    public void init() throws IllegalAccessException {
        protectedResourceChecker = Mockito.mock(ProtectedResourceChecker.class);
        resourceService = Mockito.mock(ResourceService.class);
        localStorageService = Mockito.mock(LocalStorageService.class);
        backupStorageApi = Mockito.mock(BackupStorageApi.class);
        providerManager = Mockito.mock(ProviderManager.class);
        clusterNativeApi = Mockito.mock(ClusterNativeApi.class);
        encryptorService = Mockito.mock(EncryptorService.class);
        clusterIntegrityChecker = Mockito.mock(UnifiedClusterResourceIntegrityChecker.class);
        clusterBasicService = Mockito.mock(ClusterBasicService.class);
        deployTypeService = Mockito.mock(DeployTypeService.class);
        gaussDbService = new TpopsGaussDBServiceImpl(resourceService, providerManager, resourceConnectionCheckProvider,
            clusterIntegrityChecker, taskRepositoryManager);
        gaussDBBackupInterceptor = new TpopsGaussDBBackupInterceptor(gaussDbService,
            new TpopsGaussDBAgentProvider(null, resourceService), clusterBasicService, deployTypeService);

        LocalStorageInfoRes localStorageInfoRes = new LocalStorageInfoRes();
        localStorageInfoRes.setEsn("xxxxxxxxxxxxxxxxx");
        PowerMockito.when(localStorageService.getStorageInfo()).thenReturn(localStorageInfoRes);
        MemberModifier.field(TpopsGaussDBServiceImpl.class, "taskRepositoryManager")
            .set(gaussDbService, taskRepositoryManager);
    }

    /**
     * 用例场景：GaussDB 下发备份任务 applicable 校验
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(gaussDBBackupInterceptor.applicable(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType()));
    }

    /**
     * 用例场景：GaussDB集群 更新task信息无异常
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void supplyAgent() {
        BackupTask backupTask = MockInterceptorParameter.getBackupTask();
        ProtectedResource resource = MockInterceptorParameter.getProtectedEnvironment();

        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, MockInterceptorParameter.getProtectedEnvironment2());
        resource.setDependencies(dependencies);

        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(resource));

        gaussDBBackupInterceptor.supplyAgent(backupTask);

        List<Endpoint> agents = MockInterceptorParameter.getEndpointFromProtectedEnvironment2();

        assertThat(backupTask.getAgents()).usingRecursiveComparison().isEqualTo(agents);
    }

    /**
     * 用例场景：GaussDB集群 更新task信息无异常
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void supplyNodes() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn("1");
        BackupTask backupTask = MockInterceptorParameter.getBackupTask();
        Optional<ProtectedResource> resourceOptional = Optional.of(MockInterceptorParameter.getProtectedEnvironment());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(resourceOptional);
        gaussDBBackupInterceptor.supplyNodes(backupTask);
        String type = backupTask.getAdvanceParams().get(TpopsGaussDBConstant.IS_CHECK_BACKUP_JOB_TYPE);
        Assert.assertEquals(type, "true");
    }

    /**
     * 用例场景：GaussDB集群 更新task信息无异常
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void supply_backup_task_success() {
        BackupTask backupTask = MockInterceptorParameter.getBackupTask();
        backupTask.setAdvanceParams(new HashMap<>());
        backupTask.getAdvanceParams().put(TpopsGaussDBConstant.ADVANCE_PARAMS_KEY_STORAGE_ID, "123456");
        backupTask.getAdvanceParams().put(TpopsGaussDBConstant.ADVANCE_PARAMS_KEY_BACKUP_METADATA_PATH, "/opt/share");
        backupTask.getAdvanceParams().put(TpopsGaussDBConstant.ADVANCE_PARAMS_KEY_BACKUP_TOOL_TYPE, "1");
        PowerMockito.when(backupStorageApi.getDetail("123456"))
            .thenReturn(MockInterceptorParameter.getNasDistributionStorageDetail());
        Optional<ProtectedResource> resourceOptional = Optional.of(MockInterceptorParameter.getProtectedEnvironment());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(resourceOptional);
        gaussDBBackupInterceptor.supplyBackupTask(backupTask);
    }
}