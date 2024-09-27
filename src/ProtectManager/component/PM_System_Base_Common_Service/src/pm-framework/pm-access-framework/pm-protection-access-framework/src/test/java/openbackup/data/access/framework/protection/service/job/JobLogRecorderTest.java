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
package openbackup.data.access.framework.protection.service.job;

import static org.assertj.core.api.Assertions.assertThatIllegalArgumentException;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.mock;
import static org.powermock.api.mockito.PowerMockito.whenNew;

import openbackup.data.access.framework.protection.common.constants.JobStatusLabelConstant;
import openbackup.data.access.framework.protection.service.job.JobLogRecorder;
import openbackup.data.access.framework.restore.constant.RestoreJobLabelConstant;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.UUID;

/**
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/27
 **/
@RunWith(PowerMockRunner.class)
@PrepareForTest(JobLogRecorder.class)
public class JobLogRecorderTest {
    private final JobService jobService = mock(JobService.class);

    private final JobLogRecorder jobLogRecorder = new JobLogRecorder(jobService);

    /**
     * 用例名称：验证更新任务正常步骤参数设置是否正确<br/>
     * 前置条件：无<br/>
     * check点：参数设置与预期一致<br/>
     */
    @Test
    public void should_success_when_recordJobStep_given_correct_param() {
        // Given
        String jobId = UUID.randomUUID().toString();
        JobLogBo jobLogBo = new JobLogBo();
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        try {
            whenNew(JobLogBo.class).withNoArguments().thenReturn(jobLogBo);
            whenNew(UpdateJobRequest.class).withNoArguments().thenReturn(updateJobRequest);
        } catch (Exception e) {
            e.printStackTrace();
        }
        doNothing().when(jobService).updateJob(eq(jobId), any());
        // When
        jobLogRecorder.recordJobStep(jobId, JobLogLevelEnum.INFO, RestoreJobLabelConstant.RESTORE_INIT,
            Collections.singletonList(JobStatusLabelConstant.JOB_SUCCESS_LABEL));
        // Then
        Assert.assertNotNull(jobLogBo);
        Assert.assertEquals(jobId, jobLogBo.getJobId());
        Assert.assertNotNull(jobLogBo.getStartTime());
        Assert.assertEquals(JobLogLevelEnum.INFO.getValue(), jobLogBo.getLevel());
        Assert.assertEquals(RestoreJobLabelConstant.RESTORE_INIT, jobLogBo.getLogInfo());
        Assert.assertTrue(jobLogBo.isUnique());
        Assert.assertTrue(jobLogBo.getLogInfoParam().contains(JobStatusLabelConstant.JOB_SUCCESS_LABEL));
        Assert.assertTrue(updateJobRequest.getJobLogs().contains(jobLogBo));
    }

    /**
     * 用例名称：验证更新任务错误步骤参数设置是否正确<br/>
     * 前置条件：无<br/>
     * check点：参数设置与预期一致<br/>
     */
    @Test
    public void should_success_when_recordJobErrorStep_given_correct_param() {
        // Given
        String jobId = UUID.randomUUID().toString();
        Long errorCode = 12123L;
        JobLogBo jobLogBo = new JobLogBo();
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        try {
            whenNew(JobLogBo.class).withNoArguments().thenReturn(jobLogBo);
            whenNew(UpdateJobRequest.class).withNoArguments().thenReturn(updateJobRequest);
        } catch (Exception e) {
            e.printStackTrace();
        }
        doNothing().when(jobService).updateJob(eq(jobId), any());
        // When
        jobLogRecorder.recordJobStepWithError(jobId, RestoreJobLabelConstant.RESTORE_START, errorCode, null);
        // Then
        Assert.assertNotNull(jobLogBo);
        Assert.assertEquals(jobId, jobLogBo.getJobId());
        Assert.assertNotNull(jobLogBo.getStartTime());
        Assert.assertEquals(JobLogLevelEnum.ERROR.getValue(), jobLogBo.getLevel());
        Assert.assertEquals(RestoreJobLabelConstant.RESTORE_START, jobLogBo.getLogInfo());
        Assert.assertTrue(jobLogBo.isUnique());
        Assert.assertTrue(jobLogBo.getLogInfoParam().contains(JobStatusLabelConstant.JOB_FAIL_LABEL));
        Assert.assertEquals(errorCode + "", jobLogBo.getLogDetail());
        Assert.assertTrue(updateJobRequest.getJobLogs().contains(jobLogBo));
    }

    /**
     * 用例名称：验证任务终结状态为非失败状态时返回日志级别<br/>
     * 前置条件：无<br/>
     * check点：验证返回的级别为info<br/>
     */
    @Test
    public void should_return_info_level_when_getLevelByJobStatus_given_status_not_failed() {
        Assert.assertEquals(JobLogLevelEnum.INFO, jobLogRecorder.getLevelByJobStatus(ProviderJobStatusEnum.SUCCESS));
        Assert.assertEquals(JobLogLevelEnum.INFO,
            jobLogRecorder.getLevelByJobStatus(ProviderJobStatusEnum.PARTIAL_SUCCESS));
        Assert.assertEquals(JobLogLevelEnum.INFO, jobLogRecorder.getLevelByJobStatus(ProviderJobStatusEnum.ABORTED));
        Assert.assertEquals(JobLogLevelEnum.INFO, jobLogRecorder.getLevelByJobStatus(ProviderJobStatusEnum.CANCELLED));
    }

    /**
     * 用例名称：验证任务终结状态为失败状态时返回日志级别<br/>
     * 前置条件：无<br/>
     * check点：验证返回的级别为error<br/>
     */
    @Test
    public void should_return_error_level_when_getLevelByJobStatus_given_status_failed() {
        Assert.assertEquals(JobLogLevelEnum.ERROR, jobLogRecorder.getLevelByJobStatus(ProviderJobStatusEnum.FAIL));
        Assert.assertEquals(JobLogLevelEnum.ERROR,
            jobLogRecorder.getLevelByJobStatus(ProviderJobStatusEnum.ABORT_FAILED));
    }

    /**
     * 用例名称：验证任务状态为非终止状态，日志级别的返回<br/>
     * 前置条件：无<br/>
     * check点：1.验证抛出的异常为IllegalArgumentException  2.验证异常信息符合预期<br/>
     */
    @Test
    public void should_throw_IllegalArgumentException_when_getLevelByJobStatus_given_status_not_support() {
        assertThatIllegalArgumentException().isThrownBy(
                () -> jobLogRecorder.getLevelByJobStatus(ProviderJobStatusEnum.ABORTING))
            .withMessage("function not support status=" + ProviderJobStatusEnum.ABORTING.name());
    }
}
