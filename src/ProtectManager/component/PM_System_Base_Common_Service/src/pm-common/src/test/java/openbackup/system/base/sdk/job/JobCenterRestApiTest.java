package openbackup.system.base.sdk.job;

import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.repository.RepositoryType;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobBo;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.job.model.request.JobScheduleConfig;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import openbackup.system.base.sdk.schedule.model.Schedule;
import openbackup.system.base.sdk.schedule.model.ScheduleResponse;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

import java.util.Date;
import java.util.List;

/**
 * JobCenterRestApi test
 *
 * @author jwx701567
 * @since 2021-03-12
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(JobCenterRestApiTest.class)
@AutoConfigureMockMvc
public class JobCenterRestApiTest {
    @Mock
    private JobCenterRestApi jobCenterRestApi;

    @Test
    public void complete_default_job_success() {
        JobStatusEnum jobStatusEnum = JobStatusEnum.SUCCESS;
        String jobId = "151225";
        jobCenterRestApi.completeJob(jobId, jobStatusEnum);
    }

    @Test
    public void create_default_immediate_schedule_succes() {
        ScheduleRestApi scheduleRestApi = getScheduleRestApi();
        String storageId = "12345";
        String IMPORT_ARCHIVE_COPIES_TOPIC = "IMPORT_ARCHIVE_COPIES_TOPIC";
        // 构造kafka消息的參數
        JSONObject params =
                new JSONObject().set("storageId", storageId).set("repositoryType", RepositoryType.S3.getType());

        // v1/internal/jobs接口请求体，见实体类 CreateJobRequest
        TokenBo token = TokenBo.get(null);
        String userId = VerifyUtil.isEmpty(token) ? "" : token.getUser().getId();
        JSONObject task =
                new JSONObject()
                        .set("type", JobTypeEnum.ARCHIVE_IMPORT.getValue())
                        .set("sourceId", storageId)
                        .set("userId", userId);

        scheduleRestApi.createImmediateSchedule(IMPORT_ARCHIVE_COPIES_TOPIC, true, params, task);
    }

    @Test
    public void create_default_interval_schedule_success() {
        ScheduleRestApi scheduleRestApi = getScheduleRestApi();

        ScheduleResponse intervalSchedule =
                scheduleRestApi.createIntervalSchedule(
                        "alarm_dump_task", "alarm_dump_topic", "1d", new Date(), new JSONObject());
        System.out.println(intervalSchedule);
    }

    private static JobCenterRestApi getJobCenterRestApi() {
        return new JobCenterRestApi() {
            /**
             * 创建任务
             *
             * @param job 任务
             * @return jobId 任务ID
             */
            @Override
            public String createJob(CreateJobRequest job) {
                return null;
            }

            /**
             * Query jobs by types page list response.
             *
             * @param types the types
             * @param statusList the status list
             * @param startPage the start page
             * @param pageSize the page size
             * @return the page list response
             */
            @Override
            public PageListResponse<JobBo> queryJobsByTypes(List<String> types, List<String> statusList, int startPage,
                int pageSize) {
                return null;
            }

            @Override
            public PageListResponse<JobLogBo> queryLastJobLogs(String jobId) {
                return null;
            }

            @Override
            public void updateJob(String jobId, UpdateJobRequest jobRequest) {

            }

            @Override
            public PageListResponse<JobBo> queryJobs(String jobId, int startPage, int pageSize) {
                return null;
            }

            @Override
            public void updateJobs(List<JobBo> jobBoList) {

            }

            @Override
            public void abortTask(String jobId) {

            }

            @Override
            public void verifyJobOwnership(String userId, List<String> uuidList) {

            }

            @Override
            public void updateJobSchedulePolicy(JobScheduleConfig config) {

            }

            @Override
            public void updateUnfinishedJob(){

            }
        };
    }


    private static ScheduleRestApi getScheduleRestApi() {
        return new ScheduleRestApi() {
            @Override
            public ScheduleResponse createSchedule(Schedule schedule) {
                return null;
            }

            @Override
            public void deleteSchedule(String scheduleId) {}
        };
    }
}
