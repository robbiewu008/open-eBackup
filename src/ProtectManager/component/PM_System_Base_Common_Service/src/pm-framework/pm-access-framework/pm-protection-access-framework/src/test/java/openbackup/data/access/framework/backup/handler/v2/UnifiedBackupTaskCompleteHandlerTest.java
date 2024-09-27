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
package openbackup.data.access.framework.backup.handler.v2;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.backup.service.impl.JobBackupPostProcessService;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.common.enums.v2.CopyTypeEnum;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.protection.listener.v2.UnifiedTaskCompleteListenerTest;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.Resource;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.protection.model.SlaBo;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.jupiter.api.Assertions;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * UnifiedBackupTaskCompleteHandler LLT
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-09
 */
@SpringBootTest
@RunWith(SpringRunner.class)
@ContextConfiguration(classes = {UnifiedBackupTaskCompleteHandler.class})
@ComponentScan(basePackages = "com.huawei.oceanprotect.data.access.framework.protection")
@MockBean({ProviderManager.class})
public class UnifiedBackupTaskCompleteHandlerTest {
    @MockBean
    private DmeUnifiedRestApi unifiedRestApi;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private JobCenterRestApi jobCenterRestApi;

    @MockBean
    private NotifyManager notifyManager;

    @MockBean
    private JobService jobService;

    @MockBean
    private BackupStorageApi backupStorageApi;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private UserQuotaManager userQuotaManager;

    @MockBean
    private DeployTypeService deployTypeService;

    @Autowired
    private UnifiedBackupTaskCompleteHandler unifiedBackupTaskCompleteHandler;

    @MockBean
    private MemberClusterService memberClusterService;

    @MockBean
    private JobBackupPostProcessService jobBackupPostProcessService;

    @MockBean
    private StorageUnitService storageUnitService;

    /**
     * 测试场景：任务成功处理方法执行成功
     * 前置条件：kafka组件正常
     * 检查点： 任务成功处理方法执行成功
     */
    @Test
    public void test_handle_backup_success_message() {
        UnifiedTaskCompleteListenerTest.mockQueryDmeCopyInfo(unifiedRestApi);

        UnifiedTaskCompleteListenerTest.mockSaveCopy(copyRestApi);

        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        String jobId = UUIDGenerator.getUUID();
        taskMessage.setJobId(jobId);
        taskMessage.setJobStatus(DmcJobStatus.SUCCESS.getStatus());
        Map<String, String> context = new HashMap<>();
        context.put("resource", UnifiedTaskCompleteListenerTest.RES_JSON);
        context.put("sla", UnifiedTaskCompleteListenerTest.SLA_JSON);
        context.put("chain_id", "chain_id");
        context.put("copy_format", "0");
        context.put("job_id", "0");
        taskMessage.setContext(context);
        JobBo jobBo = new JobBo();
        jobBo.setStorageUnitId("1111111111");
        PowerMockito.when(jobService.queryJob(anyString())).thenReturn(jobBo);
        NasDistributionStorageDetail detail = new NasDistributionStorageDetail();
        detail.setName("testname");
        PowerMockito.when(backupStorageApi.getDetail(ArgumentMatchers.anyString())).thenReturn(detail);
        JobBo job = new JobBo();
        job.setJobId(jobId);
        job.setSourceId(UUIDGenerator.getUUID());
        Mockito.when(jobService.queryJob(jobId)).thenReturn(job);
        Mockito.doNothing()
            .when(userQuotaManager)
            .increaseUsedQuota(anyString(), any());
        unifiedBackupTaskCompleteHandler.onTaskCompleteSuccess(taskMessage);
        Mockito.verify(notifyManager, Mockito.times(1)).send(anyString(), anyString());
    }

