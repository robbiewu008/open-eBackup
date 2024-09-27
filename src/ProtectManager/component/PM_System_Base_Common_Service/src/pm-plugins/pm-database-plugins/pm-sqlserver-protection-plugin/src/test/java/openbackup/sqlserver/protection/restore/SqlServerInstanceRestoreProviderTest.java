package openbackup.sqlserver.protection.restore;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * sqlserver单实例恢复测试类
 *
 * @author xWX950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-21
 */
@RunWith(SpringRunner.class)
@ComponentScan("com.huawei.oceanprotect.sqlserver.protection.restore")
@SpringBootTest(classes = {SqlServerInstanceRestoreProvider.class, SqlServerBaseService.class})
public class SqlServerInstanceRestoreProviderTest {
    @Autowired
    private SqlServerInstanceRestoreProvider sqlServerInstanceRestoreProvider;

    @Autowired
    private SqlServerBaseService sqlServerBaseService;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    protected ProviderManager providerManager;

    @MockBean
    private AgentUnifiedService agentUnifiedService;

    @MockBean
    private AgentUnifiedRestApi agentUnifiedRestApi;

    @MockBean
    @Qualifier("unifiedConnectionCheckProvider")
    private ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    @MockBean
    private RepositoryStrategyManager repositoryStrategyManager;

    @Before
    public void init() throws IllegalAccessException {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());
        copy.setBackupType(1);
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
    }

    /**
     * 用例场景：实例恢复provider过滤
     * 前置条件：实例备份正常
     * 检 查 点：实例恢复provider过滤
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(
            sqlServerInstanceRestoreProvider.applicable(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType()));
    }

    /**
     * 用例场景：下发单实例恢复任务
     * 前置条件：1. 资源是单实例
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void single_instance_restore_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_INSTANCE);
        task.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        ProtectedResource resource = new ProtectedResource();
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any()))
            .thenReturn(mockContext(DatabaseConstants.SUCCESS_CODE));
        HostDto hostDto = new HostDto();
        PowerMockito.when(agentUnifiedService.getHost(anyString(),anyInt())).thenReturn(hostDto);
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("8.40.99.101");
        endpoint.setPort(2181);
        endpoint.setId("123");
        sqlServerInstanceRestoreProvider.initialize(task);
        Assert.assertEquals("8.40.99.101", task.getAgents().get(0).getIp());
        Assert.assertEquals(2181, task.getAgents().get(0).getPort());
    }

    /**
     * 用例场景：下发单实例恢复任务
     * 前置条件：资源是单实例并且备份正常
     * 检 查 点：恢复时联通性检查失败
     */
    @Test
    public void should_throw_LegoCheckedException_single_instance_restore_task_connect_failed()
        throws IllegalAccessException {
        RestoreTask task = mockRestoreTask();
        task.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any())).thenReturn(mockContext(200));
        Assert.assertThrows("check connection failed.", LegoCheckedException.class,
            () -> sqlServerInstanceRestoreProvider.restoreTaskCreationPreCheck(task));
    }

    /**
     * 用例场景：下发单实例恢复任务
     * 前置条件：资源是单实例并且备份正常
     * 检 查 点：恢复时联通性检查结果为空
     */
    @Test
    public void should_throw_LegoCheckedException_single_instance_restore_task_connect_result_is_empty()
        throws IllegalAccessException {
        RestoreTask task = mockRestoreTask();
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any()))
            .thenReturn(new ResourceCheckContext());
        task.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        Assert.assertThrows("check connection result is empty.", LegoCheckedException.class,
            () -> sqlServerInstanceRestoreProvider.restoreTaskCreationPreCheck(task));
    }

    /**
     * 用例场景：恢复锁住资源成功
     * 前置条件：1. 单实例数据库级别恢复到新位置，targetObj是新单实例-->锁：新位置单实例资源
     * 检 查 点：1. 返回正确的加锁资源
     */
    @Test
    public void instance_flr_new_restore_resource_lock_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_INSTANCE);
        task.setTargetLocation(RestoreLocationEnum.NEW);
        task.setRestoreType(RestoreTypeEnum.FLR.getType());
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));
        List<LockResourceBo> lockResources = sqlServerInstanceRestoreProvider.getLockResources(task);
        Assert.assertEquals(1, lockResources.size());
    }

    /**
     * 用例场景：恢复锁住资源成功
     * 前置条件：1、单实例恢复到原位置，targetObj是原单实例-->锁：原位置单实例资源，实例下所有数据库
     *         2、单实例数据库级别恢复到原位置，targetObj是原单实例-->锁：原位置单实例资源，实例下所有数据库
     * 检 查 点：1. 返回正确的加锁资源
     */
    @Test
    public void instance_origin_restore_resource_lock_success() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_INSTANCE);
        task.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));
        PowerMockito.when(resourceService.queryRelatedResourceUuids(anyString(), any()))
            .thenReturn(new HashSet<>(Collections.singleton(UUID.randomUUID().toString())));
        List<LockResourceBo> lockResources = sqlServerInstanceRestoreProvider.getLockResources(task);
        Assert.assertEquals(2, lockResources.size());
    }

    private RestoreTask mockRestoreTask() {
        RestoreTask task = getDatabaseRestoreTask(ResourceSubTypeEnum.SQL_SERVER_INSTANCE);
        ProtectedResource resource = new ProtectedResource();
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        return task;
    }

    private RestoreTask getDatabaseRestoreTask(ResourceSubTypeEnum subTypeEnum) {
        RestoreTask task = new RestoreTask();
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository storageRepository = new StorageRepository();
        repositories.add(storageRepository);
        task.setRepositories(repositories);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setNodes(new ArrayList<>());
        Map<String, String> taskEnvironmentExtendInfo = new HashMap<>();
        taskEnvironment.setExtendInfo(taskEnvironmentExtendInfo);
        taskEnvironment.setEndpoint("8.40.99.101");
        taskEnvironment.setPort(2181);
        taskEnvironment.setUuid("1232534");
        task.setTargetEnv(taskEnvironment);
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUID.randomUUID().toString());
        taskResource.setParentUuid(UUID.randomUUID().toString());
        taskResource.setSubType(subTypeEnum.getType());
        task.setTargetObject(taskResource);
        task.setAgents(new ArrayList<>());
        return task;
    }

    private ResourceCheckContext mockContext(int code) {
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(code);
        actionResult.setBodyErr("1024");
        List<ActionResult> list = new ArrayList<>();
        list.add(actionResult);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(list);
        return context;
    }
}