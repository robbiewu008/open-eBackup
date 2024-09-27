/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.handler.v2.intelligentdetection;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;

import openbackup.data.access.framework.protection.handler.v2.intelligentdetection.UnifiedIntelligentDetectionTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

/**
 * 统一智能侦测任务完成处理器测试集合
 *
 * @author: x30028756
 * @version: [CyberEngine 1.0.0]
 * @since: 2023年3月2日23:40:36
 **/
public class UnifiedIntelligentDetectionTaskCompleteHandlerTest {
    private final JobService jobService = mock(JobService.class);

    private UnifiedIntelligentDetectionTaskCompleteHandler intelligentDetectionTaskCompleteHandler
        = new UnifiedIntelligentDetectionTaskCompleteHandler(jobService);

    /**
     * 用例场景：适配智能侦测任务类型 <br>
     * 前置条件：无 <br>
     * 检查点：流程正常完成 <br>
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(
            intelligentDetectionTaskCompleteHandler.applicable(JobTypeEnum.ANTI_RANSOMWARE.getValue() + "-v2"));
    }

    /**
     * 用例场景：执行解锁和更新job成功完成 <br>
     * 前置条件：任务执行成功 <br>
     * 检查点：流程正常完成 <br>
     */
    @Test
    public void task_complete_success() {
        PowerMockito.doNothing()
            .when(jobService).updateJob(any(String.class), any(UpdateJobRequest.class));
        TaskCompleteMessageBo taskCompleteMessage = new TaskCompleteMessageBo();
        taskCompleteMessage.setJobRequestId("1");
        intelligentDetectionTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessage);
        Mockito.verify(jobService, Mockito.times(1)).updateJob(any(), any());
    }

    /**
     * 用例场景：执行解锁和更新job失败完成 <br>
     * 前置条件：任务执行失败 <br>
     * 检查点：流程正常完成 <br>
     */
    @Test
    public void task_complete_failed() {
        PowerMockito.doNothing()
            .when(jobService).updateJob(any(String.class), any(UpdateJobRequest.class));
        TaskCompleteMessageBo taskCompleteMessage = new TaskCompleteMessageBo();
        taskCompleteMessage.setJobRequestId("1");
        intelligentDetectionTaskCompleteHandler.onTaskCompleteFailed(taskCompleteMessage);
        Mockito.verify(jobService, Mockito.times(1)).updateJob(any(), any());
    }
}