    /**
     * 测试场景：备份成功后生成副本时，如果资源子类型为文件集，则成功设置设备esn
     * 前置条件：1.备份任务完成；2.资源子类型不为DWS
     * 检查点： 设置设备esn成功
     */
    @Test
    public void should_setDeviceEsnSuccess_when_backupSuccessAndBuildCopy_given_resourceSubTypeIsFileset() {
        UnifiedTaskCompleteListenerTest.mockQueryDmeCopyInfo(unifiedRestApi);

        UnifiedTaskCompleteListenerTest.mockSaveCopy(copyRestApi);

        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        String jobId = UUIDGenerator.getUUID();
        taskMessage.setJobId(jobId);
        taskMessage.setJobStatus(DmcJobStatus.SUCCESS.getStatus());
        Map<String, String> context = new HashMap<>();
        context.put("resource", UnifiedTaskCompleteListenerTest.RES_JSON);
        context.put("sla", UnifiedTaskCompleteListenerTest.SLA_JSON);
        context.put("chain_id", "chain_id");
        context.put("copy_format", "0");
        context.put("job_id", jobId);
        taskMessage.setContext(context);
        NasDistributionStorageDetail detail = new NasDistributionStorageDetail();
        detail.setName("testname");
        PowerMockito.when(backupStorageApi.getDetail(ArgumentMatchers.anyString())).thenReturn(detail);
        JobBo job = new JobBo();
        job.setJobId(jobId);
        job.setSourceId(UUIDGenerator.getUUID());
        job.setStorageUnitId("111111");
        Mockito.when(jobService.queryJob(jobId)).thenReturn(job);
        Mockito.doNothing()
                .when(userQuotaManager)
                .increaseUsedQuota(anyString(), any());
        unifiedBackupTaskCompleteHandler.onTaskCompleteSuccess(taskMessage);
        Mockito.verify(notifyManager, Mockito.times(1)).send(anyString(), anyString());
    }

    /**
     * 测试场景：备份成功后生成副本时，如果资源子类型为DWS，则不设置设备esn
     * 前置条件：1.备份任务完成；2.资源子类型为DWS
     * 检查点： DWS类应用不设置设置设备esn
     */
    @Test
    public void should_callGetLocalClusterEsnZeroTimes_when_backupSuccessAndBuildCopy_given_resourceSubTypeIsDWS() {
        UnifiedTaskCompleteListenerTest.mockQueryDmeCopyInfo(unifiedRestApi);

        UnifiedTaskCompleteListenerTest.mockSaveCopy(copyRestApi);

        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        String jobId = UUIDGenerator.getUUID();
        taskMessage.setJobId(jobId);
        taskMessage.setJobStatus(DmcJobStatus.SUCCESS.getStatus());
        Map<String, String> context = new HashMap<>();
        context.put("resource", "{\"sub_type\": \"DWS-schema\"}");
        context.put("sla", UnifiedTaskCompleteListenerTest.SLA_JSON);
        context.put("chain_id", "chain_id");
        context.put("copy_format", "0");
        context.put("job_id", jobId);
        taskMessage.setContext(context);
        NasDistributionStorageDetail detail = new NasDistributionStorageDetail();
        detail.setName("testname");
        PowerMockito.when(backupStorageApi.getDetail(ArgumentMatchers.anyString())).thenReturn(detail);
        JobBo job = new JobBo();
        job.setJobId(jobId);
        job.setSourceId(UUIDGenerator.getUUID());
        job.setStorageUnitId("111111");
        Mockito.when(jobService.queryJob(jobId)).thenReturn(job);
        Mockito.doNothing()
                .when(userQuotaManager)
                .increaseUsedQuota(anyString(), any());
        unifiedBackupTaskCompleteHandler.onTaskCompleteSuccess(taskMessage);
        Mockito.verify(notifyManager, Mockito.times(1)).send(anyString(), anyString());
    }

    /**
     * 测试场景：任务失败处理方法执行成功
     * 前置条件：kafka组件正常
     * 检查点： 任务失败处理方法执行成功
     */
    @Test
    public void test_handle_backup_failed_message() {
        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        taskMessage.setJobStatus(DmcJobStatus.FAIL.getStatus());
        HashMap<String, String> context = new HashMap<>();
        context.put(ContextConstants.JOB_TYPE, JobTypeEnum.BACKUP.getValue());
        context.put(ContextConstants.COPY_FORMAT, "0");
        taskMessage.setContext(context);
        unifiedBackupTaskCompleteHandler.onTaskCompleteFailed(taskMessage);
        Mockito.verify(notifyManager, Mockito.times(1)).send(anyString(), anyString());
    }

