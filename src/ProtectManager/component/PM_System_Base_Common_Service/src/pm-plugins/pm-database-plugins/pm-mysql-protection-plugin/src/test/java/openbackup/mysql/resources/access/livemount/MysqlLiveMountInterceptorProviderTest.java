/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.mysql.resources.access.livemount;

import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.livemount.common.constants.LiveMountConstants;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.database.base.plugin.utils.StorageRepositoryUtil;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.interceptor.MysqlBaseMock;

import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

import static org.mockito.ArgumentMatchers.any;

/**
 * mysql即时挂载provider
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/1
 */
public class MysqlLiveMountInterceptorProviderTest {
    private final MysqlBaseService mysqlBaseService = Mockito.mock(MysqlBaseService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);

    private final MysqlLiveMountInterceptorProvider provider = new MysqlLiveMountInterceptorProvider(mysqlBaseService,
            agentUnifiedService, copyRestApi);

    /**
     * 用例场景：provider应用检验
     * 前置条件：资源类型为单实例
     * 检查点：返回true
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(provider.applicable(MysqlResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType()));
    }

    /**
     * 用例场景：provider初始化资源成功
     * 前置条件：资源类型为单实例，没有存储库
     * 检查点：不报错
     */
    @Test
    public void init_liveMount_create_task_param_with_no_repositories() {
        mockGetEndpoint();
        LiveMountCreateTask liveMountCreateTask = getLiveMountCreateTask();
        provider.initialize(liveMountCreateTask);
        Assert.assertEquals(liveMountCreateTask.getAgents().size(), 1);
    }

    /**
     * 用例场景：provider初始化资源成功
     * 前置条件：资源类型为单实例，有存储库
     * 检查点：不报错
     */
    @Test
    public void init_liveMount_create_task_param_with_repositories() {
        LiveMountCreateTask createTask = getLiveMountCreateTask();
        setDataStorageRepository(createTask);
        setExtendInfo(createTask);
        mockGetEndpoint();
        Copy copy = MysqlBaseMock.getCopy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        PowerMockito.doNothing().when(mysqlBaseService).checkSubType(any(),any());
        PowerMockito.doNothing().when(mysqlBaseService).checkVersion(any(),any());
        PowerMockito.doNothing().when(mysqlBaseService).checkNewDatabaseName(any());
        PowerMockito.doNothing().when(mysqlBaseService).checkClusterType(any(),any());
        provider.initialize(createTask);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(any());
    }

    /**
     * 用例场景：provider初始化卸载资源成功
     * 前置条件：资源类型为单实例，有存储库
     * 检查点：不报错
     */
    @Test
    public void init_liveMount_cancel_task_param_success() {
        LiveMountCancelTask cancelTask = getLiveMountCancelTask();
        setDataStorageRepository(cancelTask);
        mockGetEndpoint();
        provider.finalize(cancelTask);
        Assert.assertTrue(cancelTask.getAgents().size() == 1);
        Assert.assertTrue(cancelTask.getTargetEnv().getNodes().size() == 1);
        StorageRepository dataStorageRepository = StorageRepositoryUtil.getDataStorageRepository(cancelTask.getRepositories());
        Assert.assertTrue( LiveMountConstants.TRUE.equals(dataStorageRepository.getExtendInfo().get(LiveMountConstants.MANUAL_MOUNT)));
    }

    private void setDataStorageRepository(BaseTask baseTask) {
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(RepositoryTypeEnum.DATA.getType());
        List<StorageRepository> storageRepositories = Arrays.asList(storageRepository);
        baseTask.setRepositories(storageRepositories);
    }

    private void setExtendInfo(BaseTask baseTask) {
        HashMap<String, String> extendInfo = new HashMap<>();
        baseTask.getTargetObject().setExtendInfo(extendInfo);
    }

    private LiveMountCancelTask getLiveMountCancelTask() {
        LiveMountCancelTask cancelTask = new LiveMountCancelTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setParentUuid("111");
        cancelTask.setTargetObject(taskResource);
        return cancelTask;
    }

    private LiveMountCreateTask getLiveMountCreateTask() {
        LiveMountCreateTask createTask = new LiveMountCreateTask();
        TaskEnvironment targetEnv = new TaskEnvironment();
        TaskResource taskResource = new TaskResource();
        createTask.setTargetObject(taskResource);
        targetEnv.setUuid(UUID.randomUUID().toString());
        createTask.setTargetEnv(targetEnv);
        createTask.setAdvanceParams(new HashMap<>());
        LiveMountFileSystemShareInfo liveMountFileSystemShareInfo = new LiveMountFileSystemShareInfo();
        List<LiveMountFileSystemShareInfo> systemShareInfos = Arrays.asList(liveMountFileSystemShareInfo);
        createTask.getAdvanceParams().put(LiveMountConstants.FILE_SYSTEM_SHARE_INFO, systemShareInfos);
        return createTask;
    }

    private void mockGetEndpoint() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setExtendInfo(new HashMap<>());
        PowerMockito.when(mysqlBaseService.getEnvironmentById(any())).thenReturn(protectedEnvironment);
        HostDto hostDto = new HostDto();
        hostDto.setPort(3306);
        hostDto.setEndpoint("8.40.99.101");
        PowerMockito.when(agentUnifiedService.getHost(any(), any())).thenReturn(hostDto);
        Endpoint endpoint = new Endpoint();
        endpoint.setId(UUID.randomUUID().toString());
        PowerMockito.when(mysqlBaseService.getAgentEndpoint(protectedEnvironment)).thenReturn(endpoint);
    }
}
