/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

package openbackup.oracle.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyList;
import static org.mockito.ArgumentMatchers.anyMap;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.MessageTemplate;

import com.alibaba.fastjson.JSON;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * oracle副本恢复 测试类
 *
 * @author c30038333
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/2/2
 */
public class OracleRestoreProviderTest {
    private final OracleBaseService oracleBaseService = Mockito.mock(OracleBaseService.class);
    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);
    private final OracleSingleRestoreProvider singleProvider = Mockito.mock(OracleSingleRestoreProvider.class);
    private final OracleClusterRestoreProvider clusterProvider = Mockito.mock(OracleClusterRestoreProvider.class);
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private MessageTemplate messageTemplate = Mockito.mock(MessageTemplate.class);
    private EncryptorService encryptorService = Mockito.mock(EncryptorService.class);
    private AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
    private OracleRestoreProvider provider;

    @Before
    public void init() {
        provider = new OracleRestoreProvider(copyRestApi, oracleBaseService, singleProvider, clusterProvider,
                resourceService);
        provider.setEncryptorService(encryptorService);
        provider.setMessageTemplate(messageTemplate);
        provider.setAgentUnifiedService(agentUnifiedService);
    }

    /**
     * 用例场景：oracle恢复连通性检查
     * 前置条件：无
     * 检查点：检查通过
     */
    @Test
    public void check_connection_success() {
        provider.checkConnention(new RestoreTask());
        Assert.assertNotNull(provider);
    }

    /**
     * 用例场景：Oracle恢复过滤
     * 前置条件：副本类型为oracle或者oracle-cluster
     * 检查点：返回true
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.ORACLE_CLUSTER.getType()));
    }

    /**
     * 用例场景：恢复时检查是否需要环境离线
     * 前置条件：无
     * 检查点：shouldCheckEnvironmentIsOnline 为 false
     */
    @Test
    public void should_return_false_when_get_restore_feature() {
        Assert.assertFalse(provider.getRestoreFeature().isShouldCheckEnvironmentIsOnline());
    }

    /**
     * 用例场景：supplyRestoreTask
     * 前置条件：用单机副本恢复到集群
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_lego_checked_exception_when_restore_from_single_to_cluster() {
        RestoreTask restoreTask = mockRestoreTask();
        restoreTask.getTargetObject().setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());

        Mockito.when(copyRestApi.queryCopyByID(any())).thenReturn(mockCopy(ResourceSubTypeEnum.ORACLE));

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.supplyRestoreTask(restoreTask));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, exception.getErrorCode());
    }

    /**
     * 用例场景：supplyRestoreTask
     * 前置条件：源位置与目标位置数据库版本不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_lego_checked_exception_if_version_not_matches() {
        RestoreTask restoreTask = mockRestoreTask();
        ProtectedResource resource = new ProtectedResource();
        resource.setVersion("11.1.0.2.0");
        restoreTask.getTargetObject().setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());

        Mockito.when(copyRestApi.queryCopyByID(any())).thenReturn(mockCopy(ResourceSubTypeEnum.ORACLE_CLUSTER));
        Mockito.when(oracleBaseService.getResource(any())).thenReturn(resource);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.supplyRestoreTask(restoreTask));
        Assert.assertEquals(DatabaseErrorCode.RESTORE_RESOURCE_VERSION_INCONSISTENT, exception.getErrorCode());
    }

    /**
     * 用例场景：资源锁
     * 前置条件：无
     * 检查点：资源锁id符合预期
     */
    @Test
    public void get_lock_resources_success() {
        RestoreTask restoreTask = mockRestoreTask();
        Copy copy = mockCopy(ResourceSubTypeEnum.ORACLE_CLUSTER);
        copy.setResourceName("oracle");
        restoreTask.getTargetObject().setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());
        TaskEnvironment targetEnv = new TaskEnvironment();
        targetEnv.setUuid("123");
        restoreTask.setTargetEnv(targetEnv);
        Mockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        List<LockResourceBo> lockResources = provider.getLockResources(restoreTask);
        Assert.assertEquals("oracle123", lockResources.get(0).getId());
    }

    /**
     * 用例场景：设置下次全量
     * 前置条件：目标数据库存在
     * 检查点：目标数据库的id符合预期
     */
    @Test
    public void find_associated_resources_to_set_next_full_success() {
        Copy copy = mockCopy(ResourceSubTypeEnum.ORACLE_CLUSTER);
        copy.setResourceName("oracle");
        Mockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);

        RestoreTask restoreTask = mockRestoreTask();
        restoreTask.getTargetObject().setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());
        TaskEnvironment targetEnv = new TaskEnvironment();
        targetEnv.setUuid("123");
        restoreTask.setTargetEnv(targetEnv);

        ProtectedEnvironment cluster = new ProtectedEnvironment();
        cluster.setUuid("456");

        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        List<ProtectedResource> resource = new ArrayList<>();
        resource.add(cluster);
        response.setRecords(resource);

        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        List<String> full = provider.findAssociatedResourcesToSetNextFull(restoreTask);
        Assert.assertEquals(full.get(0), cluster.getUuid());
    }

    /**
     * 用例场景：设置下次增量转全量
     * 前置条件：目标位置数据库不存在
     * 检查点：返回空数组
     */
    @Test
    public void should_return_empty_when_restore_to_new_database() {
        Copy copy = mockCopy(ResourceSubTypeEnum.ORACLE_CLUSTER);
        copy.setResourceName("oracle");
        Mockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);

        RestoreTask restoreTask = mockRestoreTask();
        restoreTask.getTargetObject().setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());
        TaskEnvironment targetEnv = new TaskEnvironment();
        targetEnv.setUuid("123");
        restoreTask.setTargetEnv(targetEnv);

        PageListResponse<ProtectedResource> response = new PageListResponse<>();

        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        List<String> full = provider.findAssociatedResourcesToSetNextFull(restoreTask);
        Assert.assertEquals(full.size(), 0);
    }

    /**
     * 用例场景：设置恢复模式
     * 前置条件：副本类型不为CloudArchive或TapeArchive
     * 检查点：恢复模式为LocalRestore
     */
    @Test
    public void should_set_local_restore_mode_if_copy_is_not_cloud_or_tape() {
        RestoreTask restoreTask = mockRestoreTask();
        restoreTask.getTargetEnv().setSubType(ResourceSubTypeEnum.ORACLE.getType());
        ProtectedResource resource = new ProtectedResource();
        resource.setVersion("12.1.0.2.0");
        Copy copy = mockCopy(ResourceSubTypeEnum.ORACLE_CLUSTER);
        restoreTask.getTargetObject().setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());
        Map<String, Object> property = new HashMap<>();
        copy.setProperties(JSON.toJSONString(property));
        Mockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        Mockito.when(oracleBaseService.getResource(any())).thenReturn(resource);
        Mockito.when(copyRestApi.queryCopies(anyInt(), anyInt(), anyMap(), anyList())).thenReturn(mockLogCopies());
        provider.supplyRestoreTask(restoreTask);
        Assert.assertEquals(restoreTask.getRestoreMode(), RestoreModeEnum.LOCAL_RESTORE.getMode());
    }

    private RestoreTask mockRestoreTask() {
        RestoreTask task = new RestoreTask();

        task.setAdvanceParams(new HashMap<>());
        task.getAdvanceParams().put("restoreFrom", "data");

        TaskResource targetObject = new TaskResource();
        targetObject.setUuid("d79889d96c2040129ab99e8f02c807b6");
        task.setTargetObject(targetObject);

        List<StorageRepository> repositories = new ArrayList<>();
        repositories.add(new StorageRepository());
        task.setRepositories(repositories);

        task.setTargetEnv(new TaskEnvironment());
        return task;
    }

    private Copy mockCopy(ResourceSubTypeEnum subType) {
        Copy copy = new Copy();

        copy.setUuid("1065f7ed-a8d5-4e51-aa0b-f10cc9a3ba3e");
        copy.setSlaProperties("sla");
        copy.setResourceSubType(subType.getType());
        copy.setResourceName("test");
        copy.setSourceCopyType(RepositoryTypeEnum.DATA.getType());
        Map<String, Object> resource = new HashMap<>();
        resource.put(DatabaseConstants.VERSION, "12.1.0.2.0");
        copy.setResourceProperties(JSON.toJSONString(resource));

        return copy;
    }

    private BasePage<Copy> mockLogCopies() {
        BasePage<Copy> basePage = new BasePage<>();
        basePage.setItems(new ArrayList<>());
        Copy log = new Copy();
        log.setUuid("123");
        basePage.getItems().add(log);
        return basePage;
    }

    /**
     * 用例场景：恢复后置操作
     * 前置条件：恢复任务执行成功
     * 检查点：无
     */
    @Test
    public void execute_post_process_success() {
        RestoreTask restoreTask = mockRestoreTask();
        Copy copy = mockCopy(ResourceSubTypeEnum.ORACLE_CLUSTER);
        Mockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ProtectedResource resource = Mockito.mock(ProtectedResource.class);
        resource.setName("123");
        List<ProtectedResource> resources = new ArrayList<>();
        resources.add(resource);
        response.setRecords(resources);
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(response);
        provider.postProcess(restoreTask, ProviderJobStatusEnum.SUCCESS);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(any());
    }

    @Test
    public void execute_supply_snapshot_agents_success() {
        RestoreTask restoreTask = mockRestoreTask();
        Copy copy = mockCopy(ResourceSubTypeEnum.ORACLE_CLUSTER);
        Mockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        restoreTask.setAgents(Collections.singletonList(new Endpoint("1", "9.9.9.9", 9999)));
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ProtectedResource resource = Mockito.mock(ProtectedResource.class);
        resource.setName("123");
        List<ProtectedResource> resources = new ArrayList<>();
        resources.add(resource);
        response.setRecords(resources);
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(response);
        provider.supplySnapshotAgents(restoreTask);
    }

    @Test
    public void execute_supply_snapshot_nodes_success() {
        RestoreTask restoreTask = mockRestoreTask();
        Copy copy = mockCopy(ResourceSubTypeEnum.ORACLE_CLUSTER);
        Mockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ProtectedResource resource = Mockito.mock(ProtectedResource.class);
        resource.setName("123");
        List<ProtectedResource> resources = new ArrayList<>();
        resources.add(resource);
        response.setRecords(resources);
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(response);
        provider.supplySnapshotNodes(restoreTask);
    }
}

