/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.db2.protection.access.provider.copy;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.dme.AvailableTimeRanges;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2Service;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.OpServiceUtil;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * {@link Db2CopyDeleteInterceptorProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-13
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(OpServiceUtil.class)
public class Db2CopyDeleteInterceptorProviderTest {
    private final Db2Service db2Service = PowerMockito.mock(Db2Service.class);

    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final CopyService copyService = PowerMockito.mock(CopyService.class);

    private final IVpcService vpcService = PowerMockito.mock(IVpcService.class);

    private Db2CopyDeleteInterceptorProvider provider = new Db2CopyDeleteInterceptorProvider(copyRestApi,
        resourceService, db2Service, instanceResourceService);

    @Before
    public void init() {
        provider.setCopyService(copyService);
        provider.setVpcService(vpcService);
    }

    /**
     * 用例场景：框架调用到db2的副本删除
     * 前置条件：applicable输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_db2_cluster_instance_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
    }

    /**
     * 用例场景：收集删除全量副本时需要关联删除的副本ID列表
     * 前置条件：要删除的全量副本没有关联的日志副本
     * 检查点：返回空
     */
    @Test
    public void should_return_empty_if_full_is_latest_when_getCopiesCopyTypeIsFull() {
        List<String> needDelCopyIds = provider.getCopiesCopyTypeIsFull(new ArrayList<>(), mockThisCopy(), null);
        Assert.assertEquals(0, needDelCopyIds.size());
    }

    /**
     * 用例场景：收集删除全量副本时需要关联删除的副本ID列表
     * 前置条件：要删除的全量副本有关联的日志副本，日志副本开始时间等于全量副本备份时间
     * 检查点：返回副本列表
     */
    @Test
    public void should_return_all_log_copy_if_full_and_log_start_time_equal_when_getCopiesCopyTypeIsFull() {
        Copy thisCopy = mockThisCopy();
        Copy firstLogCopy = mockLogCopy();
        firstLogCopy.setProperties("{\"beginTime\":1678972641,\"endTime\":1678974888,\"backupTime\":1678974888}");
        List<String> needDelCopyIds = provider.getCopiesCopyTypeIsFull(Collections.singletonList(firstLogCopy),
            thisCopy, null);
        Assert.assertEquals(1, needDelCopyIds.size());
    }

    /**
     * 用例场景：收集删除全量副本时需要关联删除的副本ID列表
     * 前置条件：要删除的全量副本有关联的日志副本，日志副本开始时间小于全量副本备份时间，日志副本开始时间关联副本不存在
     * 检查点：返回副本列表
     */
    @Test
    public void should_return_log_copy_if_log_less_full_and_begin_copy_not_exists_when_getCopiesCopyTypeIsFull() {
        Copy thisCopy = mockThisCopy();
        Copy firstLogCopy = mockLogCopy();
        firstLogCopy.setProperties("{\"beginTime\":1678972458,\"endTime\":1678974888,\"backupTime\":1678974888}");
        PageListResponse<AvailableTimeRanges> pageListResponse = new PageListResponse<>(0, Collections.emptyList());
        PowerMockito.when(copyService.listAvailableTimeRanges(anyString(), anyLong(), anyLong(), anyInt(), anyInt()))
            .thenReturn(pageListResponse);
        List<String> needDelCopyIds = provider.getCopiesCopyTypeIsFull(Collections.singletonList(firstLogCopy),
            thisCopy, null);
        Assert.assertEquals(1, needDelCopyIds.size());
    }

    /**
     * 用例场景：收集删除全量副本时需要关联删除的副本ID列表
     * 前置条件：要删除的全量副本有关联的日志副本，日志副本开始时间小于全量副本备份时间，日志副本开始时间关联副本存在
     * 检查点：返回空列表
     */
    @Test
    public void getCopiesCopyTypeIsFull_success_if_delFullAssociateLog_logStartTLessFullBakT_beginTCopyExists() {
        Copy thisCopy = mockThisCopy();
        Copy firstLogCopy = mockLogCopy();
        firstLogCopy.setProperties("{\"beginTime\":1678972457,\"endTime\":1678974888,\"backupTime\":1678974888}");
        AvailableTimeRanges timeRanges = new AvailableTimeRanges();
        timeRanges.setStartTime(1678972457);
        timeRanges.setEndTime(1678972459);
        PageListResponse<AvailableTimeRanges> pageListResponse = new PageListResponse<>(1,
            Collections.singletonList(timeRanges));
        PowerMockito.when(copyService.listAvailableTimeRanges(anyString(), anyLong(), anyLong(), anyInt(), anyInt()))
            .thenReturn(pageListResponse);
        List<String> needDelCopyIds = provider.getCopiesCopyTypeIsFull(Collections.singletonList(firstLogCopy),
            thisCopy, null);
        Assert.assertEquals(0, needDelCopyIds.size());
    }

    /**
     * 用例场景：当删除副本是增量副本时，需要关联删除的副本
     * 前置条件：删除副本为增量副本
     * 检查点：返回关联删除的副本是否正确
     */
    @Test
    public void get_copies_when_this_copy_is_difference_increment() {
        List<String> copyUuids = provider.getCopiesCopyTypeIsDifferenceIncrement(mockCopies(), mockThisCopy(),
            mockNextFullCopy());
        Assert.assertEquals(0, copyUuids.size());
    }

    /**
     * 用例场景：设置agent信息
     * 前置条件：资源存在
     * 检查点：返回正确的agent信息
     */
    @Test
    public void supply_agent_success() {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copy = new CopyInfoBo();
        copy.setResourceId(UUID.randomUUID().toString());
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(new ProtectedResource());
        PowerMockito.when(db2Service.getAgentsByInstanceResource(any()))
            .thenReturn(Arrays.asList(new Endpoint("127.0.0.1", 50000)));
        provider.supplyAgent(task, copy);
        Assert.assertEquals(1, task.getAgents().size());
    }

    /**
     * 用例场景：设置agent信息
     * 前置条件：资源存在
     * 检查点：返回正确的agent信息
     */
    @Test
    public void should_set_vpc_if_hcs_service_when_supply_agent() throws Exception {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copy = new CopyInfoBo();
        copy.setResourceId(UUID.randomUUID().toString());
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(new ProtectedResource());
        PowerMockito.when(db2Service.getAgentsByInstanceResource(any()))
            .thenReturn(Arrays.asList(new Endpoint("127.0.0.1", 50000)));
        PowerMockito.when(resourceService.getBasicResourceById(anyBoolean(), any()))
            .thenReturn(Optional.of(mockResource()));
        PowerMockito.when(vpcService.getVpcByVpcIds(any())).thenReturn(new ArrayList<>());
        PowerMockito.mockStatic(OpServiceUtil.class);
        PowerMockito.doReturn(true).when(OpServiceUtil.class, "isHcsService");
        provider.supplyAgent(task, copy);
        Assert.assertNotNull(task.getAdvanceParams().get(DatabaseConstants.VPC_INFO_KEY));
    }

    /**
     * 用例场景：当资源存在时，副本删除设置任务处理参数，
     * 前置条件：回调到任务处理方法
     * 检查点：返回正确的参数信息
     */
    @Test
    public void execute_handle_task_when_resource_exist() {
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(mockResource()));
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(new ProtectedResource());
        PowerMockito.when(db2Service.getEnvNodesByInstanceResource(any())).thenReturn(mockNodes());
        DeleteCopyTask task = mockDeleteCopyTask();
        provider.handleTask(task, any());
        Assert.assertTrue(task.getIsForceDeleted());
    }

    /**
     * 用例场景：当资源不存存在时，副本删除设置任务处理参数，
     * 前置条件：回调到任务处理方法
     * 检查点：返回正确的参数信息
     */
    @Test
    public void execute_handle_task_when_resource_not_exist() {
        PowerMockito.when(instanceResourceService.getResourceById(any())).thenReturn(new ProtectedResource());
        PowerMockito.when(db2Service.getEnvNodesByInstanceResource(any())).thenReturn(mockNodes());
        DeleteCopyTask task = mockDeleteCopyTask();
        task.getProtectObject().setUuid(null);
        provider.handleTask(task, new CopyInfoBo());
        Assert.assertTrue(task.getIsForceDeleted());
    }

    /**
     * 用例场景：副本删除执行后置任务
     * 前置条件：副本删除成功
     * 检查点：无异常
     */
    @Test
    public void execute_post_process_success() {
        PowerMockito.when(copyRestApi.queryLatestBackupCopy(any(), any(), any())).thenReturn(mockCopy());
        provider.finalize(mockCopy(), mockTask());
        Assert.assertTrue(true);
    }

    private Copy mockCopy() {
        Copy copy = new Copy();
        copy.setUuid(UUID.randomUUID().toString());
        copy.setResourceId(UUID.randomUUID().toString());
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(1);
        copy.setGn(1);
        return copy;
    }

    private TaskCompleteMessageBo mockTask() {
        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        taskMessage.setJobStatus(3);
        taskMessage.setJobRequestId(UUID.randomUUID().toString());
        return taskMessage;
    }

    private Copy mockThisCopy() {
        Copy thisCopy = new Copy();
        thisCopy.setGn(1);
        thisCopy.setUuid(UUID.randomUUID().toString());
        thisCopy.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        thisCopy.setProperties("{\"backupTime\":1678972641}");
        thisCopy.setResourceId(UUID.randomUUID().toString());
        return thisCopy;
    }

    private Copy mockLogCopy() {
        Copy copy = new Copy();
        copy.setGn(2);
        copy.setUuid(UUID.randomUUID().toString());
        copy.setBackupType(BackupTypeConstants.LOG.getAbBackupType());
        return copy;
    }

    private List<Copy> mockCopies() {
        Copy copy2 = new Copy();
        copy2.setGn(2);
        copy2.setUuid(UUID.randomUUID().toString());
        copy2.setBackupType(BackupTypeConstants.LOG.getAbBackupType());
        Copy copy4 = new Copy();
        copy4.setGn(4);
        copy4.setUuid(UUID.randomUUID().toString());
        copy4.setBackupType(BackupTypeConstants.LOG.getAbBackupType());
        return Arrays.asList(copy2, mockNextFullCopy(), copy4);
    }

    private Copy mockNextFullCopy() {
        Copy nextFullCopy = new Copy();
        nextFullCopy.setGn(3);
        nextFullCopy.setUuid(UUID.randomUUID().toString());
        nextFullCopy.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        return nextFullCopy;
    }

    private DeleteCopyTask mockDeleteCopyTask() {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.DPF.getType());
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setExtendInfo(extendInfo);
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid(UUID.randomUUID().toString());
        DeleteCopyTask task = new DeleteCopyTask();
        task.setProtectEnv(taskEnvironment);
        task.setProtectObject(protectObject);
        return task;
    }

    private List<TaskEnvironment> mockNodes() {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        List<TaskEnvironment> nodes = new ArrayList<>();
        nodes.add(taskEnvironment);
        return nodes;
    }

    private ProtectedResource mockResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setRootUuid(UUID.randomUUID().toString());
        resource.setExtendInfoByKey(ResourceConstants.VPC_ID, "vpcId");
        return resource;
    }
}