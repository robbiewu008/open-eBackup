/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.openstack.adapter.dto;

import openbackup.openstack.adapter.enums.JobScheduleType;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import javax.validation.Valid;
import javax.validation.constraints.NotNull;

/**
 * 备份任务调度策略DTO
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-05
 */
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class JobScheduleDto {
    /**
     * 调度类型
     */
    @NotNull
    private JobScheduleType type;

    /**
     * 调度策略内容
     */
    @NotNull
    @Valid
    private SchedulePolicyDto policy;

    /**
     * 备份副本保留策略
     */
    @NotNull
    @Valid
    private ScheduleRetentionDto retention;
}