    /**
     * 测试场景：当副本保存失败时，处理备份成功消息失败
     * 前置条件：1、告警业务层响应成功；2、入参符合规格
     * 检查点：1、处理流程抛出异常；2、备份任务停止开关打开；
     */
    @Test
    public void test_handle_backup_fail_when_copy_save_fail() {
        UnifiedTaskCompleteListenerTest.mockQueryDmeCopyInfo(unifiedRestApi);

        Mockito.when(copyRestApi.saveCopy(any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "save copy failed."));

        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        taskMessage.setTaskId(UUID.randomUUID().toString());
        taskMessage.setJobStatus(DmcJobStatus.SUCCESS.getStatus());
        Map<String, String> context = new HashMap<>();
        context.put("resource", UnifiedTaskCompleteListenerTest.RES_JSON);
        context.put("sla", UnifiedTaskCompleteListenerTest.SLA_JSON);
        context.put("chain_id", "chain_id");
        taskMessage.setContext(context);

        LegoCheckedException legoCheckedException = Assertions.assertThrows(LegoCheckedException.class,
            () -> unifiedBackupTaskCompleteHandler.onTaskCompleteSuccess(taskMessage));

        Assert.assertEquals(CommonErrorCode.OPERATION_FAILED, legoCheckedException.getErrorCode());
        Mockito.verify(jobService).updateJob(eq(taskMessage.getTaskId()), any());
    }

    /**
     * 用例场景：本地文件系统快照备份完成,组装副本对象
     * 前置条件：1.本地文件系统快照备份;
     * 检  查  点：返回副本对象
     */
    @Test
    public void build_copy_success() throws Exception {
        Map<String, String> context = new HashMap<>();
        Resource resource = new Resource();
        context.put("resource", JSONObject.writeValueAsString(resource));
        SlaBo slaBo = new SlaBo();
        slaBo.setPolicyList(Collections.EMPTY_LIST);
        context.put("sla", JSONObject.writeValueAsString(slaBo));
        DmeCopyInfo dmeCopyInfo = new DmeCopyInfo();
        TaskResource protectObject = new TaskResource();
        protectObject.setSubType(ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType());
        protectObject.setExtendInfo(new HashMap<>());
        dmeCopyInfo.setProtectObject(protectObject);
        dmeCopyInfo.setType(CopyTypeEnum.NATIVE_SNAPSHOT.getDmeCopyType());
        dmeCopyInfo.setSourceCopyType(CopyTypeEnum.NATIVE_SNAPSHOT.getDmeCopyType());
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setName("test");
        taskEnvironment.setEndpoint("127.0.0.1");
        dmeCopyInfo.setProtectEnv(taskEnvironment);
        TaskResource subObject = new TaskResource();
        subObject.setName("Test-SubObj");
        dmeCopyInfo.setProtectSubObject(Collections.singletonList(subObject));
        Mockito.when(unifiedRestApi.getCopyInfo("requestId")).thenReturn(dmeCopyInfo);
        context.put("job_id", "0");
        JobBo jobBo = new JobBo();
        jobBo.setStorageUnitId("1111111111");
        PowerMockito.when(jobService.queryJob(anyString())).thenReturn(jobBo);
        CopyInfo copyInfo = Whitebox.invokeMethod(unifiedBackupTaskCompleteHandler, "buildCopy", "requestId", context);
        Assert.assertEquals("requestId", copyInfo.getUuid());
        Map<String, Object> properties =
            JSONObject.toMap(JSONObject.fromObject(copyInfo.getProperties()), Object.class);
        Object obj = properties.get(CopyPropertiesKeyConstant.KEY_PROTECT_SUB_OBJECT);
        List<TaskResource> subObjects = JSONArray.toCollection((JSONArray) obj, TaskResource.class);
        Assert.assertEquals(1, subObjects.size());
        Assert.assertEquals("Test-SubObj", subObjects.get(0).getName());
    }
}
