package openbackup.openstack.adapter.generator;

import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.JobScheduleDto;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.dto.ScheduleRetentionDto;
import openbackup.openstack.adapter.enums.JobScheduleType;
import openbackup.openstack.adapter.enums.ScheduleRetentionType;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.CreateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.RetentionDto;
import com.huawei.oceanprotect.sla.sdk.dto.ScheduleDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.IntervalUnit;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.enums.RetentionTimeUnit;
import com.huawei.oceanprotect.sla.sdk.enums.RetentionType;
import com.huawei.oceanprotect.sla.sdk.enums.SlaType;
import com.huawei.oceanprotect.sla.sdk.enums.Trigger;
import com.huawei.oceanprotect.sla.sdk.enums.TriggerAction;

import openbackup.system.base.common.constants.DateFormatConstant;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.apache.commons.beanutils.converters.DateConverter;
import org.apache.commons.lang3.StringUtils;
import org.springframework.util.Assert;

import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * SLA对象生成器
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-15
 */
public final class SlaGenerator {
    private static final String NUMBER_FORMAT = "%02d";
    private static final String TIME_SUFFIX = ":00:00";
    private static final String NAME_SEPARATOR = "_";

    private SlaGenerator() {}

    /**
     * 生成CreateSlaCommand
     *
     * @param backupJob {@link OpenStackBackupJobDto} backupJob
     * @return {@link CreateSlaCommand} createSlaCommand
     */
    public static CreateSlaCommand generateCreateSlaCommand(OpenStackBackupJobDto backupJob) {
        CreateSlaCommand sla = new CreateSlaCommand();
        // SLA名称为backupName_instanceId
        sla.setName(backupJob.getName() + NAME_SEPARATOR + backupJob.getInstanceId());
        sla.setApplication(ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());
        sla.setType(SlaType.BACKUP);
        sla.setPolicyList(buildPolicyDto(backupJob));
        return sla;
    }

    /**
     * 生成UpdateSlaCommand
     *
     * @param backupJob {@link OpenStackBackupJobDto} backupJob
     * @param oldSla 修改前SLA
     * @return {@link UpdateSlaCommand} updateSlaCommand
     */
    public static UpdateSlaCommand generateUpdateSlaCommand(OpenStackBackupJobDto backupJob, SlaDto oldSla) {
        UpdateSlaCommand sla = new UpdateSlaCommand();
        sla.setUuid(oldSla.getUuid());
        sla.setName(oldSla.getName());
        sla.setUserId(oldSla.getUserId());
        sla.setApplication(ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());
        sla.setType(SlaType.BACKUP);

        List<PolicyDto> updatedPolicy = buildPolicyDto(backupJob);
        PolicyDto oldPolicy = oldSla.getPolicyList().get(0);
        updatedPolicy.forEach(policy -> {
            policy.setName(oldPolicy.getName());
            policy.setExtParameters(oldPolicy.getExtParameters());
        });
        sla.setPolicyList(updatedPolicy);
        return sla;
    }

    private static List<PolicyDto> buildPolicyDto(OpenStackBackupJobDto backupJob) {
        PolicyDto policy = new PolicyDto();
        policy.setAction(PolicyAction.DIFFERENCE_INCREMENT);
        policy.setType(PolicyType.BACKUP);
        policy.setActive(true);
        policy.setName(backupJob.getName());
        policy.setRetention(buildRetentionDto(backupJob));
        policy.setSchedule(buildScheduleDto(backupJob));
        policy.setExtParameters(buildExtParameters(backupJob));
        return Collections.singletonList(policy);
    }

    private static RetentionDto buildRetentionDto(OpenStackBackupJobDto backupJob) {
        ScheduleRetentionDto retention = backupJob.getJobsSchedule().getRetention();
        RetentionDto retentionDto = new RetentionDto();
        // count为0表示永久保留
        if (retention.getCount() == IsmNumberConstant.ZERO) {
            retentionDto.setRetentionType(RetentionType.PERMANENT);
            return retentionDto;
        }
        if (Objects.equals(retention.getType().getType(), ScheduleRetentionType.TIME.getType())) {
            retentionDto.setRetentionType(RetentionType.TEMPORARY);
            retentionDto.setRetentionDuration(retention.getCount());
            retentionDto.setDurationUnit(RetentionTimeUnit.DAYS.getUnit());
        } else {
            retentionDto.setRetentionType(RetentionType.QUANTITY);
            retentionDto.setRetentionQuantity(retention.getCount());
        }

        return retentionDto;
    }

    private static ScheduleDto buildScheduleDto(OpenStackBackupJobDto backupJob) {
        JobScheduleDto jobsSchedule = backupJob.getJobsSchedule();
        ScheduleDto scheduleDto = new ScheduleDto();
        if (Objects.equals(jobsSchedule.getType().getType(), JobScheduleType.DAYS.getType())) {
            scheduleDto.setTrigger(Trigger.INTERVAL);
            Integer intervalDays = jobsSchedule.getPolicy().getIntervalDays();
            Assert.notNull(intervalDays, "Days schedule backup job interval days can not be null.");
            scheduleDto.setInterval(intervalDays);
            scheduleDto.setIntervalUnit(IntervalUnit.DAYS.getUnit());
        } else {
            scheduleDto.setTrigger(Trigger.CUSTOMIZE_INTERVAL);
            scheduleDto.setTriggerAction(TriggerAction.WEEK);
            List<Integer> daysOfWeek = jobsSchedule.getPolicy().getDaysOfWeek();
            Assert.notEmpty(daysOfWeek, "Week schedule backup job days of week can not be empty.");
            scheduleDto.setDaysOfWeek(
                    daysOfWeek.stream()
                            .map(week -> OpenStackConstants.WEEKS.get(week - 1))
                            .collect(Collectors.toList()));
        }
        scheduleDto.setStartTime(
                StringUtils.isBlank(jobsSchedule.getPolicy().getStartDate())
                        ? getCurrentDateTime()
                        : jobsSchedule.getPolicy().getStartDate());
        scheduleDto.setEndTime(jobsSchedule.getPolicy().getStopDate());
        scheduleDto.setWindowStart(convert2TimeString(jobsSchedule.getPolicy().getExecuteTime()));
        scheduleDto.setWindowEnd(convert2TimeString(jobsSchedule.getPolicy().getExecuteTime()));
        return scheduleDto;
    }

    private static String getCurrentDateTime() {
        DateConverter converter = new DateConverter();
        converter.setPattern(DateFormatConstant.DATE_TIME);
        return converter.convert(String.class, new Date());
    }

    private static JsonNode buildExtParameters(OpenStackBackupJobDto backupJob) {
        ObjectMapper mapper = new ObjectMapper();
        Map<String, Object> parameters = new HashMap<>();

        parameters.put(SlaConstants.AUTO_RETRY, true);
        parameters.put(SlaConstants.AUTO_RETRY_TIMES, backupJob.getAutoRetryTimes());
        parameters.put(SlaConstants.AUTO_RETRY_WAIT_MINUTES, backupJob.getAutoRetryWaitMinutes());
        return mapper.valueToTree(parameters);
    }

    private static String convert2TimeString(Integer number) {
        // number仅为0-23的数字，需转为时间
        return String.format(NUMBER_FORMAT, number) + TIME_SUFFIX;
    }
}
