package openbackup.cnware.protection.access.provider;

import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.cnware.protection.access.service.CnwareCommonService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.cluster.model.ClustersInfoVo;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述 CnwareBackupProviderTest
 *
 * @author q30048244
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-05 9:39
 */
public class CnwareBackupProviderTest {
    private static CnwareBackupProvider backupProvider;

    private static final CnwareCommonService cnwareCommonService = Mockito.mock(CnwareCommonService.class);

    private static final ResourceService resourceService = Mockito.mock(ResourceService.class);

    @BeforeClass
    public static void init() {
        backupProvider = new CnwareBackupProvider(resourceService, cnwareCommonService);
    }

    /**
     * 用例场景：Cnware备份插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        boolean cNwareVm = backupProvider.applicable("CNwareVm");
        Assert.assertTrue(cNwareVm);
    }

    /**
     * 用例场景：Cnware备份对象参数转换 <br/>
     * 前置条件：Cnware备份对象参数正确 <br/>
     * 检查点：无异常，参数补充正确
     */
    @Test
    public void test_backup_intercept_success() {
        BackupTask backupTask = mockCnWareBackupTask();
        ProtectedResource domain = MockFactory.mockProtectedResource();
        domain.setAuth(new Authentication());
        Mockito.when(resourceService.getResourceById(ArgumentMatchers.anyBoolean(), ArgumentMatchers.any()))
            .thenReturn(Optional.of(domain));

        BackupTask interceptTask = backupProvider.initialize(backupTask);
        Assert.assertEquals(0, interceptTask.getProtectSubObjects().size());
        Assert.assertEquals(domain.getAuth(), interceptTask.getProtectObject().getAuth());
        Assert.assertEquals(2, interceptTask.getRepositories().size());
    }

    /**
     * 用例场景：Openstack备份下发全部磁盘 <br/>
     * 前置条件：保护对象高级参数全部磁盘参数为true <br/>
     * 检查点：下发的磁盘信息为空，其余参数正常
     */
    @Test
    public void test_backup_intercept_success_if_all_disk_is_true() {
        BackupTask backupTask = mockCnWareBackupTask();
        backupTask.getAdvanceParams().put(CnwareConstant.ALL_DISK, "true");
        ClustersInfoVo localClusterVoInfo = new ClustersInfoVo();
        localClusterVoInfo.setStorageEsn("esn_01");
        Mockito.when(resourceService.getResourceById(ArgumentMatchers.anyBoolean(), ArgumentMatchers.any()))
            .thenReturn(Optional.of(MockFactory.mockProtectedResource()));

        BackupTask interceptTask = backupProvider.initialize(backupTask);
        Assert.assertEquals(0, interceptTask.getProtectSubObjects().size());
        Assert.assertEquals(2, interceptTask.getRepositories().size());
    }

    private BackupTask mockCnWareBackupTask() {
        BackupTask backupTask = new BackupTask();

        TaskResource protectObject = new TaskResource();
        protectObject.setUuid(UUIDGenerator.getUUID());
        protectObject.setParentUuid(UUIDGenerator.getUUID());
        Map<String, String> proExtendInfo = new HashMap<>();
        proExtendInfo.put(CnwareConstant.DOMAIN_ID_KEY, "default");
        protectObject.setExtendInfo(proExtendInfo);
        backupTask.setProtectObject(protectObject);

        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put(CnwareConstant.DISK_INFO, "[]");
        advanceParams.put(CnwareConstant.ALL_DISK, "ture");
        backupTask.setAdvanceParams(advanceParams);

        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        TaskEnvironment taskEnvironment = BeanTools.copy(environment, TaskEnvironment::new);
        backupTask.setProtectEnv(taskEnvironment);

        StorageRepository dataRepository = new StorageRepository();
        dataRepository.setType(RepositoryTypeEnum.DATA.getType());
        List<StorageRepository> repositoryList = new ArrayList<>();
        repositoryList.add(dataRepository);
        backupTask.setRepositories(repositoryList);
        return backupTask;
    }

}
