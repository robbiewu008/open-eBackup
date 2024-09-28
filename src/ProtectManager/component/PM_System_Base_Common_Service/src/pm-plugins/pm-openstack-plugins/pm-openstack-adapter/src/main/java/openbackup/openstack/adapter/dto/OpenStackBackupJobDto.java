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
