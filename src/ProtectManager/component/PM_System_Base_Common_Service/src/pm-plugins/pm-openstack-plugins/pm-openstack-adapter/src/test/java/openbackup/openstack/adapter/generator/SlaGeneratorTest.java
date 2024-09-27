package openbackup.openstack.adapter.generator;

import static org.assertj.core.api.Assertions.assertThat;

import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.dto.SchedulePolicyDto;
import openbackup.openstack.adapter.testdata.TestDataGenerator;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.RetentionDto;
import com.huawei.oceanprotect.sla.sdk.dto.ScheduleDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.enums.RetentionType;
import com.huawei.oceanprotect.sla.sdk.enums.SlaType;
import com.huawei.oceanprotect.sla.sdk.enums.Trigger;
import com.huawei.oceanprotect.sla.sdk.enums.TriggerAction;

import openbackup.system.base.common.constants.DateFormatConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.joda.time.DateTime;
import org.joda.time.format.DateTimeFormat;
import org.junit.Assert;
import org.junit.Test;

import java.io.IOException;
import java.util.Arrays;
import java.util.Date;

/**
 * {@link SlaGenerator} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-14
 */
public class SlaGeneratorTest {
    /**
     * 用例名称：验证将BackupJob转为SLA对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_return_correctSlaInfo_when_convertJobToSla_given_backupJob() throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        SlaBase sla = SlaGenerator.generateCreateSlaCommand(backupJob);

        assertThat(sla.getApplication()).isEqualTo(ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());
        assertThat(sla.getName()).isEqualTo(backupJob.getName() + "_" + backupJob.getInstanceId());
        assertThat(sla.getType()).isEqualTo(SlaType.BACKUP);
        assertThat(sla.getPolicyList()).hasSize(1);
    }

    /**
     * 用例名称：验证将BackupJob转为Policy对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_return_correctPolicyInfo_when_convertJobToSla_given_backupJob() throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        SlaBase sla = SlaGenerator.generateCreateSlaCommand(backupJob);

        PolicyDto policy = sla.getPolicyList().get(0);
        assertThat(policy.getAction()).isEqualTo(PolicyAction.DIFFERENCE_INCREMENT);
        assertThat(policy.getType()).isEqualTo(PolicyType.BACKUP);
        assertThat(policy.getActive()).isTrue();
        assertThat(policy.getName()).isEqualTo(backupJob.getName());
        assertThat(policy.getRetention()).isNotNull();
        assertThat(policy.getSchedule()).isNotNull();
        assertThat(policy.getExtParameters().get("auto_retry").asBoolean()).isTrue();
        assertThat(policy.getExtParameters().get("auto_retry_times").asInt()).isEqualTo(backupJob.getAutoRetryTimes());
        assertThat(policy.getExtParameters().get("auto_retry_wait_minutes").asInt())
                .isEqualTo(backupJob.getAutoRetryWaitMinutes());
    }

    /**
     * 用例名称：验证将count为0的BackupJob转为SLA的PermanentRetention对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_return_permanentRetention_when_convertJobToSla_given_count_zero() throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createZeroCountRetentionWeekScheduleJob();
        SlaBase sla = SlaGenerator.generateCreateSlaCommand(backupJob);
        PolicyDto policy = sla.getPolicyList().get(0);

        RetentionDto retention = policy.getRetention();
        assertThat(retention.getRetentionType()).isEqualTo(RetentionType.PERMANENT);
    }

    /**
     * 用例名称：验证将timeRetentionBackupJob转为SLA的TemporaryRetention对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_return_temporaryRetention_when_convertJobToSla_given_timeRetentionBackupJob()
            throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        SlaBase sla = SlaGenerator.generateCreateSlaCommand(backupJob);
        PolicyDto policy = sla.getPolicyList().get(0);

        RetentionDto retention = policy.getRetention();
        assertThat(retention.getRetentionType()).isEqualTo(RetentionType.TEMPORARY);
        assertThat(retention.getRetentionDuration()).isEqualTo(backupJob.getJobsSchedule().getRetention().getCount());
        assertThat(retention.getDurationUnit()).isEqualTo("d");
    }

    /**
     * 用例名称：验证将numberRetentionBackupJob转为SLA的QuantityRetention对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_return_quantityRetention_when_convertJobToSla_given_numberRetentionBackupJob()
            throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createNumberRetentionWeekScheduleJob();
        SlaBase sla = SlaGenerator.generateCreateSlaCommand(backupJob);
        PolicyDto policy = sla.getPolicyList().get(0);

        RetentionDto retention = policy.getRetention();
        assertThat(retention.getRetentionType()).isEqualTo(RetentionType.QUANTITY);
        assertThat(retention.getRetentionQuantity()).isEqualTo(backupJob.getJobsSchedule().getRetention().getCount());
    }

    /**
     * 用例名称：验证将weekScheduleBackupJob转为SLA的CustomizeSchedule对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_return_customizeSchedule_when_convertJobToSla_given_weekScheduleBackupJob() throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createNumberRetentionWeekScheduleJob();
        SlaBase sla = SlaGenerator.generateCreateSlaCommand(backupJob);
        PolicyDto policy = sla.getPolicyList().get(0);

        ScheduleDto schedule = policy.getSchedule();
        assertThat(schedule.getTrigger()).isEqualTo(Trigger.CUSTOMIZE_INTERVAL);
        assertThat(schedule.getTriggerAction()).isEqualTo(TriggerAction.WEEK);
        assertThat(schedule.getDaysOfWeek()).usingRecursiveComparison().isEqualTo(Arrays.asList("mon", "thu", "sun"));
        // startDate为空，则为当前时间
        assertThat(backupJob.getJobsSchedule().getPolicy().getStartDate()).isNull();
        assertThat(
                        DateTime.parse(schedule.getStartTime(), DateTimeFormat.forPattern(DateFormatConstant.DATE_TIME))
                                .toDate())
                .isEqualToIgnoringSeconds(new Date());
        assertThat(schedule.getEndTime()).isNull();
        // 24点转为00:00:00
        assertThat(schedule.getWindowStart()).isEqualTo("23:00:00");
        assertThat(schedule.getWindowEnd()).isEqualTo(schedule.getWindowStart());
    }

    /**
     * 用例名称：验证将daysScheduleBackupJob转为SLA的IntervalSchedule对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_return_intervalSchedule_when_convertJobToSla_given_daysScheduleBackupJob() throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        SchedulePolicyDto backupJobPolicy = backupJob.getJobsSchedule().getPolicy();
        SlaBase sla = SlaGenerator.generateCreateSlaCommand(backupJob);
        PolicyDto policy = sla.getPolicyList().get(0);

        ScheduleDto schedule = policy.getSchedule();
        assertThat(schedule.getTrigger()).isEqualTo(Trigger.INTERVAL);
        assertThat(schedule.getTriggerAction()).isNull();
        assertThat(schedule.getDaysOfWeek()).isNull();
        assertThat(schedule.getInterval()).isEqualTo(backupJobPolicy.getIntervalDays());
        assertThat(schedule.getStartTime()).isEqualTo(backupJobPolicy.getStartDate());
        assertThat(schedule.getEndTime()).isEqualTo(backupJobPolicy.getStopDate());
        assertThat(schedule.getWindowStart()).isEqualTo("00:00:00");
        assertThat(schedule.getWindowEnd()).isEqualTo(schedule.getWindowStart());
    }

    /**
     * 用例名称：验证创建将daysScheduleBackupJob转为SLA时，如果intervalDays为空，则抛出异常<br/>
     * 前置条件：无<br/>
     * check点：daysScheduleBackupJob的intervalDays必须存在<br/>
     */
    @Test
    public void should_throwIllegalArgumentException_when_convertJobToSla_given_daysScheduleWithoutIntervalDaysJob()
            throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createDaysScheduleWithoutIntervalDaysJob();
        IllegalArgumentException exception =
                Assert.assertThrows(IllegalArgumentException.class, () -> SlaGenerator.generateCreateSlaCommand(backupJob));
        assertThat(exception.getMessage()).isEqualTo("Days schedule backup job interval days can not be null.");
    }

    /**
     * 用例名称：验证创建将weekScheduleBackupJob转为SLA时，如果daysOfWeek为空，则抛出异常<br/>
     * 前置条件：无<br/>
     * check点：weekScheduleBackupJob的daysOfWeek必须存在<br/>
     */
    @Test
    public void should_throwIllegalArgumentException_when_convertJobToSla_given_weekScheduleWithoutDaysOfWeek()
            throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createWeekScheduleWithoutDaysOfWeekJob();
        IllegalArgumentException exception =
                Assert.assertThrows(IllegalArgumentException.class, () -> SlaGenerator.generateCreateSlaCommand(backupJob));
        assertThat(exception.getMessage()).isEqualTo("Week schedule backup job days of week can not be empty.");
    }
}
