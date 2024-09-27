package openbackup.informix.protection.access.provider.back;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.persistence.dao.ProtectedEnvironmentExtendInfoMapper;
import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentExtendInfoPo;
import com.huawei.oceanprotect.client.resource.manager.service.AgentLanFreeService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * InformixBackupInterceptorProviderTest
 *
 * @author zWX951267
 * @version [DataBackup 1.5.0]
 * @since 2023-05-26
 */
@Slf4j
public class InformixBackupInterceptorProviderTest {
    private static final InformixService informixService = PowerMockito.mock(InformixService.class);
    private static final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private static final AgentLanFreeService agentLanFreeService = PowerMockito.mock(AgentLanFreeService.class);
    private static final ProtectedEnvironmentExtendInfoMapper protectedEnvironmentExtendInfoMapper =
            PowerMockito.mock(ProtectedEnvironmentExtendInfoMapper.class);
    private static final InformixBackupInterceptorProvider provider =
            new InformixBackupInterceptorProvider(informixService,
            agentLanFreeService, protectedEnvironmentExtendInfoMapper, resourceService);

    /**
     * 用例场景：备份数据完善
     * 前置条件：无
     * 检查点：是否调用informixService进行参数校验与封装
     */
    @Test
    @Ignore
    public void intercept_failed() {
        BackupTask backupTask = new BackupTask();
        backupTask.setProtectEnv(new TaskEnvironment());
        backupTask.setProtectObject(new TaskResource());
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository dataRepository = new StorageRepository();
        dataRepository.setType(RepositoryTypeEnum.DATA.getType());
        dataRepository.setExtendInfo(new HashMap<>());
        repositories.add(dataRepository);
        backupTask.setRepositories(repositories);
        backupTask.getProtectObject().setSubType(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType());
        backupTask.getProtectObject().setExtendInfo(new HashMap<>());
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType());
        Mockito.when(informixService.getResourceById(any())).thenReturn(protectedResource);
        ActionResult[] actionResults = new ActionResult[1];
        actionResults[0] = new ActionResult(1, "123");
        Mockito.when(resourceService.check(any())).thenReturn(actionResults);
        Mockito.when(protectedEnvironmentExtendInfoMapper.selectOne(any()))
                .thenReturn(new ProtectedEnvironmentExtendInfoPo());
        provider.initialize(backupTask);
        Assert.assertEquals(3, backupTask.getRepositories().size());
    }

    /**
     * 用例场景：备份数据完善
     * 前置条件：无
     * 检查点：是否调用informixService进行参数校验与封装
     */
    @Test
    @Ignore
    public void intercept_success() {
        BackupTask backupTask = new BackupTask();
        backupTask.setProtectEnv(new TaskEnvironment());
        backupTask.setProtectObject(new TaskResource());
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository dataRepository = new StorageRepository();
        dataRepository.setType(RepositoryTypeEnum.DATA.getType());
        dataRepository.setExtendInfo(new HashMap<>());
        repositories.add(dataRepository);
        backupTask.setRepositories(repositories);
        backupTask.getProtectObject().setSubType(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType());
        backupTask.getProtectObject().setExtendInfo(new HashMap<>());
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType());
        ActionResult[] actionResults = new ActionResult[1];
        actionResults[0] = new ActionResult(1, "123");
        Mockito.when(resourceService.check(any())).thenReturn(actionResults);
        Mockito.when(protectedEnvironmentExtendInfoMapper.selectOne(any()))
                .thenReturn(new ProtectedEnvironmentExtendInfoPo());
        Mockito.when(informixService.getResourceById(any())).thenReturn(protectedResource);
        provider.initialize(backupTask);
        Assert.assertEquals(3, backupTask.getRepositories().size());
    }

    /**
     * 用例场景：applicable集群实例类型
     * 前置条件：类型为Informix-clusterInstance/Informix-singleInstance
     * 检查点：是否返回true
     */
    @Test
    public void applicable_informix_intercept_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType()));
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType()));
    }
}