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
package openbackup.data.access.framework.protection.common.converters;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.everyItem;
import static org.hamcrest.Matchers.hasProperty;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.notNullValue;

import openbackup.data.access.framework.protection.controller.req.UpdateJobLogRequest;
import openbackup.data.access.framework.protection.controller.req.UpdateJobStatusRequest;
import openbackup.data.access.framework.protection.dto.TaskCompleteMessageDto;
import openbackup.data.access.framework.protection.mocks.UpdateJobStatusMocker;

import com.huawei.oceanprotect.job.dto.JobLogDto;

import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述
 *
 * @author w00616953
 * @since 2021-12-10
 */
public class JobStatusDtoConverterTest {
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Test
    public void test_convert_status_request_to_job_status_success() {
        UpdateJobStatusRequest jobStatusRequest = UpdateJobStatusMocker.mockUpdateJobStatusRequest();

        UpdateJobRequest jobRequest = JobDataConverter.convertStatusRequest2UpdateJobRequest(jobStatusRequest);

        assertThat(jobRequest.getStatus(), equalTo(JobStatusEnum.PARTIAL_SUCCESS));
        assertThat(jobRequest.getSpeed(), equalTo("123 KB/s"));
        assertThat(jobRequest.getProgress(), equalTo(1));
        assertThat(jobRequest.getExtendStr(), is(notNullValue()));
        assertThat(jobRequest.getExtendStr(), equalTo("{\"key\":\"value\"}"));
    }

    @Test
    public void test_get_job_logs_from_status_request_success() {
        UpdateJobStatusRequest jobStatusRequest = UpdateJobStatusMocker.mockUpdateJobStatusRequest();
        String jobId = jobStatusRequest.getJobRequestId();
        UpdateJobLogRequest jobLogRequest = UpdateJobStatusMocker.mockUpdateJobLogRequest();
        UpdateJobLogRequest jobLogRequest1 = UpdateJobStatusMocker.mockUpdateJobLogRequest();
        jobStatusRequest.setJobLogs(Arrays.asList(jobLogRequest, jobLogRequest1));

        List<JobLogDto> jobLogDtos = JobDataConverter.getJobLogsFromStatusRequest(jobId, jobStatusRequest);

        assertThat(2, equalTo(jobLogDtos.size()));
        assertThat(jobLogDtos, everyItem(hasProperty("jobId", equalTo(jobId))));
        assertThat(jobLogDtos, everyItem(hasProperty("startTime", equalTo(1634006791713L))));
        assertThat(jobLogDtos, everyItem(hasProperty("level", equalTo("info"))));
        assertThat(jobLogDtos, everyItem(hasProperty("logInfo", equalTo("dme_vmware_create_vm_snapshot_label"))));
        assertThat(
                jobLogDtos,
                everyItem(hasProperty("logInfoParam", equalTo("[\"Backup_snapshot_2021-Oct-12 10:46:31.700294\"]"))));
    }

    @Test
    public void tes_should_success_convert_status_request_with_log_given_negative_speed() {
        UpdateJobStatusRequest jobStatusRequest = UpdateJobStatusMocker.mockUpdateJobStatusRequest();
        String jobId = jobStatusRequest.getJobRequestId();
        jobStatusRequest.setSpeed(-1L);

        UpdateJobRequest jobRequest = JobDataConverter.convertStatusRequest2UpdateJobRequest(jobStatusRequest);

        assertThat(StringUtils.EMPTY, equalTo(jobRequest.getSpeed()));
    }

    @Test
    public void test_convert_status_request_to_task_complete_dto_without_extend_info_success() {
        UpdateJobStatusRequest jobStatusRequest = UpdateJobStatusMocker.mockUpdateJobStatusRequest();
        String jobId = jobStatusRequest.getJobRequestId();
        String jobType = "backup";
        JobBo jobBo = new JobBo();
        jobBo.setType(jobType);
        TaskCompleteMessageDto completeMessageDto =
                JobDataConverter.convertJobStatusRequestToTaskCompleteDto(jobBo, jobStatusRequest);

        assertThat(completeMessageDto, is(notNullValue()));
        assertThat(completeMessageDto.getJobType(), equalTo(jobType));
        assertThat(completeMessageDto.getJobId(), equalTo(jobId));
        assertThat(completeMessageDto.getJobStatus(), equalTo(jobStatusRequest.getStatus()));
        assertThat(completeMessageDto.getJobRequestId(), equalTo(jobStatusRequest.getJobRequestId()));
        Assert.assertTrue(completeMessageDto.getExtendsInfo().toString().contains("taskId"));
    }
}
