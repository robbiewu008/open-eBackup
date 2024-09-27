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
package openbackup.data.access.framework.protection.controller;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.controller.JobController;
import openbackup.data.access.framework.protection.controller.req.UpdateJobStatusRequest;
import openbackup.data.access.framework.protection.listener.ITaskCompleteListener;
import openbackup.data.access.framework.protection.service.job.UnifiedJobProvider;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.accesspoint.model.StopPlanBo;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;

import java.lang.reflect.Field;
import java.util.UUID;

/**
 * JobControllerTest LLT
 *
 * @author m00576658
 * @since 2021-03-05
 */

@RunWith(PowerMockRunner.class)
@PrepareForTest(JobController.class)
public class JobControllerTest {
    @Mock
    private ProviderManager registry;

    @Mock
    private JobCenterRestApi jobCenter;

    @Mock
    private ScheduleRestApi scheduleRestApi;

    @Mock
    private DeployTypeService deployTypeService;

    @Mock
    private JobService jobService;

    @Mock
    private ITaskCompleteListener taskCompleteListener;

    @InjectMocks
    private JobController jobController;

    @Mock
    private UnifiedJobProvider unifiedJobProvider;

    @Mock
    private RedissonClient redissonClient;

    @Test
    public void testStopTask() {
        String jobId = UUID.randomUUID().toString();
        StopPlanBo stopPlanBo = new StopPlanBo();
        stopPlanBo.setAssociativeId(UUID.randomUUID().toString());
        stopPlanBo.setSourceSubType(ResourceSubTypeEnum.ORACLE.getType());
        stopPlanBo.setType(JobTypeEnum.BACKUP);
        JobProvider jobProvider = PowerMockito.mock(JobProvider.class);
        PowerMockito.when(registry.findProvider(any(), any(), any())).thenReturn(jobProvider);

        jobController.stopTask(jobId,stopPlanBo);
        Mockito.verify(jobProvider, Mockito.times(1)).stopJob(any());
    }

    /**
     * 用例名称：安全一体机/主存防勒索，停止创建快照任务
     * 前置条件：安全一体机/主存防勒索，任务是创建快照
     * check点：停止成功
     */
    @Test
    public void testStopTask_HyberDetect_CyberEngine() throws NoSuchFieldException, IllegalAccessException {
        PowerMockito.when(deployTypeService.isCyberEngine()).thenReturn(true);
        String jobId = UUID.randomUUID().toString();
        StopPlanBo stopPlanBo = new StopPlanBo();
        stopPlanBo.setAssociativeId(UUID.randomUUID().toString());
        stopPlanBo.setSourceSubType(ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType());
        stopPlanBo.setType(JobTypeEnum.BACKUP);
        JobProvider jobProvider = PowerMockito.mock(JobProvider.class);
        PowerMockito.when(registry.findProvider(any(), any(), any())).thenReturn(jobProvider);
        Field unifiedJobProviderField = JobController.class.getDeclaredField("unifiedJobProvider");
        unifiedJobProviderField.setAccessible(true);
        unifiedJobProviderField.set(jobController, unifiedJobProvider);
        unifiedJobProviderField.setAccessible(false);
        jobController.stopTask(jobId, stopPlanBo);
        Mockito.verify(unifiedJobProvider, Mockito.times(1)).stopJob(any());

        PowerMockito.when(deployTypeService.isHyperDetectDeployType()).thenReturn(true);
        jobController.stopTask(jobId, stopPlanBo);
        Mockito.verify(unifiedJobProvider, Mockito.times(2)).stopJob(any());
    }

    /**
     * 用例名称：安全一体机，停止勒索侦测任务
     * 前置条件：安全一体机，任务是勒索侦测
     * check点：停止成功
     */
    @Test
    public void testStopTask_AntiRansomware_CyberEngine() {
        PowerMockito.when(deployTypeService.isCyberEngine()).thenReturn(true);
        String jobId = UUID.randomUUID().toString();
        StopPlanBo stopPlanBo = new StopPlanBo();
        stopPlanBo.setAssociativeId(UUID.randomUUID().toString());
        stopPlanBo.setSourceSubType(ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType());
        stopPlanBo.setType(JobTypeEnum.ANTI_RANSOMWARE);
        JobProvider jobProvider = PowerMockito.mock(JobProvider.class);
        PowerMockito.when(registry.findProvider(any(), any(), any())).thenReturn(jobProvider);
        jobController.stopTask(jobId, stopPlanBo);
        Mockito.verify(jobProvider, Mockito.times(1)).stopJob(any());
    }

    @Test
    public void test_update_job_status() {
        String jobId = UUID.randomUUID().toString();
        UpdateJobStatusRequest jobStatusRequest = new UpdateJobStatusRequest();
        jobStatusRequest.setJobRequestId(jobId);
        jobStatusRequest.setTaskId(UUID.randomUUID().toString());
        jobStatusRequest.setStatus(DmeJobStatusEnum.SUCCESS.getTypeName());
        JobBo jobBo = new JobBo();
        jobBo.setStatus(JobStatusEnum.RUNNING.name());
        jobBo.setProgress(55);
        PowerMockito.when(jobService.queryJob(jobId)).thenReturn(jobBo);
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        PowerMockito.when(rBucket.get()).thenReturn(1);
        PowerMockito.when(rBucket.compareAndSet(any(),any())).thenReturn(true);
        jobController.updateJobStatus(jobId, jobStatusRequest);
        Mockito.verify(taskCompleteListener, Mockito.times(0)).taskComplete(any());
        Mockito.verify(jobService, Mockito.times(0)).updateJobLogs(any(),any());
    }
}
