package openbackup.openstack.adapter.generator;

import static org.assertj.core.api.Assertions.assertThat;

import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.JobScheduleDto;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import openbackup.openstack.adapter.dto.SchedulePolicyDto;
import openbackup.openstack.adapter.dto.ScheduleRetentionDto;
import openbackup.openstack.adapter.enums.JobResult;
import openbackup.openstack.adapter.enums.JobScheduleType;
import openbackup.openstack.adapter.enums.OpenStackJobStatus;
import openbackup.openstack.adapter.enums.OpenStackJobType;
import openbackup.openstack.adapter.enums.ScheduleRetentionType;
import openbackup.openstack.adapter.testdata.TestDataGenerator;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.RetentionDto;
import com.huawei.oceanprotect.sla.sdk.dto.ScheduleDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;

import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;

import org.junit.Test;

import java.io.IOException;
import java.util.Arrays;

/**
 * {@link OpenStackModelsGenerator} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-29
 */
public class OpenStackModelsGeneratorTest {
    /**
     * 用例名称：验证生成的OpenStackBackupJobDto对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_returnCorrectValue_when_generateBackupJob_given_nullJobAndGlobalSla() throws IOException {
        ProtectedObjectInfo protectedObject = TestDataGenerator.createProtectedObject();
        SlaDto sla = TestDataGenerator.createGlobalSla();
        OpenStackBackupJobDto backupJob = OpenStackModelsGenerator.generateBackupJob(protectedObject, sla, null);
        assertThat(backupJob).isNotNull();
        assertThat(backupJob.getId()).isEqualTo(protectedObject.getResourceId());
        assertThat(backupJob.getName()).isEqualTo(
            protectedObject.getExtParameters().get(OpenStackConstants.NAME).toString());
        assertThat(backupJob.getDescription()).isEqualTo(
            protectedObject.getExtParameters().get(OpenStackConstants.DESCRIPTION).toString());
        assertThat(backupJob.getType().getType()).isEqualTo(
            protectedObject.getExtParameters().get(OpenStackConstants.BACKUP_TYPE).toString());
        assertThat(backupJob.getInstanceId()).isEqualTo(
            protectedObject.getExtParameters().get(OpenStackConstants.INSTANCE_ID).toString());
        assertThat(backupJob.getAutoRetryTimes()).isEqualTo(
            sla.getPolicyList().get(0).getExtParameters().get(SlaConstants.AUTO_RETRY_TIMES).asInt());
        assertThat(backupJob.getAutoRetryWaitMinutes()).isEqualTo(
            sla.getPolicyList().get(0).getExtParameters().get(SlaConstants.AUTO_RETRY_WAIT_MINUTES).asInt());
        assertThat(backupJob.getLastResult()).isBlank();
        assertThat(backupJob.getStatus()).isEqualTo(OpenStackJobStatus.RUNNING.getStatus());
        assertThat(backupJob.getJobsSchedule()).isNull();
    }

    /**
     * 用例名称：验证如果为周期性和永久保留的SLA，则生成DaysSchedule，并且其他属性设置正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_returnDaysSchedule_when_generateBackupJob_given_intervalAndPermanentSla() throws IOException {
        ProtectedObjectInfo object = TestDataGenerator.createUnprotectedObject();
        SlaDto sla = TestDataGenerator.createIntervalAndPermanentSla();
        ScheduleDto slaSchedule = sla.getPolicyList().get(0).getSchedule();
        JobBo job = TestDataGenerator.createSuccessJob();
        OpenStackBackupJobDto backupJob = OpenStackModelsGenerator.generateBackupJob(object, sla, job);
        assertThat(backupJob.getLastResult()).isEqualTo(JobResult.SUCCESS.getResult());
        assertThat(backupJob.getStatus()).isEqualTo(OpenStackJobStatus.STOP.getStatus());
        JobScheduleDto jobsSchedule = backupJob.getJobsSchedule();
        assertThat(jobsSchedule).isNotNull();
        assertThat(jobsSchedule.getType()).isEqualTo(JobScheduleType.DAYS);
        SchedulePolicyDto jobPolicy = jobsSchedule.getPolicy();
        assertThat(jobPolicy).isNotNull();
        assertThat(jobPolicy.getIntervalDays()).isEqualTo(slaSchedule.getInterval());
        assertThat(jobPolicy.getDaysOfWeek()).isNull();
        assertThat(jobPolicy.getStartDate()).isEqualTo(slaSchedule.getStartTime());
        assertThat(jobPolicy.getStopDate()).isEqualTo(slaSchedule.getEndTime());
        assertThat(jobPolicy.getExecuteTime()).isZero();
        ScheduleRetentionDto jobRetention = jobsSchedule.getRetention();
        assertThat(jobRetention.getType()).isEqualTo(ScheduleRetentionType.TIME);
        assertThat(jobRetention.getCount()).isZero();
    }

    /**
     * 用例名称：验证如果为按周和按日期保留的SLA，则生成WeekSchedule，并且其他属性设置正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_returnWeekSchedule_when_generateBackupJob_given_weekAndTemporarySla() throws IOException {
        ProtectedObjectInfo object = TestDataGenerator.createProtectedObject();
        SlaDto sla = TestDataGenerator.createWeekAndTemporarySla();
        RetentionDto slaRetention = sla.getPolicyList().get(0).getRetention();
        JobBo job = TestDataGenerator.createFailJob();
        OpenStackBackupJobDto backupJob = OpenStackModelsGenerator.generateBackupJob(object, sla, job);
        assertThat(backupJob.getLastResult()).isEqualTo(JobResult.FAIL.getResult());
        JobScheduleDto jobsSchedule = backupJob.getJobsSchedule();
        assertThat(jobsSchedule.getType()).isEqualTo(JobScheduleType.WEEKS);
        SchedulePolicyDto jobPolicy = jobsSchedule.getPolicy();
        assertThat(jobPolicy.getIntervalDays()).isNull();
        assertThat(jobPolicy.getDaysOfWeek()).usingRecursiveComparison().isEqualTo(Arrays.asList(1,2,3,4,5,6));
        assertThat(jobPolicy.getExecuteTime()).isEqualTo(10);
        ScheduleRetentionDto jobRetention = jobsSchedule.getRetention();
        assertThat(jobRetention.getType()).isEqualTo(ScheduleRetentionType.TIME);
        assertThat(jobRetention.getCount()).isEqualTo(slaRetention.getRetentionDuration());
    }

    /**
     * 用例名称：验证如果为按周和按数量保留的SLA，则生成NumberRetention，并且其他属性设置正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_returnQuantityRetention_when_generateBackupJob_given_quantitySla() throws IOException {
        ProtectedObjectInfo object = TestDataGenerator.createProtectedObject();
        SlaDto sla = TestDataGenerator.createWeekAndQuantitySla();
        RetentionDto slaRetention = sla.getPolicyList().get(0).getRetention();
        JobBo job = TestDataGenerator.createRunningJob();
        OpenStackBackupJobDto backupJob = OpenStackModelsGenerator.generateBackupJob(object, sla, job);
        assertThat(backupJob.getLastResult()).isEqualTo(JobResult.OTHERS.getResult());
        JobScheduleDto jobsSchedule = backupJob.getJobsSchedule();
        assertThat(jobsSchedule.getType()).isEqualTo(JobScheduleType.WEEKS);
        SchedulePolicyDto jobPolicy = jobsSchedule.getPolicy();
        assertThat(jobPolicy.getDaysOfWeek()).usingRecursiveComparison().isEqualTo(Arrays.asList(1,7));
        ScheduleRetentionDto jobRetention = jobsSchedule.getRetention();
        assertThat(jobRetention.getType()).isEqualTo(ScheduleRetentionType.NUMBER);
        assertThat(jobRetention.getCount()).isEqualTo(slaRetention.getRetentionQuantity());
    }

    /**
     * 用例名称：验证生成的OpenStackRestoreJobDto对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_returnCorrectValue_when_generateRestoreJob_given_runningJob() {
        JobBo job = createJobBo();
        job.setStatus(JobStatusEnum.RUNNING.name());
        OpenStackRestoreJobDto restoreJob = OpenStackModelsGenerator.generateRestoreJob(job);

        assertThat(restoreJob).isNotNull();
        assertThat(restoreJob.getId()).isEqualTo(job.getJobId());
        assertThat(restoreJob.getCopyId()).isEqualTo(job.getCopyId());
        JSONObject advanceParams = JSONObject.fromObject(job.getMessage())
            .getJSONObject("payload")
            .getJSONObject("advanceParams");
        assertThat(restoreJob.getName()).isEqualTo(advanceParams.getString(OpenStackConstants.RESTORE_NAME));
        assertThat(restoreJob.getDescription()).isEqualTo(advanceParams.getString(OpenStackConstants.DESCRIPTION));
        assertThat(restoreJob.getType()).isEqualTo(OpenStackJobType.SERVER);
        assertThat(restoreJob.getInstanceId()).isEqualTo(advanceParams.getString(OpenStackConstants.INSTANCE_ID));
        assertThat(restoreJob.getResult()).isEqualTo(JobResult.OTHERS.getResult());
        assertThat(restoreJob.getStatus()).isEqualTo(OpenStackJobStatus.RUNNING.getStatus());
    }

    /**
     * 用例名称：验证生成的OpenStackRestoreJobDto对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_returnCorrectValue_when_generateRestoreJob_given_failJob() {
        JobBo job = createJobBo();
        job.setStatus(JobStatusEnum.FAIL.name());
        OpenStackRestoreJobDto restoreJob = OpenStackModelsGenerator.generateRestoreJob(job);

        assertThat(restoreJob.getResult()).isEqualTo(JobResult.FAIL.getResult());
        assertThat(restoreJob.getStatus()).isEqualTo(OpenStackJobStatus.COMPLETED.getStatus());
    }

    private JobBo createJobBo() {
        JobBo job = new JobBo();
        job.setJobId(UUIDGenerator.getUUID());
        job.setCopyId(UUIDGenerator.getUUID());
        JSONObject jobMessage = new JSONObject();
        JSONObject jobPayload = new JSONObject();
        JSONObject advanceParams = new JSONObject();
        advanceParams.put(OpenStackConstants.RESTORE_NAME, "restore name");
        advanceParams.put(OpenStackConstants.DESCRIPTION, "restore description");
        advanceParams.put(OpenStackConstants.RESTORE_TYPE, OpenStackJobType.SERVER.getType());
        advanceParams.put(OpenStackConstants.INSTANCE_ID, UUIDGenerator.getUUID());
        jobPayload.put("advanceParams", advanceParams.toString());
        jobMessage.put("payload", jobPayload.toString());
        job.setMessage(jobMessage.toString());

        return job;
    }
}
