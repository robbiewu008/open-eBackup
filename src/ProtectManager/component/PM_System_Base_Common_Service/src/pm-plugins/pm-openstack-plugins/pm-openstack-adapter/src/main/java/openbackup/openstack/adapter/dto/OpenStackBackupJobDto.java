/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.openstack.adapter.dto;

import openbackup.openstack.adapter.enums.OpenStackJobType;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import javax.validation.Valid;
import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;

/**
 * 云核OpenStack备份任务Dto
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-05
 */
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class OpenStackBackupJobDto {
    /**
     * 任务id
     */
    private String id;

    /**
     * 任务名称
     */
    private String name;

    /**
     * 任务描述
     */
    @NotBlank(message = "Backup job description can not be empty.")
    private String description;

    /**
     * 备份对象实例
     */
    private OpenStackJobType type;

    /**
     * 备份对象实例id，虚拟机id或卷id
     */
    private String instanceId;

    /**
     * 失败重试次数
     */
    @Min(1)
    @Max(5)
    private Integer autoRetryTimes;

    /**
     * 失败重试等待时间，单位：分钟
     */
    @Min(1)
    @Max(30)
    private Integer autoRetryWaitMinutes;

    /**
     * 备份任务调度策略
     */
    @Valid
    private JobScheduleDto jobsSchedule;

    /**
     * 最近一次任务执行结果
     */
    private String lastResult;

    /**
     * 任务状态
     */
    private String status;
}
